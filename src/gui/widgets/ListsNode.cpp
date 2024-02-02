#include "gui/widgets/ListsNode.h"

// ItemNode

ItemNode::ItemNode(const QString& data, std::wstring wStrData, QStandardItem* item)
	: QStandardItem(data)
{
	m_wstr = wStrData;
}

ItemNode::~ItemNode()
{
}

void ItemNode::setWStrData(std::wstring wstr)
{
	m_wstr = wstr;
}

std::wstring ItemNode::getWStrData() const
{
	return m_wstr;
}
