#include "gui/widgets/AuthorListNode.h"

AuthorListNode::AuthorListNode(const QString& data, const SafePtr<Author>& author, QStandardItem * item)
	: QStandardItem(data)
{
	m_author = author;
}

AuthorListNode::~AuthorListNode()
{
}

void AuthorListNode::setAuthor(const SafePtr<Author>& author)
{
	m_author = author;
}

SafePtr<Author> AuthorListNode::getAuthor() const
{
	return (m_author);
}