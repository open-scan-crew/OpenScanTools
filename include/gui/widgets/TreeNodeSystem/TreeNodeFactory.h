#ifndef TREE_NODE_FACTORY_H
#define TREE_NODE_FACTORY_H

#include "gui/widgets/TreeNodeSystem/TreeNode.h"

class QString;
class ProjectTreePanel;
class OpenScanToolsGraphManager;

class TreeNodeFactory
{
public:
	TreeNodeFactory(ProjectTreePanel* treePanel, OpenScanToolsGraphManager& graphManager);
	~TreeNodeFactory();

	TreeNode* constructTreeNode(const SafePtr<AGraphNode>& data, TreeNode *parent, TreeType treetype);

	TreeNode* constructMasterNode(const QString& name, TreeType treeType);
	TreeNode* constructPlaceholderNode(TreeType treeType);
	TreeNode* constructStructNode(const QString& name, 
		std::function<std::unordered_set<SafePtr<AGraphNode>>(const OpenScanToolsGraphManager&)> getChildFonction,
		std::function<bool(const SafePtr<AGraphNode>&)> dropFilter,
		std::function<void(IDataDispatcher&, std::unordered_set<SafePtr<AGraphNode>>)> controlSender,
		TreeNode *parent = nullptr);

	std::vector<TreeNode*> constructSubList(const std::unordered_map<ElementType, QString>& values, TreeNode* parentNode);

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
	OpenScanToolsGraphManager& m_graphManager;
	ProjectTreePanel* m_treePanel;
};

#endif // !TREENODEFACTORY_H_
