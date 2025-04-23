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
	typedef std::function<std::unordered_set<SafePtr<AGraphNode>>(const GraphManager&)> fctGetChildren;
	typedef std::function<bool(const SafePtr<AGraphNode>&)> fctCanDrop;
	typedef std::function<void(IDataDispatcher&, std::unordered_set<SafePtr<AGraphNode>>)> fctOnDrop;

	TreeNode(const SafePtr<AGraphNode>& data,
		fctGetChildren getChildFonction,
		fctCanDrop canBeDropFilter,
		fctOnDrop onDropFunction,
		TreeType treeType);
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
	QList<QVariant> m_nodeData;

	SafePtr<AGraphNode> m_data;
	TreeType m_treeType;
	ElementType m_elemType;

	fctGetChildren m_getChildFonction;
	fctCanDrop m_canBeDropFilter;
	fctOnDrop m_onDropFunction;

	bool m_isStructNode = false;
};

#endif // _TREENODE_H_