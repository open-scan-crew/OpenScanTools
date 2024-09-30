#include "controller/controls/ControlTree.h"
#include "controller/controls/ControlFunction.h"
#include "controller/Controller.h"
#include "controller/ControlListener.h"
#include "controller/functionSystem/FunctionManager.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataTree.h"
#include "gui/GuiData/GuiData3dObjects.h"
#include "magic_enum/magic_enum.hpp"
#include "utils/Logger.h"
#include "gui/Texts.hpp"

#include "models/3d/Graph/ClusterNode.h"
#include "models/3d/Graph/GraphManager.h"

namespace control::tree
{
	//
	// CreateCluster
	//

	CreateCluster::CreateCluster(const TreeType& type, const SafePtr<AGraphNode>& parent)
        : m_parent(parent)
        , m_type(type)
	{}

	CreateCluster::~CreateCluster()
	{
	}

	void CreateCluster::doFunction(Controller& controller)
	{
		m_cluster = make_safe<ClusterNode>();
		{
			WritePtr<ClusterNode> wCluster = m_cluster.get();
			wCluster->setTreeType(m_type);
			wCluster->setDefaultData(controller);
		}

		if (m_parent)
			AGraphNode::addOwningLink(m_parent, m_cluster);

		controller.getControlListener()->notifyUIControl(new control::function::AddNodes({ m_cluster }));
	}

	ControlType CreateCluster::getType() const
	{
		return (ControlType::createClusterTree);
	}

	//
	// DropElement
	//

	DropElements::DropElements(const SafePtr<AGraphNode>& destParent, TreeType destTreeType, const std::unordered_set<SafePtr<AGraphNode>>& nodesToDrop)
	{
		m_parent = destParent;
		m_type = destTreeType;


		for (const SafePtr<AGraphNode>& dragNode : nodesToDrop)
		{
			SafePtr<AGraphNode> oldParent = AGraphNode::getOwningParent(dragNode, destTreeType);
			if (oldParent)
			{
				if (m_toMove.find(oldParent) == m_toMove.end())
					m_toMove[oldParent] = std::unordered_set<SafePtr<AGraphNode>>();
				m_toMove[oldParent].insert(dragNode);
			}
			else
				m_toCreate.insert(dragNode);
		}
    }

	DropElements::~DropElements()
	{}

	void DropElements::doFunction(Controller& controller)
	{
		CONTROLLOG << "control::tree::DropElements::doFunction" << LOGENDL;

		std::unordered_set< SafePtr<AGraphNode>> toActualizeItems;

		for (const SafePtr<AGraphNode>& node : m_toCreate)
		{
			AGraphNode::addOwningLink(m_parent, node);
			toActualizeItems.insert(node);
		}

		for (const std::pair<SafePtr<AGraphNode>, std::unordered_set<SafePtr<AGraphNode>>>& oldParent_Nodes : m_toMove)
		{
			for (const SafePtr<AGraphNode>& node : oldParent_Nodes.second)
			{
				AGraphNode::removeOwningLink(oldParent_Nodes.first, node);
				AGraphNode::addOwningLink(m_parent, node);
				toActualizeItems.insert(node);
			}
		}

		controller.actualizeTreeView(toActualizeItems);
	}

	bool DropElements::canUndo() const
	{
		return (!m_toCreate.empty() || !m_toMove.empty());
	}

	void DropElements::undoFunction(Controller& controller)
	{
		CONTROLLOG << "control::tree::DropElements::undoFunction" << LOGENDL;

		std::unordered_set< SafePtr<AGraphNode>> toActualizeItems;

		for (const SafePtr<AGraphNode>& node : m_toCreate)
		{
			AGraphNode::removeOwningLink(m_parent, node);
			toActualizeItems.insert(node);
		}

		for (const std::pair<SafePtr<AGraphNode>, std::unordered_set<SafePtr<AGraphNode>>>& oldParent_Nodes : m_toMove)
		{
			for (const SafePtr<AGraphNode>& node : oldParent_Nodes.second)
			{
				AGraphNode::removeOwningLink(m_parent, node);
				AGraphNode::addOwningLink(oldParent_Nodes.first, node);
				toActualizeItems.insert(node);
			}
		}

		controller.actualizeTreeView(toActualizeItems);
	}

	ControlType DropElements::getType() const
	{
		return (ControlType::DropElement);
	}

	//
	// RemoveElemFromHierarchy
	//

	RemoveElemFromHierarchy::RemoveElemFromHierarchy(std::list<TreeNode*> treeNodeToRemove)
	{
		for (TreeNode* node : treeNodeToRemove)
		{
			if (!node->getData())
				continue;
			
			if (!node->parent())
				continue;

			TreeNode* parentNode = static_cast<TreeNode*>(node->parent());
			SafePtr<AGraphNode> parent = parentNode->getData();
			if (!parent)
				continue;

			if (m_oldParent_toRemoveNode.find(parent) == m_oldParent_toRemoveNode.end())
				m_oldParent_toRemoveNode[parent] = std::unordered_set<SafePtr<AGraphNode>>();
			m_oldParent_toRemoveNode[parent].insert(node->getData());

		}
	}

	RemoveElemFromHierarchy::~RemoveElemFromHierarchy()
	{}

	void RemoveElemFromHierarchy::doFunction(Controller& controller)
	{
		std::unordered_map<SafePtr<AGraphNode>, std::unordered_set<SafePtr<AGraphNode>> > copy = m_oldParent_toRemoveNode;
		m_oldParent_toRemoveNode.clear();
		for (std::pair<SafePtr<AGraphNode>, std::unordered_set<SafePtr<AGraphNode>>> pair : copy)
		{
			SafePtr<AGraphNode> parent = pair.first;
			for (SafePtr<AGraphNode> child : pair.second)
			{
				if (!AGraphNode::removeOwningLink(parent, child))
					continue;

				if (m_oldParent_toRemoveNode.find(parent) == m_oldParent_toRemoveNode.end())
					m_oldParent_toRemoveNode[parent] = std::unordered_set<SafePtr<AGraphNode>>();
				m_oldParent_toRemoveNode[parent].insert(child);

				{
					WritePtr<AGraphNode> wObj = child.get();
					if (wObj && wObj->getType() == ElementType::Cluster)
						wObj->setDead(true);
				}
			}

			controller.actualizeTreeView(parent);
		}
	}

	bool RemoveElemFromHierarchy::canUndo() const
	{
		return !m_oldParent_toRemoveNode.empty();
	}

	void RemoveElemFromHierarchy::undoFunction(Controller& controller)
	{
		std::unordered_map<SafePtr<AGraphNode>, std::unordered_set<SafePtr<AGraphNode>> > copy = m_oldParent_toRemoveNode;
		m_oldParent_toRemoveNode.clear();
		for (std::pair<SafePtr<AGraphNode>, std::unordered_set<SafePtr<AGraphNode>>> pair : copy)
		{
			SafePtr<AGraphNode> parent = pair.first;
			for (SafePtr<AGraphNode> child : pair.second)
			{
				if (!AGraphNode::addOwningLink(parent, child))
					continue;

				if (m_oldParent_toRemoveNode.find(parent) == m_oldParent_toRemoveNode.end())
					m_oldParent_toRemoveNode[parent] = std::unordered_set<SafePtr<AGraphNode>>();
				m_oldParent_toRemoveNode[parent].insert(child);

				{
					WritePtr<AGraphNode> wObj = child.get();
					if (wObj && wObj->getType() == ElementType::Cluster)
						wObj->setDead(false);
				}
			}

			controller.actualizeTreeView(parent);
		}
	}

	ControlType RemoveElemFromHierarchy::getType() const
	{
		return (ControlType::removeFromHierarchy);
	}
}