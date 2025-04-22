#ifndef TREE_NODE_FACTORY_H
#define TREE_NODE_FACTORY_H

#include "gui/widgets/TreeNodeSystem/TreeNode.h"

class QString;
class ProjectTreePanel;
class GraphManager;

class TreeNodeFactory
{
public:
	TreeNodeFactory(ProjectTreePanel* treePanel, GraphManager& graphManager);
	~TreeNodeFactory();

	TreeNode* constructTreeNode(const SafePtr<AGraphNode>& data, TreeNode *parent, TreeType treetype);

	TreeNode* constructMasterNode(const QString& name, TreeType treeType);
	TreeNode* constructPlaceholderNode(TreeType treeType);

	typedef std::function<std::unordered_set<SafePtr<AGraphNode>>(const GraphManager&)> fctGetChildren;
	typedef std::function<bool(const SafePtr<AGraphNode>&)> fctCanDrop;
	typedef std::function<void(IDataDispatcher&, std::unordered_set<SafePtr<AGraphNode>>)> fctOnDrop;

	TreeNode* constructStructNode(const QString& name, 
		fctGetChildren getChildFonction,
		fctCanDrop can_drop,
		fctOnDrop on_drop,
		TreeNode *parent = nullptr);

	TreeNode* addSubList(TreeNode* parentNode, QString name, ElementType type);
	// Special for Box/Grid filter
	TreeNode* addBoxesSubList(TreeNode* parentNode, QString name, bool only_box);

	TreeNode* constructColorNodes(const std::unordered_set<ElementType>& types, TreeNode* parentNode);
	TreeNode* constructStatusNodes(const std::unordered_set<ElementType>& types, TreeNode* parentNode);
	TreeNode* constructClipMethodNodes(const std::unordered_set<ElementType>& types, TreeNode* parentNode);

	/*
	std::vector<StructTreeNode*> constructMeasureListNodes(TreeNode *parentNode);
	std::vector<StructTreeNode*> constructObjectTypeNodes(TreeNode *parentNode);

	TreeNode* constructDisciplineNodes(const std::unordered_set<ElementType>& types, TreeNode* parentNode);
	TreeNode* constructPhaseNodes(const std::unordered_set<ElementType>& types, TreeNode* parentNode);
	*/

	void updateNode(TreeNode* node);

private:
	Qt::CheckState recGetNodeState(TreeNode* node);

private:
	GraphManager& m_graphManager;
	ProjectTreePanel* m_treePanel;
};

#endif // !TREENODEFACTORY_H_
