#ifndef TREE_NODE_H
#define TREE_NODE_H
#include "models/Types.hpp"
#include "utils/safe_ptr.h"

#include <QtCore/qlist.h>
#include <QtCore/qvariant.h>
#include <QtGui/qstandarditemmodel.h>

class AGraphNode;
class IDataDispatcher;
class GraphManager;

/*! TreeNode fais le lien entre un element d'arbre Qt et un TreeElement */
class TreeNode : public QStandardItem
{
public:
	TreeNode(const SafePtr<AGraphNode>& data,
				std::function<std::unordered_set<SafePtr<AGraphNode>>(const GraphManager&)> getChildFonction,
				std::function<bool(SafePtr<AGraphNode>)> canBeDropFilter,
				std::function<void(IDataDispatcher&, std::unordered_set<SafePtr<AGraphNode>>)> onDropFunction,
				TreeType treeType
			);
	~TreeNode();

	ElementType getType() const;
	TreeType getTreeType() const;
	SafePtr<AGraphNode> getData() const;

	bool isStructNode() const;
	void setStructNode(bool structNode);

	void setType(ElementType type);

	std::unordered_set<SafePtr<AGraphNode>> getChildren(const GraphManager& manager);
	bool canDataDrop(const SafePtr<AGraphNode>& data);
	void dropControl(IDataDispatcher& dataDispatcher, const std::unordered_set<SafePtr<AGraphNode>>& datas);

	QVariant getDataVariant(int column) const;


protected:
	QList<QVariant>	m_nodeData;

	SafePtr<AGraphNode> m_data;
	TreeType	m_treeType;
	ElementType m_elemType;

	std::function<std::unordered_set<SafePtr<AGraphNode>>(const GraphManager&)> m_getChildFonction;
	std::function<bool(const SafePtr<AGraphNode>&)> m_canBeDropFilter;
	std::function<void(IDataDispatcher&, std::unordered_set<SafePtr<AGraphNode>>)> m_onDropFunction;

	bool m_isStructNode = false;
};

#endif // _TREENODE_H_