#include "gui/widgets/TreeNodeSystem/TreeNode.h"
#include "gui/widgets/TreeNodeSystem/TreeModel.h"
#include "utils/Logger.h"

#include "services/MarkerDefinitions.hpp"
#include <QtCore/QStringList>

#include "models/graph/AGraphNode.h"

#define GTLOG Logger::log(LoggerMode::GTLog)

TreeNode::TreeNode(const SafePtr<AGraphNode>& data,
					std::function<std::unordered_set<SafePtr<AGraphNode>>(const GraphManager&)> getChildFonction,
					std::function<bool(SafePtr<AGraphNode>)> canBeDropFilter,
					std::function<void(IDataDispatcher&, std::unordered_set<SafePtr<AGraphNode>>)> onDropFunction,
					TreeType treeType
				)
	: QStandardItem()
	, m_data(data)
	, m_elemType(ElementType::None)
	, m_getChildFonction(getChildFonction)
	, m_onDropFunction(onDropFunction)
	, m_canBeDropFilter(canBeDropFilter)
{
	setEditable(false);
	m_treeType = treeType;

	{
		ReadPtr<AGraphNode> rData = data.cget();
		if (rData)
			m_elemType = rData->getType();
	}
}

TreeNode::~TreeNode()
{}

QVariant TreeNode::getDataVariant(int column) const
{
	return (m_nodeData.value(column));
}

ElementType TreeNode::getType() const
{
	return m_elemType;
}

TreeType TreeNode::getTreeType() const
{
	return (m_treeType);
}

SafePtr<AGraphNode> TreeNode::getData() const
{
	return m_data;
}

bool TreeNode::isStructNode() const
{
	return m_isStructNode;
}

void TreeNode::setStructNode(bool isStructNode)
{
	m_isStructNode = isStructNode;
}

void TreeNode::setType(ElementType type)
{
	m_elemType = type;
}

std::unordered_set<SafePtr<AGraphNode>> TreeNode::getChildren(const GraphManager& manager)
{
	return (m_getChildFonction(manager));
}

bool TreeNode::canDataDrop(const SafePtr<AGraphNode>& data)
{
	return (m_canBeDropFilter(data));
}

void TreeNode::dropControl(IDataDispatcher& dataDispatcher, const std::unordered_set<SafePtr<AGraphNode>>& datas)
{
	m_onDropFunction(dataDispatcher, datas);
}