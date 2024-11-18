#include "gui/texts/TreePanelTexts.hpp"

#include "gui/widgets/ProjectTreePanel.h"

#include <QtWidgets/QScrollBar>
#include <QtWidgets/qmenu.h>
#include <QtGui/QDrag>
#include <QtGui/QDragLeaveEvent>

#include "controller/controls/ControlTree.h"
#include "controller/controls/ControlSpecial.h"
#include "controller/controls/ControlExportPC.h"
#include "controller/controls/ControlFunctionPiping.h"

#include "gui/GuiData/GuiDataGeneralProject.h"
//#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiData3dObjects.h"
#include "utils/Logger.h"

#include "gui/widgets/TreeNodeSystem/TreeNode.h"
#include "gui/Texts.hpp"

#include "models/graph/ViewPointNode.h"
#include "models/graph/ClusterNode.h"
#include "models/graph/ScanNode.h"
#include "models/graph/PointNode.h"
#include "models/graph/CylinderNode.h"
#include "models/graph/SphereNode.h"
#include "models/graph/BoxNode.h"
#include "models/graph/MeshObjectNode.h"
#include "models/graph/ScanObjectNode.h"
#include "models/graph/SimpleMeasureNode.h"
#include "models/graph/PolylineMeasureNode.h"

#include "models/graph/GraphManager.h"

#include "controller/controls/ControlClippingEdition.h"

#include "magic_enum/magic_enum.hpp"

#define GTLOG Logger::log(LoggerMode::GTLog)
#define GTELOG Logger::log(LoggerMode::GTExtraLog)

ProjectTreePanel::ProjectTreePanel(IDataDispatcher& dataDispatcher, GraphManager& graphManager, float guiScale)
	: QTreeView()
	, m_dataDispatcher(dataDispatcher)
	, m_graphManager(graphManager)
	, m_nodeFactory(new TreeNodeFactory(this, graphManager))
{
	setDragEnabled(true);
	setAcceptDrops(true);
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setHeaderHidden(true);
	setAnimated(false);
	setSortingEnabled(false);
	setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
	setEditTriggers(QAbstractItemView::EditTriggers());

	this->setMaximumWidth(350 * guiScale);
	this->show();

	// Link action
	QObject::connect(this, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showTreeMenu(QPoint)));
	QObject::connect(this, &QTreeView::doubleClicked, this, &ProjectTreePanel::treeViewMoveProgress);
	QObject::connect(this, &QTreeView::collapsed, this, &ProjectTreePanel::onCollapse);
	QObject::connect(this, &QTreeView::expanded, this, &ProjectTreePanel::onExpand);
	//QObject::connect(this, &QTreeView::collapsed, this, &ProjectTreePanel::collapseChildren);

    registerGuiDataFunction(guiDType::actualizeNodes, &ProjectTreePanel::actualizeNodes);
	registerGuiDataFunction(guiDType::selectElems, &ProjectTreePanel::selectItems);
    registerGuiDataFunction(guiDType::projectLoaded, &ProjectTreePanel::cleanTree);
}

ProjectTreePanel::~ProjectTreePanel()
{
	delete m_nodeFactory;
	m_dataDispatcher.unregisterObserver(this);
}

void ProjectTreePanel::informData(IGuiData* data)
{
	if (m_model != nullptr)
		m_model->blockSignals(true);
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		GTELOG << "Tree " << magic_enum::enum_name(data->getType()) << " event" << LOGENDL;
		treeMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
	if (m_model != nullptr)
		m_model->blockSignals(false);
}

void ProjectTreePanel::blockAllSignals(bool block)
{
	blockSignals(block);
	m_model->blockSignals(block);
	QItemSelectionModel* selectionModel = this->selectionModel();
	if(selectionModel)
		selectionModel->blockSignals(block);
}

void ProjectTreePanel::actualizeNodes(IGuiData* data)
{
	if (m_model == nullptr)
		return;


	GuiDataTreeActualizeNodes* actualizeNodes = static_cast<GuiDataTreeActualizeNodes*>(data);
	std::chrono::steady_clock::time_point tp_start = std::chrono::steady_clock::now();
	
	//Update
	{
		std::unordered_set<TreeNode*> nodesToUpdate;
		std::unordered_set<TreeType> treetypes;
		for (const SafePtr<AGraphNode>& obj : actualizeNodes->m_objects)
		{
			ReadPtr<AGraphNode> rObj = obj.cget();
			if (rObj)
				for (TreeType treetype : rObj->getArboTreeTypes())
					treetypes.insert(treetype);
		}

		for (TreeType treetype : treetypes)
		{
			if (m_rootNodes.find(treetype) != m_rootNodes.end())
			{
				for (TreeNode* treenode : m_rootNodes[treetype])
					nodesToUpdate.insert(treenode);
			}
		}


		for (TreeNode* node : nodesToUpdate)
			m_nodeFactory->updateNode(node);
	}

	applyWaitingRefresh();

	std::chrono::steady_clock::time_point tp_end = std::chrono::steady_clock::now();
	
	float totalDuration = std::chrono::duration<float, std::milli>(tp_end - tp_start).count();

	GTLOG << "TreeUpdate total duration : " 
		<< totalDuration << " ms" << LOGENDL;
}

void  ProjectTreePanel::selectItems(IGuiData* data)
{
	if (m_model == nullptr)
		return;
	updateSelection((static_cast<GuiDataSelectItems*>(data))->m_objects);
	applyWaitingRefresh();
}

void ProjectTreePanel::cleanTree(IGuiData* data)
{
    GuiDataProjectLoaded* projectLoaded = static_cast<GuiDataProjectLoaded*>(data);

	if (projectLoaded->m_isProjectLoad == false)
	{
		m_lastTreeNodeSelected = nullptr;
		m_rootNodes.clear();
		m_dropList.clear();
		if (m_model != nullptr)
			m_model->getTreeNodes().clear();
		m_model = nullptr;
		this->setModel(m_model);

	}
	else
		generateTreeModel();

}

void ProjectTreePanel::applyWaitingRefresh()
{
    int scrollValue = verticalScrollBar()->value();

	blockAllSignals(true);

	QModelIndexList indList;
	for (int i = 0; i < m_model->rowCount(); i++)
		if (m_model->item(i))
			indList.push_back(m_model->item(i)->index());

	for (const QModelIndex& ind : indList)
	{
		if (isExpanded(ind))
		{
			collapse(ind);
			expand(ind);
		}
	}

	blockAllSignals(false);

    verticalScrollBar()->setValue(scrollValue);
}

void ProjectTreePanel::removeTreeNode(TreeNode* node)
{
	if (node->isStructNode())
		return;

    // 1. On enlève de la map SafePtr<AGraphNode> -> std::vector<TreeNode*> le TreeNode correspondant
    if (m_model->getTreeNodes().find(node->getData()) != m_model->getTreeNodes().end())
    {
        std::vector<TreeNode*>& nodes = m_model->getTreeNodes().at(node->getData());
            // Supprime le TreeNode du vector
            for (auto it = nodes.begin(); it != nodes.end(); it++)
            {
                if ((*it) == node)
                {
                    it = nodes.erase(it);
                        break;
                }
            }
        // Suppression de la paire <SafePtr<AGraphNode> , std::vector<TreeNode*>> dans la map si il n'y a plus de TreeNode associé
        if (nodes.empty())
            m_model->getTreeNodes().erase(node->getData());
    }

    // 3. Remove from Qt TreeModel
    for (int i = node->rowCount(); i > 0; i--)
		removeTreeNode(static_cast<TreeNode*>(node->child(i - 1)));

	QItemSelectionModel* selectionModel = this->selectionModel();
	if (selectionModel)
		selectionModel->blockSignals(true);
    // WARNING ! After removing a QStandardItem from its parent, the item will be destroyed.     !
    // ! ! ! ! ! Ence, it is unsafe to use the 'node' after calling 'QStandardItem::removeRow()' !
    QStandardItem* parent = node->parent();
	if (parent)
		parent->QStandardItem::removeRow(node->index().row());
	else
		m_model->removeRow(node->index().row());

	if (selectionModel)
		selectionModel->blockSignals(false);
}

void ProjectTreePanel::updateSelection(const std::unordered_set<SafePtr<AGraphNode>>& datas)
{
	// On vide la liste Qt de s�lection
	selectionModel()->blockSignals(true);

	selectionModel()->clear(); 
	m_selectedNodes.clear();

	for (const SafePtr<AGraphNode>& data : datas)
	{
		std::vector<TreeNode*> treeNodes = m_model->getTreeNodes(data);
		for (TreeNode* treeNode : treeNodes)
			selectionModel()->select(m_model->indexFromItem(treeNode), QItemSelectionModel::Select);

		m_selectedNodes.insert(data);
	}

	selectionModel()->blockSignals(false);
}

/*
void ProjectTreePanel::recRefreshCheckStateQtItem_ascendant(QStandardItem* node)
{
    if (!node)
        return;

	if (node->isCheckable())
	{
		Qt::CheckState state = node->checkState();
		bool stateInit = false;
		for (int r = 0; r < node->rowCount(); ++r)
		{
			if (!node->child(r)->isCheckable())
				continue;

			if (stateInit)
				state = state != node->child(r)->checkState() ? Qt::CheckState::PartiallyChecked : state;
			else
			{
				state = node->child(r)->checkState();
				stateInit = true;
			}
		}
		node->setCheckState(state);
	}

    QStandardItem* parent = node->parent();
	if (parent != nullptr)
		recRefreshCheckStateQtItem_ascendant(parent);
}
*/

void ProjectTreePanel::generateTreeModel()
{
	GTLOG << "Generate Tree Model" << LOGENDL;

	m_model = new TreeModel(1, 1, this);
	setDragDropMode(QAbstractItemView::InternalMove);
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setDragEnabled(true);
	setAcceptDrops(true);
	setDropIndicatorShown(true);

	SafePtr<ClusterNode> hierarchyNode = m_graphManager.getHierarchyMasterCluster();

	TreeNode* hierarchyTreeNode = m_nodeFactory->constructTreeNode(hierarchyNode, nullptr, TreeType::Hierarchy);
	hierarchyTreeNode->setText(TEXT_HIERARCHY_TREE_NODE);
	hierarchyTreeNode->setType(ElementType::MasterCluster);
	hierarchyTreeNode->setSelectable(true);
	hierarchyTreeNode->setCheckable(true);
	hierarchyTreeNode->setCheckState(Qt::CheckState::Checked);
    m_model->setItem(0, hierarchyTreeNode);
	if (m_rootNodes.find(TreeType::Hierarchy) == m_rootNodes.end())
		m_rootNodes[TreeType::Hierarchy] = std::vector<TreeNode*>();
	m_rootNodes[TreeType::Hierarchy].push_back(hierarchyTreeNode);

	m_itemsNode = m_nodeFactory->constructMasterNode(TEXT_ITEMS_TREE_NODE, TreeType::RawData);
    m_model->setItem(1, m_itemsNode);
	m_itemsNode->setCheckable(true);
	m_itemsNode->setCheckState(Qt::CheckState::Checked);

	TreeNode* viewpointsAttrNode = m_nodeFactory->constructMasterNode(TEXT_VIEWPOINTS_TREE_NODE, TreeType::ViewPoint);

	TreeNode* scansAttrNode = buildTreeModelBranch(TEXT_SCANS_TREE_NODE, TreeType::Scan);

	TreeNode* tagsAttrNode = m_nodeFactory->constructMasterNode(TEXT_TAGS_TREE_NODE, TreeType::Tags);

	TreeNode* boxesAttrNode = m_nodeFactory->constructMasterNode(TEXT_BOXES_TREE_NODE, TreeType::Boxes);

	TreeNode* measureAttrNode = m_nodeFactory->constructMasterNode(TEXT_MEASURES_TREE_NODE, TreeType::Measures);

	TreeNode* pipesTree = m_nodeFactory->constructMasterNode(TEXT_PIPES_TREE_NODE, TreeType::Pipe);

	TreeNode* spheresTree = m_nodeFactory->constructMasterNode(TEXT_SPHERES_TREE_NODE, TreeType::Sphere);

	TreeNode* pointsTree = m_nodeFactory->constructMasterNode(TEXT_POINT_TREE_NODE, TreeType::Point);

	TreeNode* pcObjsTree = m_nodeFactory->constructMasterNode(TEXT_PC_OBJECT_TREE_NODE, TreeType::Pco);

	TreeNode* objsTree = m_nodeFactory->constructMasterNode(TEXT_OBJ_OBJECT_TREE_NODE, TreeType::MeshObjects);

	m_itemsNode->appendRow(viewpointsAttrNode);
	m_itemsNode->appendRow(scansAttrNode);
	m_itemsNode->appendRow(tagsAttrNode);
	m_itemsNode->appendRow(boxesAttrNode);
	m_itemsNode->appendRow(measureAttrNode);
	m_itemsNode->appendRow(pipesTree);
	m_itemsNode->appendRow(spheresTree);
	m_itemsNode->appendRow(pointsTree);
	m_itemsNode->appendRow(pcObjsTree);
	m_itemsNode->appendRow(objsTree);

	measureAttrNode->appendRow(buildTreeModelBranch(TEXT_MEASURES_TREE_ROOT_NODE, TreeType::Measures));
	tagsAttrNode->appendRow(buildTreeModelBranch(TEXT_TAGS_TREE_ROOT_NODE, TreeType::Tags));
	boxesAttrNode->appendRow(buildTreeModelBranch(TEXT_BOXES_TREE_ROOT_NODE, TreeType::Boxes));
	pcObjsTree->appendRow(buildTreeModelBranch(TEXT_PC_OBJECT_TREE_ROOT_NODE, TreeType::Pco));
	pointsTree->appendRow(buildTreeModelBranch(TEXT_POINT_TREE_ROOT_NODE, TreeType::Point));
	pipesTree->appendRow(buildTreeModelBranch(TEXT_PIPES_TREE_ROOT_NODE, TreeType::Pipe));
	pipesTree->appendRow(buildTreeModelBranch(TEXT_PIPING_TREE_ROOT_NODE, TreeType::Piping));
	spheresTree->appendRow(buildTreeModelBranch(TEXT_SPHERES_TREE_ROOT_NODE, TreeType::Sphere));
	objsTree->appendRow(buildTreeModelBranch(TEXT_MESHOBJECT_TREE_ROOT_NODE, TreeType::MeshObjects));
	viewpointsAttrNode->appendRow(buildTreeModelBranch(TEXT_VIEWPOINTS_TREE_ROOT_NODE, TreeType::ViewPoint));

	m_nodeFactory->constructColorNodes({ ElementType::ViewPoint }, viewpointsAttrNode);

	m_nodeFactory->constructColorNodes({ ElementType::Tag }, tagsAttrNode);
	m_nodeFactory->constructStatusNodes({ ElementType::Tag }, tagsAttrNode);
	m_nodeFactory->constructClipMethodNodes({ ElementType::Tag }, tagsAttrNode);

	m_nodeFactory->constructSubList({
		{ ElementType::Box, TEXT_SIMPLE_BOX_SUB_NODE },
		{ ElementType::Grid, TEXT_GRID_SUB_NODE }}, boxesAttrNode);

	m_nodeFactory->constructStatusNodes({ ElementType::Box, ElementType::Grid }, boxesAttrNode);
	m_nodeFactory->constructClipMethodNodes({ ElementType::Box, ElementType::Grid }, boxesAttrNode);
	m_nodeFactory->constructColorNodes({ ElementType::Box, ElementType::Grid }, boxesAttrNode);


	std::vector<TreeNode*> measures = m_nodeFactory->constructSubList({
		{ ElementType::SimpleMeasure, TEXT_SIMPLE_MEASURE },
		{ ElementType::PolylineMeasure, TEXT_POLYLINE_MEASURE },
		{ ElementType::ColumnTiltMeasure, TEXT_COLUMNTILT_MEASURE },
		{ ElementType::BeamBendingMeasure, TEXT_BEAMBENDING_MEASURE },
		{ ElementType::SimpleMeasure, TEXT_SIMPLE_MEASURE },
		{ ElementType::PolylineMeasure, TEXT_POLYLINE_MEASURE },
		{ ElementType::PipeToPipeMeasure, TEXT_PIPETOPIPE_MEASURE },
		{ ElementType::PipeToPlaneMeasure, TEXT_PIPETOPLANE_MEASURE },
		{ ElementType::PointToPlaneMeasure, TEXT_POINTTOPLANE_MEASURE },
		{ ElementType::PointToPipeMeasure, TEXT_POINTTOPIPE_MEASURE } }, measureAttrNode);

	m_nodeFactory->constructStatusNodes({ ElementType::SimpleMeasure }, measures[0]);
	m_nodeFactory->constructClipMethodNodes({ ElementType::SimpleMeasure }, measures[0]);

	m_nodeFactory->constructStatusNodes({ ElementType::PolylineMeasure }, measures[1]);
	m_nodeFactory->constructClipMethodNodes({ ElementType::PolylineMeasure }, measures[1]);

	std::vector<TreeNode*> pipes = m_nodeFactory->constructSubList({
		{ ElementType::Cylinder, TEXT_CYLINDER_SUB_NODE },
		{ ElementType::Torus, TEXT_TORUS_SUB_NODE }}, pipesTree);

	m_nodeFactory->constructStatusNodes({ ElementType::Torus, ElementType::Cylinder }, pipesTree);
	m_nodeFactory->constructClipMethodNodes({ ElementType::Torus, ElementType::Cylinder }, pipesTree);
	m_nodeFactory->constructColorNodes({ ElementType::Torus, ElementType::Cylinder }, pipesTree);

	m_nodeFactory->constructColorNodes({ ElementType::Point }, pointsTree);
	m_nodeFactory->constructStatusNodes({ ElementType::Point }, pointsTree);
	m_nodeFactory->constructClipMethodNodes({ ElementType::Point }, pointsTree);

	m_nodeFactory->constructStatusNodes({ ElementType::Sphere }, spheresTree);
	m_nodeFactory->constructClipMethodNodes({ ElementType::Sphere }, spheresTree);
	m_nodeFactory->constructColorNodes({ ElementType::Sphere }, spheresTree);

	m_nodeFactory->constructColorNodes({ ElementType::PCO }, pcObjsTree);
	m_nodeFactory->constructStatusNodes({ ElementType::PCO }, pcObjsTree);

	m_nodeFactory->constructColorNodes({ ElementType::MeshObject }, objsTree);

	this->setModel(m_model);
	this->expand(m_itemsNode->index());

	QItemSelectionModel* pISM = this->selectionModel();
	QObject::connect(pISM, &QItemSelectionModel::selectionChanged, this, &ProjectTreePanel::treeSelectionChanged);
	QObject::connect(m_model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(onTreeDataChanged(QStandardItem*)));

	GTLOG << "End Generate Tree Model" << LOGENDL;
}

TreeNode* ProjectTreePanel::buildTreeModelBranch(const QString& name, TreeType treetype)
{
	std::function<bool(const SafePtr<AGraphNode>& data)> filter = 
		[treetype](const SafePtr<AGraphNode>& data)
	{
		bool hasParent = bool(AGraphNode::getOwningParent(data, treetype));
		ReadPtr<AGraphNode> rData = data.cget();
		if (!rData)
			return false;
		return (rData->getDefaultTreeType() == treetype && !hasParent);
	};
	std::function<std::unordered_set<SafePtr<AGraphNode>>(const GraphManager&)> getChildFonction =
		[filter](const GraphManager& manager)
	{
		return manager.getNodesOnFilter(filter);
	};

	std::function<bool(const SafePtr<AGraphNode>& data)> canDrop =
		[treetype](const SafePtr<AGraphNode>& data)
	{
		ReadPtr<AGraphNode> rData = data.cget();
		if (!rData)
			return false;
		return (rData->getDefaultTreeType() == treetype);
	};

	std::function<void(IDataDispatcher& dispatcher, std::unordered_set<SafePtr<AGraphNode>>)> onDrop =
		[treetype](IDataDispatcher& dispatcher, std::unordered_set<SafePtr<AGraphNode>> datas)
	{
		dispatcher.sendControl(new control::tree::DropElements(SafePtr<AGraphNode>(), treetype, datas));
	};

	TreeNode* item = new TreeNode(SafePtr<AGraphNode>(), getChildFonction, canDrop, onDrop, treetype);

	item->setText(name);
	item->setStructNode(true);
	item->setType(ElementType::MasterCluster); 
	item->setSelectable(true);
	item->setCheckable(true);

	m_nodeFactory->updateNode(item);

	if (m_rootNodes.find(treetype) == m_rootNodes.end())
		m_rootNodes[treetype] = std::vector<TreeNode*>();
	m_rootNodes[treetype].push_back(item);

	return (item);
}

/*! Lors d'un double-clic, bouge la cam�ra du viewport vers la donn�e s�lectionn�e*/
void ProjectTreePanel::treeViewMoveProgress(const QModelIndex& doubleClickedInd)
{
	TreeNode* node = static_cast<TreeNode*>(m_model->itemFromIndex(doubleClickedInd));
	if(node->getData())
		m_dataDispatcher.updateInformation(new GuiDataMoveToData(node->getData()));
}

void ProjectTreePanel::treeSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QModelIndexList list = this->selectionModel()->selectedIndexes();
	QModelIndexList newSelectInds = selected.indexes();

	std::unordered_set<SafePtr<AGraphNode>> newSelect;
    for (const QModelIndex& id : list)
    {
        TreeNode* node = static_cast<TreeNode*>(m_model->itemFromIndex(id));
		assert(node != nullptr);
        if (node->getData())
			newSelect.insert(node->getData());
    }

	m_selectedNodes = newSelect;
	m_dataDispatcher.sendControl(new control::special::MultiSelect(newSelect, false));
}

void ProjectTreePanel::collapseChildren(const QModelIndex& collapsInd)
{
	for (int r = 0; r < m_model->itemFromIndex(collapsInd)->rowCount(); ++r)
	{
		QModelIndex childInd = m_model->itemFromIndex(collapsInd)->child(r)->index();
		if (isExpanded(childInd))
			collapse(childInd);
	}
	
}

void ProjectTreePanel::createCluster()
{
	m_dataDispatcher.sendControl(new control::tree::CreateCluster(m_lastTreeNodeSelected->getTreeType(), m_lastTreeNodeSelected->getData()));
    expand(m_lastTreeNodeSelected->index());
}

void ProjectTreePanel::selectClusterChildrens()
{
	QModelIndexList list = this->selectionModel()->selectedIndexes();
	std::unordered_set<SafePtr<AGraphNode>> toSelect;

	for (const QModelIndex& index : list)
	{
		TreeNode* node = static_cast<TreeNode*>(m_model->itemFromIndex(index));
		toSelect.merge(node->getChildren(m_graphManager));
	}

	m_dataDispatcher.sendControl(new control::special::MultiSelect(toSelect, true));
}

void ProjectTreePanel::removeElemFromHierarchy()
{
	QModelIndexList list = this->selectionModel()->selectedIndexes();
	std::list<TreeNode*> nodes;

	for (const QModelIndex& index : list)
	{
		TreeNode* node = static_cast<TreeNode*>(m_model->itemFromIndex(index));
		if (node->getType() != ElementType::MasterCluster)
			nodes.push_back(node);
	}
	if (nodes.size() > 0)
		m_dataDispatcher.sendControl(new control::tree::RemoveElemFromHierarchy(nodes));
}

void ProjectTreePanel::disconnectPiping()
{
	//m_dataDispatcher.sendControl(new control::function::piping::DisconnectPiping(m_lastTreeNodeSelected->getDataId()));
}

void ProjectTreePanel::moveToItem()
{
	m_dataDispatcher.updateInformation(new GuiDataMoveToData(m_lastTreeNodeSelected->getData()));
}

void ProjectTreePanel::enableClipping()
{
	m_dataDispatcher.sendControl(new control::clippingEdition::SetClipActive(true, false, true));
}

void ProjectTreePanel::disableClipping()
{
	m_dataDispatcher.sendControl(new control::clippingEdition::SetClipActive(false, false, true));
}

void ProjectTreePanel::disableAllClippings()
{
	m_dataDispatcher.sendControl(new control::clippingEdition::SetClipActive(false, true, false));
}

void ProjectTreePanel::disableAllRamps()
{
	m_dataDispatcher.sendControl(new control::clippingEdition::SetRampActive(false, true, false));
}

void ProjectTreePanel::externCliping()
{
	m_dataDispatcher.sendControl(new control::clippingEdition::SetMode(ClippingMode::showExterior, false, true));
}

void ProjectTreePanel::internClipping()
{
	m_dataDispatcher.sendControl(new control::clippingEdition::SetMode(ClippingMode::showInterior, false, true));
}

void ProjectTreePanel::pickItems()
{
	QModelIndexList list = this->selectionModel()->selectedIndexes();

	for (const QModelIndex& index : list)
	{
		TreeNode* node = static_cast<TreeNode*>(m_model->itemFromIndex(index));
		SafePtr<AGraphNode> data = node->getData();
		if (node != nullptr && m_pickedItems.find(data) == m_pickedItems.end())
			m_pickedItems.insert(data);
	}


	if (m_lastTreeNodeSelected != nullptr)
	{
		SafePtr<AGraphNode> data = m_lastTreeNodeSelected->getData();
		if(m_pickedItems.find(data) == m_pickedItems.end())
			m_pickedItems.insert(data);
	}
}

void ProjectTreePanel::dropManualItems()
{
	QModelIndex index = this->indexAt(m_lastPoint);
	if (index.isValid() == false)
	{
		m_dropList.clear();
		return;
	}

	TreeNode *destNode = static_cast<TreeNode*>(m_model->itemFromIndex(index));

	for (const SafePtr<AGraphNode>& data : m_pickedItems)
		if (checkDropAreaIsValid(destNode, data) == true)
			m_dropList.push_back(data);
	m_pickedItems.clear();
	dropElem(destNode);
}

void ProjectTreePanel::dropElem(TreeNode* dest)
{
	std::unordered_set<SafePtr<AGraphNode>> datas;
	for (const SafePtr<AGraphNode>& data : m_dropList)
	{
		if (data)
			datas.insert(data);
	}
	dest->dropControl(m_dataDispatcher, datas);
}

/*! Supprime les objets s�lectionn�s */
void ProjectTreePanel::deleteTreeElement()
{
	m_dataDispatcher.sendControl(new control::special::DeleteSelectedElements(true));
}

void ProjectTreePanel::multiChangeAttributes()
{
	m_dataDispatcher.updateInformation(new GuiDataMultiObjectProperties(m_selectedNodes));
}
void recNodeToggle(TreeNode* node, GraphManager& graphManager, std::unordered_set<SafePtr<AGraphNode>>& nodeToToggle)
{

	SafePtr<AGraphNode> nodeData = node->getData();
	if (nodeData)
		nodeToToggle.insert(nodeData);
	else
	{
		std::unordered_set<SafePtr<AGraphNode>> children = node->getChildren(graphManager);
		if (!children.empty())
			nodeToToggle.merge(children);
		else
		{
			for (int r = 0; r < node->rowCount(); ++r)
			{
				TreeNode* child = static_cast<TreeNode*>(node->child(r));
				recNodeToggle(child, graphManager, nodeToToggle);
			}
		}
	}
}

void ProjectTreePanel::onTreeDataChanged(QStandardItem* item)
{
    // NOTE(robin) - On ne peut pas informer le modèle d'un état partiellement visible
    if (item->checkState() == Qt::CheckState::PartiallyChecked)
        return;

	blockAllSignals(true);

    QModelIndexList list = this->selectionModel()->selectedIndexes();
    bool state = (item->checkState() == Qt::CheckState::Checked) ? true : false;
    // Choose between the selected items and the item changed (if not selected)
    if (list.contains(item->index()) == false)
    {
        list.clear();
        list.push_back(item->index());
    }

	std::unordered_set<SafePtr<AGraphNode>> nodeToToggle;
    for (const QModelIndex& index : list)
    {
        TreeNode* node = static_cast<TreeNode*>(m_model->itemFromIndex(index));
		recNodeToggle(node, m_graphManager, nodeToToggle);
    }

	m_dataDispatcher.sendControl(new control::special::ShowHideDatas(nodeToToggle, state, true));

	blockAllSignals(false);
}

void ProjectTreePanel::treeDragEvent(QDragEnterEvent* dEvent)
{
	m_dropList.clear();

	for (const SafePtr<AGraphNode>& data : m_selectedNodes)
	{
		if (!data)
		{
			dEvent->setAccepted(false);
			m_dropList.clear();
			return;
		}
		m_dropList.push_back(data);
	}

	m_highlightNode = nullptr;
	dEvent->accept();
}

void ProjectTreePanel::treeMoveEvent(QDragMoveEvent* mevent)
{
    QModelIndex highlightIndex = this->indexAt(mevent->pos());

    TreeNode* destNode = static_cast<TreeNode*>(m_model->itemFromIndex(highlightIndex));
    bool canDrop = true;

    for (const SafePtr<AGraphNode>& data : m_dropList)
    {
        if (checkDropAreaIsValid(destNode, data) == false)
        {
            canDrop = false;
            break;
        }
    }
    mevent->setAccepted(canDrop);
}

void ProjectTreePanel::treeDropEvent(QDropEvent* dEvent)
{
    TreeNode* destNode = static_cast<TreeNode*>(m_model->itemFromIndex(this->indexAt(dEvent->pos())));

    bool canDrop = true;

    for (const SafePtr<AGraphNode>& data : m_dropList)
    {
        if (checkDropAreaIsValid(destNode, data) == false)
        {
            canDrop = false;
            break;
        }
    }
    dEvent->setAccepted(false);
    if (canDrop == true)
        dropElem(destNode);
}

/*! Vérifie si le noeud _origin_ peut �tre avoir comme parent _dest_ (retourne 'True' si oui, sinon 'False*/
bool ProjectTreePanel::checkDropAreaIsValid(TreeNode* dest, const SafePtr<AGraphNode>& dropData)
{
	// verify that the user do not try to D&D a object in itself
	if (dropData == dest->getData())
		return false;

	if (AGraphNode::isOwningAncestor(dropData, dest->getData()))
		return false;

	return(dest->canDataDrop(dropData));
}

void ProjectTreePanel::onExpand(const QModelIndex& ind)
{
	TreeNode* expandNode = static_cast<TreeNode*>(m_model->itemFromIndex(ind));
	m_nodeFactory->updateNode(expandNode);
}

void ProjectTreePanel::onCollapse(const QModelIndex& ind)
{
	TreeNode* expandNode = static_cast<TreeNode*>(m_model->itemFromIndex(ind));
	m_nodeFactory->updateNode(expandNode);
}

/*! G�n�re le menu contextuel appara�sant selon le point QPoint p cliqu� */
void ProjectTreePanel::showTreeMenu(QPoint p)
{
	QModelIndex index = this->indexAt(p);
	if (index.isValid() == false)
		return;
	m_lastTreeNodeSelected = static_cast<TreeNode*>(m_model->itemFromIndex(index));
	// FIXME(robin) - Check memory leak
	QMenu* menu = new QMenu(this);
	menu->setAttribute(Qt::WA_DeleteOnClose);

	auto addActionToMenu = [this](QMenu* menu, QAction* action, void (ProjectTreePanel::*funct)())
	{
		menu->addAction(action);
		QObject::connect(action, &QAction::triggered, this, funct);
		return;
	};

	// Create Cluster
	if (m_lastTreeNodeSelected->getType() == ElementType::Cluster || m_lastTreeNodeSelected->getType() == ElementType::MasterCluster)
	{
		addActionToMenu(menu, new QAction(TEXT_CONTEXT_NEW_CLUSTER, this), &ProjectTreePanel::createCluster);
		addActionToMenu(menu, new QAction(TEXT_CONTEXT_SELECT_ITEMS, this), &ProjectTreePanel::selectClusterChildrens);
	}
	
	// Move to Item
	if (m_lastTreeNodeSelected->getType() != ElementType::Cluster && m_lastTreeNodeSelected->getType() != ElementType::MasterCluster 
		&& m_lastTreeNodeSelected->getType() != ElementType::None)
	{
		addActionToMenu(menu, new QAction(TEXT_CONTEXT_MOVE_TO_ITEM, this), &ProjectTreePanel::moveToItem);
	}
	// Picked Selected
	if (m_lastTreeNodeSelected->getType() != ElementType::MasterCluster && m_lastTreeNodeSelected->getType() != ElementType::None)
	{
		addActionToMenu(menu, new QAction(TEXT_CONTEXT_PICK_ITEMS, this), &ProjectTreePanel::pickItems);
	}
	// Drop Selected
	if ( m_lastTreeNodeSelected->isDropEnabled() && 
		(m_lastTreeNodeSelected->getType() == ElementType::Cluster || m_lastTreeNodeSelected->getType() == ElementType::MasterCluster 
			|| m_lastTreeNodeSelected->getType() == ElementType::None))
	{
		QAction* dropAct = new QAction(TEXT_CONTEXT_DROP_ITEMS, this);
		m_lastPoint = p;
		if (m_pickedItems.empty())
			dropAct->setEnabled(false);
		addActionToMenu(menu, dropAct, &ProjectTreePanel::dropManualItems);
	}

	
	// Clipping enable, disable, show intern, show extern
	if (m_lastTreeNodeSelected->getType() == ElementType::Box ||
		m_lastTreeNodeSelected->getType() == ElementType::Grid ||
		m_lastTreeNodeSelected->getType() == ElementType::Tag ||
		m_lastTreeNodeSelected->getType() == ElementType::Point ||
		m_lastTreeNodeSelected->getType() == ElementType::SimpleMeasure ||
		m_lastTreeNodeSelected->getType() == ElementType::PolylineMeasure ||
		m_lastTreeNodeSelected->getType() == ElementType::Cylinder ||
		m_lastTreeNodeSelected->getType() == ElementType::Sphere ||
		m_lastTreeNodeSelected->getType() == ElementType::Torus)
	{
		addActionToMenu(menu, new QAction(TEXT_CONTEXT_ENABLE_CLIPPING, this), &ProjectTreePanel::enableClipping);
		addActionToMenu(menu, new QAction(TEXT_CONTEXT_DISABLE_CLIPPING, this), &ProjectTreePanel::disableClipping);
		addActionToMenu(menu, new QAction(TEXT_CONTEXT_INTERIOR_CLIPPING, this), &ProjectTreePanel::internClipping);
		addActionToMenu(menu, new QAction(TEXT_CONTEXT_EXTERIOR_CLIPPING, this), &ProjectTreePanel::externCliping);
	}

	// Desactivate all clippings
	if (m_lastTreeNodeSelected == m_itemsNode) 
	{
		addActionToMenu(menu, new QAction(TEXT_CONTEXT_DISABLE_ALL_CLIPPING, this), &ProjectTreePanel::disableAllClippings);
		addActionToMenu(menu, new QAction(TEXT_CONTEXT_DISABLE_ALL_RAMPS, this), &ProjectTreePanel::disableAllRamps);
	}

	// Disconnect Line
	/*if (m_lastTreeNodeSelected->getType() == ElementType::Piping) {
		QAction* disconnectAct = new QAction(TEXT_CONTEXT_DISCONNECT_LINE, this);
		disconnectAct->setToolTip(TEXT_TOOLTIP_DISCONNECT_LINE);
		addActionToMenu(menu, disconnectAct, &ProjectTreePanel::disconnectPiping);
	}*/

	// Remove From hierarchy
	if (m_lastTreeNodeSelected->getTreeType() == TreeType::Hierarchy
		&& m_lastTreeNodeSelected->getType() != ElementType::MasterCluster)
	{
		addActionToMenu(menu, new QAction(TEXT_CONTEXT_REMOVE_ITEM, this), &ProjectTreePanel::removeElemFromHierarchy);
	}

	// Delete Cluster - Delete Items
	if (m_lastTreeNodeSelected->getType() == ElementType::Cluster)
	{
		addActionToMenu(menu, new QAction(TEXT_CONTEXT_DELETE_CLUSTER_ITEM, this), &ProjectTreePanel::deleteTreeElement);
	}

	// Export Scan
	if (m_lastTreeNodeSelected->getType() == ElementType::Scan || m_lastTreeNodeSelected->getType() == ElementType::PCO)
		addActionToMenu(menu, new QAction(TEXT_CONTEXT_EXPORT_SCAN, this), &ProjectTreePanel::exportScan);

	// Delete Item
	if (m_lastTreeNodeSelected->getType() != ElementType::Cluster
		&& m_lastTreeNodeSelected->getType() != ElementType::None && m_lastTreeNodeSelected->getType() != ElementType::MasterCluster)
	{
		addActionToMenu(menu, new QAction(TEXT_CONTEXT_DELETE_ITEM, this), &ProjectTreePanel::deleteTreeElement);
		if (m_selectedNodes.size() > 1)
			addActionToMenu(menu, new QAction(TEXT_CONTEXT_CHANGE_ATTRIBUTES, this), &ProjectTreePanel::multiChangeAttributes);
	}

	menu->popup(viewport()->mapToGlobal(p));

	GTLOG << "show tree menu on type : " << magic_enum::enum_name<ElementType>(m_lastTreeNodeSelected->getType()) << LOGENDL;
}

/*! Red�finition de la m�thode Qt qui traite l'�venement de 'Drag' (correspond au d�but du Drag&Drop) */
void ProjectTreePanel::dragEnterEvent(QDragEnterEvent* qevent)
{
	QModelIndex index = indexAt(qevent->pos());

	if (index.isValid() == false)
	{
		qevent->setDropAction(Qt::IgnoreAction);
		return;
	}

    treeDragEvent(qevent);
}

/*! Red�finition de la m�thode Qt qui traite l'�venement de 'DragMove' (se r�p�te entre un �v�nement de 'Drag' et 'Drop') */
void ProjectTreePanel::dragMoveEvent(QDragMoveEvent* mevent)
{
	QModelIndex index = indexAt(mevent->pos());

	if (index.isValid() == false)
	{
		mevent->setDropAction(Qt::IgnoreAction);
		return;
	}

    treeMoveEvent(mevent);
}

/*! Red�finition d'une m�thode Qt pour traiter l'�venement de 'Drop' (correspond � la fin du Drag&Drop) */
void ProjectTreePanel::dropEvent(QDropEvent* dEvent)
{
    QModelIndex index = indexAt(dEvent->pos());

    if (index.isValid() == false)
    {
        dEvent->setDropAction(Qt::IgnoreAction);
        return;
    }

    treeDropEvent(dEvent);
}

void ProjectTreePanel::exportScan()
{
	QModelIndexList list = this->selectionModel()->selectedIndexes();
	std::unordered_set<SafePtr<APointCloudNode>> pcsToExport;

	for (const QModelIndex& index : list)
	{
		TreeNode* node = static_cast<TreeNode*>(m_model->itemFromIndex(index));
		if (node->getData())
			pcsToExport.insert(static_pointer_cast<APointCloudNode>(node->getData()));
	}
	if (pcsToExport.empty())
		return;

	ExportInitMessage message(false, false, true, true, ObjectStatusFilter::SELECTED);
	m_dataDispatcher.sendControl(new control::exportPC::StartExport(message));
}
