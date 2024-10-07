#include "gui/widgets/TreeNodeSystem/TreeNodeFactory.h"
#include "controller/controls/ControlDataEdition.h"
#include "controller/controls/ControlClippingEdition.h"
#include "controller/controls/ControlTree.h"

#include "gui/widgets/ProjectTreePanel.h"
#include "models/graph/GraphManager.hxx"
#include "models/graph/TagNode.h"

#include "models/3d/NodeFunctions.h"

#include "gui/style/IconObject.h"

#include "utils/ProjectStringSets.hpp"
#include "gui/texts/TreePanelTexts.hpp"
#include "utils/ProjectColor.hpp"

#include "gui/Texts.hpp"
#include "gui/DataDispatcher.h"

#define TREELOG Logger::log(LoggerMode::TreeLog)

TreeNodeFactory::TreeNodeFactory(ProjectTreePanel* treePanel, GraphManager& graphManager)
	: m_treePanel(treePanel)
	, m_graphManager(graphManager)
{}

TreeNodeFactory::~TreeNodeFactory()
{ }

TreeNode* TreeNodeFactory::constructTreeNode(const SafePtr<AGraphNode>& data, TreeNode *parent, TreeType treetype)
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

	m_treePanel->blockAllSignals(true);

	TreeNode* newNode = new TreeNode(data, getChildFonction, canDrop, onDrop, treetype);

	newNode->setCheckable(true);
	newNode->setStructNode(false);

	if (parent != nullptr)
		parent->appendRow(newNode);

	std::unordered_map<SafePtr<AGraphNode>, std::vector<TreeNode*>>& treenodes = m_treePanel->m_model->getTreeNodes();

	m_treePanel->blockAllSignals(false);

	updateNode(newNode);

	if (treenodes.find(data) == treenodes.end())
		treenodes.insert({ data, std::vector<TreeNode*>() });
	treenodes[data].push_back(newNode);


	return newNode;
}

/*
TreeNode* TreeNodeFactory::constructDisciplineNodes(const std::unordered_set<ElementType>& types, TreeNode* parentNode)
{
	std::vector<std::wstring> disciplines;
	disciplines = ProjectStringSets::getStringSet(L"DISCIPLINE");

	TreeNode* DisciplineNodes = constructStructNode(TEXT_DISCIPLINE_TREE_NODE, [](const SafePtr<AGraphNode>& data) { return false; },
		[](IDataDispatcher&, std::unordered_set<SafePtr<AGraphNode>> ) { return; }, [](const SafePtr<AGraphNode>& data) { return false; }, parentNode);

	for (auto it = disciplines.begin(); it != disciplines.end(); it++)
	{
		std::wstring str = std::wstring(*it);
		std::function<std::unordered_set<SafePtr<AGraphNode>>(const GraphManager&, const SafePtr<AGraphNode>&)> c_disciplineClip = [types, str](const SafePtr<AGraphNode>& data)
		{
			ReadPtr<AGraphNode> rData = data.cget();
			if (!rData)
				return false;
			if (types.find(rData->getType()) != types.end()) return (false);
			if (rData->getDiscipline() == (str)) return (true);
			return (false);
		};
		std::function<void(IDataDispatcher& dispatcher, std::unordered_set<SafePtr<AGraphNode>> )> s_disciplineClip = 
			[str](IDataDispatcher& dispatcher, std::unordered_set<SafePtr<AGraphNode>> datas)
		{
			std::unordered_set<SafePtr<AObjectNode>> objects;
			for (const SafePtr<AGraphNode>& data : datas)
				objects.insert(static_pointer_cast<AObjectNode>(data));
			dispatcher.sendControl(new control::dataEdition::SetDiscipline(objects, str));
		};
		std::function<bool(const SafePtr<AGraphNode>& data)> df_disciplineClip = [types](const SafePtr<AGraphNode>& data)
		{
			ReadPtr<AGraphNode> rData = data.cget();
			if (!rData)
				return false;
			if (types.find(rData->getType()) == types.end()) return (true);
			return (false);
		};
		TreeNode* discipline = constructStructNode(QString::fromStdWString(*it), f_disciplineClip, s_disciplineClip, df_disciplineClip, DisciplineNodes);
		discipline->setCheckable(true); 
		discipline->setCheckState(Qt::CheckState::Checked);
	}

	return (DisciplineNodes);
}

TreeNode* TreeNodeFactory::constructPhaseNodes(const std::unordered_set<ElementType>& types, TreeNode* parentNode)
{
	std::vector<std::wstring> phases;
	phases = ProjectStringSets::getStringSet(L"PHASE");

	TreeNode* PhaseNodes = constructStructNode(TEXT_PHASE_TREE_NODE, [](SafePtr<AGraphNode> data) { return false; },
		[](IDataDispatcher&, std::unordered_set<SafePtr<AGraphNode>> ) { return; }, [](SafePtr<AGraphNode> data) { return false; }, parentNode);

	for (auto it = phases.begin(); it != phases.end(); it++)
	{
		std::wstring str = std::wstring(*it);
		std::function<bool(const SafePtr<AGraphNode>& data)> f_phaseTag = [types, str](const SafePtr<AGraphNode>& data)
		{
			ReadPtr<AGraphNode> rData = data.cget();
			if (!rData)
				return false;
			if (types.find(rData->getType()) != types.end()) return (false);
			if (rData->getPhase() == (str)) return (true);
			return (false);
		};
		std::function<void(IDataDispatcher& dispatcher, std::unordered_set<SafePtr<AGraphNode>> )> s_PhaseTag = 
			[str](IDataDispatcher& dispatcher, std::unordered_set<SafePtr<AGraphNode>> datas)
		{
			std::unordered_set<SafePtr<AObjectNode>> objects;
			for (const SafePtr<AGraphNode>& data : datas)
				objects.insert(static_pointer_cast<AObjectNode>(data));
			dispatcher.sendControl(new control::dataEdition::SetPhase(objects, str));
		};
		std::function<bool(const SafePtr<AGraphNode>& data)> df_phaseTag = [types](const SafePtr<AGraphNode>& data)
		{
			ReadPtr<AGraphNode> rData = data.cget();
			if (!rData)
				return false;
			if (types.find(rData->getType()) == types.end()) return (true);
			return (false);
		};
		TreeNode* phase = constructStructNode(QString::fromStdWString(*it), f_phaseTag, s_PhaseTag, df_phaseTag, PhaseNodes);
		phase->setCheckable(true);
		phase->setCheckState(Qt::CheckState::Checked);
	}

	return (PhaseNodes);
}
*/

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
Qt::CheckState TreeNodeFactory::recGetNodeState(TreeNode* node)
{
	bool isVisible = true;
	SafePtr<AGraphNode> data = node->getData();
	if(data)
	{
		ReadPtr<AGraphNode> rData = data.cget();
		if (rData)
			isVisible = rData->isVisible();
	}

	Qt::CheckState state = isVisible ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
	bool stateInit = true;
	bool isExpanded = m_treePanel->isExpanded(node->index());
	
	if (isExpanded)
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

void TreeNodeFactory::updateNode(TreeNode* node)
{
	QStandardItem* parent = node->parent();
	if (parent && !m_treePanel->isExpanded(parent->index()))
		return;

	SafePtr<AGraphNode> data = node->getData();
	ElementType type;
	std::wstring name;
	Color32 color;
	bool isVisible;
	bool isSelected;

	if (data)
	{
		{
			ReadPtr<AObjectNode> rObj = static_pointer_cast<AObjectNode>(data).cget();
			if (!rObj)
				return;

			isVisible = rObj->isVisible();
			type = rObj->getType();
			color = rObj->getColor();
			name = rObj->getComposedName();
			isSelected = rObj->isSelected();
		}

		scs::MarkerIcon tagIcon = scs::MarkerIcon::Max_Enum;
		if (type == ElementType::Tag)
		{
			ReadPtr<TagNode> rTag = static_pointer_cast<TagNode>(data).cget();
			if (rTag)
				tagIcon = rTag->getMarkerIcon();
		}

		if (nodeFunctions::isMissingFile(data))
			name = L"?" + name;

		m_treePanel->blockAllSignals(true);

		node->setText(QString::fromStdWString(name));
		node->setIcon(scs::IconManager::getInstance().getIcon(type, tagIcon, QColor(color.r, color.g, color.b)));
		if (node->isCheckable())
			node->setCheckState(isVisible ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

		QItemSelectionModel* selectModel = m_treePanel->selectionModel();
		if (selectModel)
			selectModel->setCurrentIndex(node->index(), isSelected ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);

		if (isSelected)
			m_treePanel->m_selectedNodes.insert(data);
		else
			m_treePanel->m_selectedNodes.erase(data);

		m_treePanel->blockAllSignals(false);
	}

	bool isExpanded = m_treePanel->isExpanded(node->index());

	std::unordered_map<SafePtr<AGraphNode>, TreeNode*> oldChildren;

	std::unordered_set<TreeNode*> nodesToUpdate;
	std::unordered_set<TreeNode*> nodesToDelete;

	bool toSort = true;

	if (isExpanded)
	{
		for (int i = node->rowCount(); i > 0; i--)
		{
			TreeNode* child = static_cast<TreeNode*>(node->child(i - 1));
			SafePtr<AGraphNode> dataChild = child->getData();
			if(dataChild)
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
	if (isExpanded)
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
			if(!child->isStructNode())
				nodesToDelete.insert(child);
		}
		if (!newChildren.empty())
			node->appendRow(constructPlaceholderNode(node->getTreeType()));
	}

	for (TreeNode* upNode : nodesToUpdate)
		updateNode(upNode);

	for (TreeNode* delNode : nodesToDelete)
		m_treePanel->removeTreeNode(delNode);

	m_treePanel->blockAllSignals(true);

	if (toSort && isExpanded)
		node->sortChildren(0);

	if(node->isCheckable())
		node->setCheckState(recGetNodeState(node));

	m_treePanel->blockAllSignals(false);
}

std::vector<TreeNode*> TreeNodeFactory::constructSubList(const std::unordered_map<ElementType, QString>& values, TreeNode* parentNode)
{
	std::vector<TreeNode*> Nodes;
	for (const std::pair<ElementType, QString>& iterator : values)
	{
		ElementType type(iterator.first);
		std::function<std::unordered_set<SafePtr<AGraphNode>>(const GraphManager&)> getChildFonction = 
		[type](const GraphManager& manager)
		{
			return manager.getNodesByTypes({ type });
		};

		TreeNode* element = constructStructNode(iterator.second, 
													getChildFonction,
													[](const SafePtr<AGraphNode>& data) { return false; },
													[](IDataDispatcher&, std::unordered_set<SafePtr<AGraphNode>> data) { return; }, 
													parentNode);
		element->setCheckable(true);
		element->setCheckState(Qt::CheckState::Checked);
		Nodes.push_back(element);
	}

	return Nodes;
}

TreeNode* TreeNodeFactory::constructColorNodes(const std::unordered_set<ElementType>& types, TreeNode* parentNode)
{
	std::vector<std::pair<QString, Color32>> colors = { {TEXT_GREEN, ProjectColor::getColor("GREEN")},
														{TEXT_RED, ProjectColor::getColor("RED")},
														{TEXT_ORANGE, ProjectColor::getColor("ORANGE")},
														{TEXT_YELLOW, ProjectColor::getColor("YELLOW")},
														{TEXT_BLUE, ProjectColor::getColor("BLUE")},
														{TEXT_PURPLE, ProjectColor::getColor("PURPLE")},
														{TEXT_LIGHT_GREY, ProjectColor::getColor("LIGHT GREY")},
														{TEXT_BROWN, ProjectColor::getColor("BROWN")}
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

		std::function<bool(const SafePtr<AGraphNode>& data)> df_colorClip = [color, types](const SafePtr<AGraphNode>& data)
		{
			ReadPtr<AGraphNode> rData = data.cget();
			if (!rData)
				return false;
			if (types.find(rData->getType()) != types.end()) 
				return (true);
			return (false);
		};

		std::function<void(IDataDispatcher& dispatcher, std::unordered_set<SafePtr<AGraphNode>>)> s_colorClip =
			[color](IDataDispatcher& dispatcher, std::unordered_set<SafePtr<AGraphNode>> datas)
		{
			std::unordered_set<SafePtr<AObjectNode>> objects;
			for (const SafePtr<AGraphNode>& data : datas)
				objects.insert(static_pointer_cast<AObjectNode>(data));
			dispatcher.sendControl(new control::dataEdition::SetColor(objects, color));
		};

		TreeNode* colorStruct = constructStructNode(colorName, getChildFonction, df_colorClip, s_colorClip, ColorNodes);
		colorStruct->setCheckable(true);
		colorStruct->setCheckState(Qt::CheckState::Checked);

	}

	return (ColorNodes);
}

TreeNode* TreeNodeFactory::constructStatusNodes(const std::unordered_set<ElementType>& types, TreeNode* parentNode)
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

		std::function<bool(const SafePtr<AGraphNode>& data)> df_statusActClip = [types](const SafePtr<AGraphNode>& data)
		{
			ReadPtr<AGraphNode> rData = data.cget();
			if (!rData)
				return false;
			if (types.find(rData->getType()) != types.end()) return (true);
			return (false);
		};

		std::function<void(IDataDispatcher& dispatcher, std::unordered_set<SafePtr<AGraphNode>>)> s_statusClip =
			[state](IDataDispatcher& dispatcher, std::unordered_set<SafePtr<AGraphNode>> datas)
		{
			std::unordered_set<SafePtr<AClippingNode>> clips;
			for (const SafePtr<AGraphNode>& data : datas)
				clips.insert(static_pointer_cast<AClippingNode>(data));
			dispatcher.sendControl(new control::clippingEdition::SetClipActive(clips, state));
		};

		TreeNode* statusNode = constructStructNode(state ? TEXT_ACTIVE : TEXT_INACTIVE, getChildFonction, df_statusActClip, s_statusClip, parentStatusNode);
		statusNode->setCheckable(true);
		statusNode->setCheckState(Qt::CheckState::Checked);
	}

	return (parentStatusNode);
}

TreeNode* TreeNodeFactory::constructClipMethodNodes(const std::unordered_set<ElementType>& types, TreeNode* parentNode)
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

		std::function<bool(const SafePtr<AGraphNode>& data)> df_CM = [types](const SafePtr<AGraphNode>& data)
		{
			ReadPtr<AGraphNode> rData = data.cget();
			if (!rData)
				return false;
			if (types.find(rData->getType()) != types.end())
				return (true);
			return (false);
		};

		std::function<void(IDataDispatcher& dispatcher, std::unordered_set<SafePtr<AGraphNode>>)> s_CM =
			[clipMode](IDataDispatcher& dispatcher, std::unordered_set<SafePtr<AGraphNode>> datas)
		{
			std::unordered_set<SafePtr<AClippingNode>> clips;
			for (const SafePtr<AGraphNode>& data : datas)
				clips.insert(static_pointer_cast<AClippingNode>(data));
			dispatcher.sendControl(new control::clippingEdition::SetMode(clips, clipMode));
		};

		TreeNode* CMNode = constructStructNode(name, getChildFonction, df_CM, s_CM, clipMethNode);
		CMNode->setCheckable(true);
		CMNode->setCheckState(Qt::CheckState::Checked);
	}

	return (clipMethNode);
}

TreeNode* TreeNodeFactory::constructMasterNode(const QString& name, TreeType treeType)
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

TreeNode* TreeNodeFactory::constructPlaceholderNode(TreeType treeType)
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

TreeNode* TreeNodeFactory::constructStructNode(const QString& name,
	std::function<std::unordered_set<SafePtr<AGraphNode>>(const GraphManager&)> getChildFonction, 
	std::function<bool(const SafePtr<AGraphNode>&)> dropFilter,
	std::function<void(IDataDispatcher&, std::unordered_set<SafePtr<AGraphNode>> )> controlSender,
	TreeNode *parent)
{
	TreeType treetype = TreeType::RawData;
	if (parent != nullptr)
		treetype = parent->getTreeType();

	TreeNode* newNode = new TreeNode(SafePtr<AGraphNode>(), getChildFonction, dropFilter, controlSender, treetype);
	newNode->setStructNode(true);

	updateNode(newNode);

	newNode->setText(name);
	newNode->setCheckable(false);
	if (parent != nullptr)
		parent->appendRow(newNode);

	std::unordered_map<TreeType, std::vector<TreeNode*>>& rootNodes = m_treePanel->m_rootNodes;

	if (rootNodes.find(treetype) == rootNodes.end())
		rootNodes[treetype] = std::vector<TreeNode*>();
	rootNodes[treetype].push_back(newNode);

	return (newNode);
}
