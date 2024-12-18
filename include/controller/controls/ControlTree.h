#ifndef CONTROL_TREE_H
#define CONTROL_TREE_H

#include "controller/controls/IControl.h"
#include "utils/safe_ptr.h"
#include "gui/widgets/TreeNodeSystem/TreeNode.h"
#include "models/Types.hpp"

#include <list>

class AGraphNode;
class ClusterNode;

namespace control::tree
{
	/*! Création d'un cluster */
	class CreateCluster : public AControl
	{
	public:
		/*!
				\param TreeType type - Type de TreeSytem où on créé le cluster

				\param treeId parentId - id du TreeElement parent du nouveau cluster  */
		CreateCluster(const TreeType& type, const SafePtr<AGraphNode>& parent);
		~CreateCluster();
		void doFunction(Controller& controller) override;
		ControlType getType() const override;
	private:
		TreeType m_type;
		SafePtr<AGraphNode> m_parent;
		SafePtr<ClusterNode> m_cluster;
	};

	/*! Déplacement des élèments de l'arbre */
	class DropElements : public AControl
	{
	public:
		/*! 
				\param TreeType type Type(id) de l'arbre dans l'arborescence ou on veut drop 

				\param const SafePtr<AGraphNode>& parent Le noeud sur lequel on drop
				
				\param std::list<treeId> dragItems Les ids treeId des noeuds séléctionnées (draggués) présent dans le même sous-arbre de destination qu'on veut drop
		
		*/
		DropElements(const SafePtr<AGraphNode>& destParent, TreeType destTreeType, const std::unordered_set<SafePtr<AGraphNode>>& nodesToDrop);
		~DropElements();
		void doFunction(Controller& controller) override;
		bool canUndo() const override;
		void undoFunction(Controller& controller) override;
		ControlType getType() const override;
	private:
		SafePtr<AGraphNode> m_parent;
		TreeType m_type;
		std::unordered_map<SafePtr<AGraphNode>, std::unordered_set<SafePtr<AGraphNode>>> m_toMove;
		std::unordered_set<SafePtr<AGraphNode>> m_toCreate;
	};

	class RemoveElemFromHierarchy : public AControl
	{
	public:
		RemoveElemFromHierarchy(std::list<TreeNode*> treeNodeToRemove);
		~RemoveElemFromHierarchy();
		void doFunction(Controller& controller) override;
		bool canUndo() const override;
		void undoFunction(Controller& controller) override;
		ControlType getType() const override;
	private:
		std::unordered_map<SafePtr<AGraphNode>, std::unordered_set<SafePtr<AGraphNode>>> m_oldParent_toRemoveNode;
	};
}

#endif