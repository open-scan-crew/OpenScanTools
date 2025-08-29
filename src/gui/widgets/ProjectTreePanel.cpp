#include "gui/widgets/ProjectTreePanel.h"

#include "controller/controls/ControlTree.h"
#include "controller/controls/ControlSpecial.h"
#include "controller/controls/ControlExportPC.h"
#include "controller/controls/ControlClippingEdition.h"
#include "controller/controls/ControlDataEdition.h"

#include "gui/IDataDispatcher.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiData3dObjects.h"
#include "gui/GuiData/GuiDataTree.h"
#include "gui/widgets/TreeNodeSystem/TreeNode.h"
#include "gui/Texts.hpp"
#include "gui/texts/TreePanelTexts.hpp"
#include "gui/style/IconObject.h"

#include "models/graph/ClusterNode.h"
#include "models/graph/APointCloudNode.h"
#include "models/graph/GraphManager.h"
#include "models/graph/GraphManager.hxx"
#include "models/graph/TagNode.h"

#include "models/3d/NodeFunctions.h"

#include "utils/Logger.h"
#include "utils/ProjectColor.hpp"

#include <QtWidgets/qscrollbar.h>
#include <QtWidgets/qmenu.h>
#include <QtGui/qevent.h>

#include "magic_enum/magic_enum.hpp"

#define GTLOG Logger::log(LoggerMode::GTLog)
#define GTELOG Logger::log(LoggerMode::GTExtraLog)

ProjectTreePanel::ProjectTreePanel(IDataDispatcher& dataDispatcher, GraphManager& graphManager, float guiScale)
    : QTreeView()
    , m_dataDispatcher(dataDispatcher)
    , m_graphManager(graphManager)
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
            updateNode(node);
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
        // Suppression de la paire <SafePtr<AGraphNode> , std::vector<TreeNode*>> dans la map si il n'y a plus de TreeNode associÃ©
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

void ProjectTreePanel::addRootToUpdateSystem(TreeNode* node, TreeType treeType)
{
    std::unordered_map<TreeType, std::vector<TreeNode*>>& rootNodes = m_rootNodes;
    if (rootNodes.find(treeType) == rootNodes.end())
        rootNodes[treeType] = std::vector<TreeNode*>();
    rootNodes[treeType].push_back(node);
}

void ProjectTreePanel::updateSelection(const std::unordered_set<SafePtr<AGraphNode>>& datas)
{
    // On vide la liste Qt de sï¿½lection
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

    TreeNode* hierarchyTreeNode = constructTreeNode(hierarchyNode, nullptr, TreeType::Hierarchy);
    hierarchyTreeNode->setText(TEXT_HIERARCHY_TREE_NODE);
    hierarchyTreeNode->setType(ElementType::MasterCluster);
    hierarchyTreeNode->setSelectable(true);
    hierarchyTreeNode->setCheckable(true);
    hierarchyTreeNode->setCheckState(Qt::CheckState::Checked);

    m_model->setItem(0, hierarchyTreeNode);
    addRootToUpdateSystem(hierarchyTreeNode, TreeType::Hierarchy);

    m_itemsNode = constructMasterNode(TEXT_ITEMS_TREE_NODE, TreeType::RawData);
    m_model->setItem(1, m_itemsNode);
    m_itemsNode->setCheckable(true);
    m_itemsNode->setCheckState(Qt::CheckState::Checked);

    TreeNode* viewpointsAttrNode = constructMasterNode(TEXT_VIEWPOINTS_TREE_NODE, TreeType::ViewPoint);

    TreeNode* scansAttrNode = buildTreeModelBranch(TEXT_SCANS_TREE_NODE, TreeType::Scan);

    TreeNode* tagsAttrNode = constructMasterNode(TEXT_TAGS_TREE_NODE, TreeType::Tags);

    TreeNode* boxesAttrNode = constructMasterNode(TEXT_BOXES_TREE_NODE, TreeType::Boxes);

    TreeNode* measureAttrNode = constructMasterNode(TEXT_MEASURES_TREE_NODE, TreeType::Measures);

    TreeNode* pipesTree = constructMasterNode(TEXT_PIPES_TREE_NODE, TreeType::Pipe);

    TreeNode* spheresTree = constructMasterNode(TEXT_SPHERES_TREE_NODE, TreeType::Sphere);

    TreeNode* pointsTree = constructMasterNode(TEXT_POINT_TREE_NODE, TreeType::Point);

    TreeNode* pcObjsTree = constructMasterNode(TEXT_PC_OBJECT_TREE_NODE, TreeType::Pco);

    TreeNode* objsTree = constructMasterNode(TEXT_OBJ_OBJECT_TREE_NODE, TreeType::MeshObjects);

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

    constructColorNodes({ ElementType::ViewPoint }, viewpointsAttrNode);

    constructColorNodes({ ElementType::Tag }, tagsAttrNode);
    constructStatusNodes({ ElementType::Tag }, tagsAttrNode);
    constructClipMethodNodes({ ElementType::Tag }, tagsAttrNode);

    addBoxesSubList(boxesAttrNode, TEXT_SIMPLE_BOX_SUB_NODE, true);
    addBoxesSubList(boxesAttrNode, TEXT_GRID_SUB_NODE, false);

    constructStatusNodes({ ElementType::Box }, boxesAttrNode);
    constructClipMethodNodes({ ElementType::Box }, boxesAttrNode);
    constructColorNodes({ ElementType::Box }, boxesAttrNode);

    TreeNode* simple_m = addSubList(measureAttrNode, TEXT_SIMPLE_MEASURE, ElementType::SimpleMeasure);
    TreeNode* poly_m = addSubList(measureAttrNode, TEXT_POLYLINE_MEASURE, ElementType::PolylineMeasure);
    TreeNode* ctm = addSubList(measureAttrNode, TEXT_COLUMNTILT_MEASURE, ElementType::ColumnTiltMeasure);
    TreeNode* bbm = addSubList(measureAttrNode, TEXT_BEAMBENDING_MEASURE, ElementType::BeamBendingMeasure);
    TreeNode* pipe_to_pipe = addSubList(measureAttrNode, TEXT_PIPETOPIPE_MEASURE, ElementType::PipeToPipeMeasure);
    TreeNode* pipe_to_plane = addSubList(measureAttrNode, TEXT_PIPETOPLANE_MEASURE, ElementType::PipeToPlaneMeasure);
    TreeNode* point_to_plane = addSubList(measureAttrNode, TEXT_POINTTOPLANE_MEASURE, ElementType::PointToPlaneMeasure);
    TreeNode* point_to_pipe = addSubList(measureAttrNode, TEXT_POINTTOPIPE_MEASURE, ElementType::PointToPipeMeasure);

    constructStatusNodes({ ElementType::SimpleMeasure }, simple_m);
    constructClipMethodNodes({ ElementType::SimpleMeasure }, simple_m);

    constructStatusNodes({ ElementType::PolylineMeasure }, poly_m);
    constructClipMethodNodes({ ElementType::PolylineMeasure }, poly_m);

    TreeNode* cylinder = addSubList(pipesTree, TEXT_CYLINDER_SUB_NODE, ElementType::Cylinder);
    TreeNode* torus = addSubList(pipesTree, TEXT_TORUS_SUB_NODE, ElementType::Torus);

    constructStatusNodes({ ElementType::Torus, ElementType::Cylinder }, pipesTree);
    constructClipMethodNodes({ ElementType::Torus, ElementType::Cylinder }, pipesTree);
    constructColorNodes({ ElementType::Torus, ElementType::Cylinder }, pipesTree);

    constructColorNodes({ ElementType::Point }, pointsTree);
    constructStatusNodes({ ElementType::Point }, pointsTree);
    constructClipMethodNodes({ ElementType::Point }, pointsTree);

    constructStatusNodes({ ElementType::Sphere }, spheresTree);
    constructClipMethodNodes({ ElementType::Sphere }, spheresTree);
    constructColorNodes({ ElementType::Sphere }, spheresTree);

    constructColorNodes({ ElementType::PCO }, pcObjsTree);
    constructStatusNodes({ ElementType::PCO }, pcObjsTree);

    constructColorNodes({ ElementType::MeshObject }, objsTree);

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

    TreeNode::fctGetChildren getChildFonction =
        [filter](const GraphManager& manager)
    {
        return manager.getNodesOnFilter(filter);
    };

    TreeNode::fctCanDrop canDrop =
        [treetype](const SafePtr<AGraphNode>& data)
    {
        ReadPtr<AGraphNode> rData = data.cget();
        if (!rData)
            return false;
        return (rData->getDefaultTreeType() == treetype);
    };

    TreeNode::fctOnDrop onDrop =
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

    addRootToUpdateSystem(item, treetype);

    return (item);
}

TreeNode* ProjectTreePanel::constructMasterNode(const QString& name, TreeType treeType)
{
    TreeNode* newItem = new TreeNode(SafePtr<AGraphNode>(),
        [](const GraphManager& manager) { return std::unordered_set<SafePtr<AGraphNode>>(); },
        [](const SafePtr<AGraphNode>& data) { return false; },
        [](IDataDispatcher&, std::unordered_set<SafePtr<AGraphNode>>) { return; },
        treeType);

    newItem->setText(name);
    newItem->setCheckable(false);
    newItem->setDropEnabled(false);
    newItem->setStructNode(true);

    return (newItem);
}

TreeNode* ProjectTreePanel::constructPlaceholderNode(TreeType treeType)
{
    TreeNode* newItem = new TreeNode(SafePtr<AGraphNode>(),
        [](const GraphManager& manager) { return std::unordered_set<SafePtr<AGraphNode>>(); },
        [](const SafePtr<AGraphNode>& data) { return false; },
        [](IDataDispatcher&, std::unordered_set<SafePtr<AGraphNode>>) { return; },
        treeType);

    newItem->setText(TEXT_PLACEHOLDER_CHILDREN);
    newItem->setDropEnabled(false);
    newItem->setCheckable(false);
    newItem->setStructNode(false);

    return (newItem);
}

TreeNode* ProjectTreePanel::constructStructNode(const QString& name,
    TreeNode::fctGetChildren getChildFct,
    TreeNode::fctCanDrop dropFilter,
    TreeNode::fctOnDrop f_on_drop,
    TreeNode* parent)
{
    TreeType treetype = TreeType::RawData;
    if (parent != nullptr)
        treetype = parent->getTreeType();

    TreeNode* newNode = new TreeNode(SafePtr<AGraphNode>(), getChildFct, dropFilter, f_on_drop, treetype);
    newNode->setStructNode(true);
    newNode->setText(name);
    newNode->setCheckable(true);
    if (parent != nullptr)
        parent->appendRow(newNode);

    addRootToUpdateSystem(newNode, treetype);

    return (newNode);
}

TreeNode* ProjectTreePanel::addSubList(TreeNode* parentNode, QString name, ElementType type)
{
    TreeNode::fctGetChildren getChildFonction =
        [type](const GraphManager& manager)
        {
            return manager.getNodesByTypes({ type });
        };

    TreeNode* element = constructStructNode(name,
        getChildFonction,
        [](const SafePtr<AGraphNode>& data) { return false; },
        [](IDataDispatcher&, std::unordered_set<SafePtr<AGraphNode>> data) { return; },
        parentNode);

    return element;
}

TreeNode* ProjectTreePanel::addBoxesSubList(TreeNode* parentNode, QString name, bool only_box)
{
    std::function<bool(ReadPtr<AGraphNode>& rData)> childCondition =
        [only_box](ReadPtr<AGraphNode>& rData)
        {
            if (rData->getType() != ElementType::Box)
                return (false);
            const BoxNode* p_box = static_cast<const BoxNode*>(&rData);
            return (only_box == p_box->isSimpleBox());
        };

    TreeNode::fctGetChildren getChildFct =
        [childCondition](const GraphManager& manager)
        {
            return manager.getNodesOnFilter<AGraphNode>(childCondition);
        };

    TreeNode::fctCanDrop canDrop = [](const SafePtr<AGraphNode>& data) { return false; };

    TreeNode::fctOnDrop onDrop = [](IDataDispatcher&, std::unordered_set<SafePtr<AGraphNode>> data) { return; };

    // constructStructNode
    TreeNode* treeNode = new TreeNode(SafePtr<AGraphNode>(), getChildFct, canDrop, onDrop, TreeType::Boxes);
    treeNode->setText(name);
    treeNode->setStructNode(true);
    treeNode->setCheckable(true);
    treeNode->setCheckState(Qt::CheckState::Checked);

    if (parentNode != nullptr)
        parentNode->appendRow(treeNode);

    addRootToUpdateSystem(treeNode, TreeType::Boxes);

    return treeNode;
}

TreeNode* ProjectTreePanel::constructColorNodes(const std::unordered_set<ElementType>& types, TreeNode* parentNode)
{
    std::vector<std::pair<QString, Color32>> colors = {
        { TEXT_GREEN, ProjectColor::getColor("GREEN") },
        { TEXT_RED, ProjectColor::getColor("RED") },
        { TEXT_ORANGE, ProjectColor::getColor("ORANGE") },
        { TEXT_YELLOW, ProjectColor::getColor("YELLOW") },
        { TEXT_BLUE, ProjectColor::getColor("BLUE") },
        { TEXT_PURPLE, ProjectColor::getColor("PURPLE") },
        { TEXT_LIGHT_GREY, ProjectColor::getColor("LIGHT GREY") },
        { TEXT_BROWN, ProjectColor::getColor("BROWN") }
    };

    TreeNode* ColorNodes = constructMasterNode(TEXT_COLORS_TREE_NODE, parentNode->getTreeType());

    parentNode->appendRow(ColorNodes);

    for (std::pair<QString, Color32> nameColor : colors)
    {
        QString colorName = nameColor.first;
        Color32 color = nameColor.second;

        std::function<bool(ReadPtr<AGraphNode>& rData)> childCondition =
            [color, types](ReadPtr<AGraphNode>& rData)
            {
                if (types.find(rData->getType()) == types.end())
                    return (false);
                if (rData->getColor() == color)
                    return (true);
                return (false);
            };
        std::function<std::unordered_set<SafePtr<AGraphNode>>(const GraphManager&)> getChildFonction =
            [childCondition](const GraphManager& manager)
            {
                return manager.getNodesOnFilter<AGraphNode>(childCondition);
            };

        std::function<bool(const SafePtr<AGraphNode>& data)> canDrop = [types](const SafePtr<AGraphNode>& data)
            {
                ReadPtr<AGraphNode> rData = data.cget();
                if (!rData)
                    return false;
                if (types.find(rData->getType()) != types.end())
                    return (true);
                return (false);
            };

        std::function<void(IDataDispatcher& dispatcher, std::unordered_set<SafePtr<AGraphNode>>)> onDrop =
            [color](IDataDispatcher& dispatcher, std::unordered_set<SafePtr<AGraphNode>> datas)
            {
                dispatcher.sendControl(new control::dataEdition::SetColor(datas, color));
            };

        TreeNode* colorStruct = constructStructNode(colorName, getChildFonction, canDrop, onDrop, ColorNodes);
    }

    return (ColorNodes);
}

TreeNode* ProjectTreePanel::constructStatusNodes(const std::unordered_set<ElementType>& types, TreeNode* parentNode)
{
    std::vector<bool> status = { false, true };

    TreeNode* parentStatusNode = constructMasterNode(TEXT_STATUS_TREE_NODE, parentNode->getTreeType());

    parentNode->appendRow(parentStatusNode);

    for (bool state : status)
    {
        std::function<bool(const SafePtr<AGraphNode>& data)> childCondition =
            [state, types](const SafePtr<AGraphNode>& data)
            {
                {
                    ReadPtr<AGraphNode> rData = data.cget();
                    if (!rData)
                        return false;
                    if (types.find(rData->getType()) == types.end())
                        return false;
                }

                {
                    ReadPtr<AClippingNode> rClip = static_pointer_cast<AClippingNode>(data).cget();
                    if (!rClip)
                        return false;
                    return (rClip->isClippingActive() == state);
                }

                return true;
            };

        std::function<std::unordered_set<SafePtr<AGraphNode>>(const GraphManager&)> getChildFonction =
            [childCondition](const GraphManager& manager)
            {
                return manager.getNodesOnFilter(childCondition);
            };

        std::function<bool(const SafePtr<AGraphNode>& data)> canDrop = [types](const SafePtr<AGraphNode>& data)
            {
                ReadPtr<AGraphNode> rData = data.cget();
                if (!rData)
                    return false;
                if (types.find(rData->getType()) != types.end()) return (true);
                return (false);
            };

        std::function<void(IDataDispatcher& dispatcher, std::unordered_set<SafePtr<AGraphNode>>)> onDrop =
            [state](IDataDispatcher& dispatcher, std::unordered_set<SafePtr<AGraphNode>> datas)
            {
                std::unordered_set<SafePtr<AClippingNode>> clips;
                for (const SafePtr<AGraphNode>& data : datas)
                    clips.insert(static_pointer_cast<AClippingNode>(data));
                dispatcher.sendControl(new control::clippingEdition::SetClipActive(clips, state));
            };

        TreeNode* statusNode = constructStructNode(state ? TEXT_ACTIVE : TEXT_INACTIVE, getChildFonction, canDrop, onDrop, parentStatusNode);
    }

    return (parentStatusNode);
}

TreeNode* ProjectTreePanel::constructClipMethodNodes(const std::unordered_set<ElementType>& types, TreeNode* parentNode)
{
    std::vector<std::pair<QString, ClippingMode>> clipModes = { {TEXT_CLIPINTERN, ClippingMode::showInterior},
                                                                {TEXT_CLIPEXTERN, ClippingMode::showExterior}
    };

    TreeNode* clipMethNode = constructMasterNode(TEXT_CLIPMETH_TREE_NODE, parentNode->getTreeType());

    parentNode->appendRow(clipMethNode);

    for (std::pair<QString, ClippingMode> nameClipMode : clipModes)
    {
        QString name = nameClipMode.first;
        ClippingMode clipMode = nameClipMode.second;

        std::function<bool(const SafePtr<AGraphNode>& data)> childCondition =
            [clipMode, types](const SafePtr<AGraphNode>& data)
            {
                {
                    ReadPtr<AGraphNode> rData = data.cget();
                    if (!rData)
                        return false;
                    if (types.find(rData->getType()) == types.end())
                        return false;
                }

                {
                    ReadPtr<AClippingNode> rClip = static_pointer_cast<AClippingNode>(data).cget();
                    if (!rClip)
                        return false;
                    return (rClip->getClippingMode() == clipMode);
                }

                return true;
            };

        std::function<std::unordered_set<SafePtr<AGraphNode>>(const GraphManager&)> getChildFonction =
            [childCondition](const GraphManager& manager)
            {
                return manager.getNodesOnFilter(childCondition);
            };

        std::function<bool(const SafePtr<AGraphNode>& data)> canDrop = [types](const SafePtr<AGraphNode>& data)
            {
                ReadPtr<AGraphNode> rData = data.cget();
                if (!rData)
                    return false;
                if (types.find(rData->getType()) != types.end())
                    return (true);
                return (false);
            };

        std::function<void(IDataDispatcher& dispatcher, std::unordered_set<SafePtr<AGraphNode>>)> onDrop =
            [clipMode](IDataDispatcher& dispatcher, std::unordered_set<SafePtr<AGraphNode>> datas)
            {
                std::unordered_set<SafePtr<AClippingNode>> clips;
                for (const SafePtr<AGraphNode>& data : datas)
                    clips.insert(static_pointer_cast<AClippingNode>(data));
                dispatcher.sendControl(new control::clippingEdition::SetMode(clips, clipMode));
            };

        TreeNode* CMNode = constructStructNode(name, getChildFonction, canDrop, onDrop, clipMethNode);
    }

    return (clipMethNode);
}


void ProjectTreePanel::updateNode(TreeNode* node)
{
    QStandardItem* parent = node->parent();
    if (parent && !isExpanded(parent->index()))
        return;

    SafePtr<AGraphNode> data = node->getData();
    ElementType type;
    std::wstring name;
    Color32 color;
    scs::MarkerIcon icon;
    bool isVisible;
    bool isSelected;

    if (data)
    {
        {
            ReadPtr<AGraphNode> rdata = data.cget();
            if (!rdata)
                return;

            isVisible = rdata->isVisible();
            type = rdata->getType();
            color = rdata->getColor();
            icon = rdata->getMarkerIcon();
            name = rdata->getComposedName();
            isSelected = rdata->isSelected();
        }

        if (nodeFunctions::isMissingFile(data))
            name = L"?" + name;

        blockAllSignals(true);

        node->setText(QString::fromStdWString(name));
        node->setIcon(scs::IconManager::getInstance().getIcon(icon, color));
        if (node->isCheckable())
            node->setCheckState(isVisible ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

        QItemSelectionModel* selectModel = selectionModel();
        if (selectModel)
            selectModel->setCurrentIndex(node->index(), isSelected ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);

        if (isSelected)
            m_selectedNodes.insert(data);
        else
            m_selectedNodes.erase(data);

        blockAllSignals(false);
    }

    bool is_expanded = isExpanded(node->index());

    std::unordered_map<SafePtr<AGraphNode>, TreeNode*> oldChildren;

    std::unordered_set<TreeNode*> nodesToUpdate;
    std::unordered_set<TreeNode*> nodesToDelete;

    bool toSort = true;

    if (is_expanded)
    {
        for (int i = node->rowCount(); i > 0; i--)
        {
            TreeNode* child = static_cast<TreeNode*>(node->child(i - 1));
            SafePtr<AGraphNode> dataChild = child->getData();
            if (dataChild)
                oldChildren.insert({ dataChild, child });
            else
            {
                if (child->isStructNode())
                {
                    toSort = false;
                    nodesToUpdate.insert(child);
                }
                else
                    nodesToDelete.insert(child);
            }
        }
    }

    std::unordered_set<SafePtr<AGraphNode>> newChildren = node->getChildren(m_graphManager);
    if (is_expanded)
    {
        for (const SafePtr<AGraphNode>& child : newChildren)
        {
            if (oldChildren.find(child) == oldChildren.end())
                TreeNode* tChild = constructTreeNode(child, node, node->getTreeType());
            else
            {
                TreeNode* treeNode = oldChildren.at(child);
                nodesToUpdate.insert(treeNode);
            }
        }

        for (const auto& pair : oldChildren)
        {
            if (!pair.second->isStructNode() && newChildren.find(pair.first) == newChildren.end())
                nodesToDelete.insert(pair.second);
        }
    }
    else
    {
        for (int i = node->rowCount(); i > 0; i--)
        {
            TreeNode* child = static_cast<TreeNode*>(node->child(i - 1));
            if (!child->isStructNode())
                nodesToDelete.insert(child);
        }
        if (!newChildren.empty())
            node->appendRow(constructPlaceholderNode(node->getTreeType()));
    }

    for (TreeNode* upNode : nodesToUpdate)
        updateNode(upNode);

    for (TreeNode* delNode : nodesToDelete)
        removeTreeNode(delNode);

    blockAllSignals(true);

    if (toSort && is_expanded)
        node->sortChildren(0);

    if (node->isCheckable())
        node->setCheckState(recGetNodeState(node));

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

/*! Vérifie si le noeud _origin_ peut ï¿½tre avoir comme parent _dest_ (retourne 'True' si oui, sinon 'False*/
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
    updateNode(expandNode);
}

void ProjectTreePanel::onCollapse(const QModelIndex& ind)
{
    TreeNode* expandNode = static_cast<TreeNode*>(m_model->itemFromIndex(ind));
    updateNode(expandNode);
}

/*! Lors d'un double-clic, bouge la caméra du viewport vers la donnï¿½e sï¿½lectionnï¿½e*/
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

/*! Supprime les objets sï¿½lectionnï¿½s */
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
    // NOTE(robin) - We cannot inform the model of partial check state
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

/*! Génère le menu contextuel apparaisant selon le point QPoint p cliqué */
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

/*! Redï¿½finition de la mï¿½thode Qt qui traite l'ï¿½venement de 'Drag' (correspond au dï¿½but du Drag&Drop) */
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

/*! Redï¿½finition de la mï¿½thode Qt qui traite l'ï¿½venement de 'DragMove' (se rï¿½pï¿½te entre un ï¿½vï¿½nement de 'Drag' et 'Drop') */
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

/*! Redéfinition d'une méthode Qt pour traiter l'évenement de 'Drop' (correspond à la fin du Drag&Drop) */
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

///////////////////////////////////////

TreeNode* ProjectTreePanel::constructTreeNode(const SafePtr<AGraphNode>& data, TreeNode* parent, TreeType treetype)
{
    std::function<std::unordered_set<SafePtr<AGraphNode>>(const GraphManager&)> getChildFonction =
        [data](const GraphManager& manager)
        {
            std::unordered_set<SafePtr<AGraphNode>> children = AGraphNode::getOwningChildren(data);
            return children;
        };

    std::function<bool(const SafePtr<AGraphNode>& data)> canDrop =
        [data](const SafePtr<AGraphNode>& child)
        {
            ReadPtr<AGraphNode> rData = data.cget();
            if (!rData)
                return false;
            return rData->isAcceptableOwningChild(child);
        };

    std::function<void(IDataDispatcher& dispatcher, std::unordered_set<SafePtr<AGraphNode>>)> onDrop =
        [treetype, data](IDataDispatcher& dispatcher, std::unordered_set<SafePtr<AGraphNode>> datas)
        {
            dispatcher.sendControl(new control::tree::DropElements(data, treetype, datas));
        };

    blockAllSignals(true);

    TreeNode* newNode = new TreeNode(data, getChildFonction, canDrop, onDrop, treetype);

    newNode->setCheckable(true);
    newNode->setStructNode(false);

    if (parent != nullptr)
        parent->appendRow(newNode);

    std::unordered_map<SafePtr<AGraphNode>, std::vector<TreeNode*>>& treenodes = m_model->getTreeNodes();

    blockAllSignals(false);

    updateNode(newNode);

    if (treenodes.find(data) == treenodes.end())
        treenodes.insert({ data, std::vector<TreeNode*>() });
    treenodes[data].push_back(newNode);


    return newNode;
}

Qt::CheckState recStateOnData(const SafePtr<AGraphNode>& data)
{
    bool isVisible = true;
    if (data)
    {
        ReadPtr<AGraphNode> rData = data.cget();
        if (rData)
            isVisible = rData->isVisible();
    }

    Qt::CheckState state = isVisible ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
    bool stateInit = true;

    std::unordered_set<SafePtr<AGraphNode>> nodeChildren = AGraphNode::getOwningChildren(data);
    for (const SafePtr<AGraphNode>& child : nodeChildren)
    {
        Qt::CheckState childState = recStateOnData(child);

        if (stateInit)
        {
            state = childState;
            stateInit = false;
        }
        else
        {
            if (childState != state)
                state = Qt::CheckState::PartiallyChecked;
        }
    }

    return state;
}

Qt::CheckState ProjectTreePanel::recGetNodeState(TreeNode* node)
{
    bool isVisible = true;
    SafePtr<AGraphNode> data = node->getData();
    if (data)
    {
        ReadPtr<AGraphNode> rData = data.cget();
        if (rData)
            isVisible = rData->isVisible();
    }

    Qt::CheckState state = isVisible ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
    bool stateInit = true;
    bool is_expanded = isExpanded(node->index());

    if (is_expanded)
    {
        for (int i = node->rowCount(); i > 0; i--)
        {
            QStandardItem* childNode = node->child(i - 1);
            if (!childNode || !childNode->isCheckable())
                continue;

            if (stateInit)
            {
                state = childNode->checkState();
                stateInit = false;
            }
            else
            {
                if (childNode->checkState() != state)
                    state = Qt::CheckState::PartiallyChecked;
            }
        }
    }
    else
    {
        std::unordered_set<SafePtr<AGraphNode>> nodeChildren = node->getChildren(m_graphManager);
        for (const SafePtr<AGraphNode>& child : nodeChildren)
        {
            Qt::CheckState childState = recStateOnData(child);
            if (stateInit)
            {
                state = childState;
                stateInit = false;
            }
            else
            {
                if (childState != state)
                    state = Qt::CheckState::PartiallyChecked;
            }
        }
    }

    return state;
}
