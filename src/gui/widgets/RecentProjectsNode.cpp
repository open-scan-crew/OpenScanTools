#include "gui/widgets/RecentProjectsNode.h"

RecentProjectsNode::RecentProjectsNode(QString & data, QString author, QDate date, QStandardItem * item)
	: QStandardItem(data)
{
	m_author = author;
	m_date = date;
}

RecentProjectsNode::~RecentProjectsNode()
{
}

void RecentProjectsNode::setDate(QDate date) 
{
	m_date = date;
}

QDate RecentProjectsNode::getDate() const
{
	return (m_date);
}