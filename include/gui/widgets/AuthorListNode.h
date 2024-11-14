#ifndef AUTHORLISTNODE_H_
#define AUTHORLISTNODE_H_

#include <QtGui/qstandarditemmodel.h>
#include "models/OpenScanToolsModelEssentials.h"

class Author;

class AuthorListNode : public QStandardItem
{
public:
	explicit AuthorListNode(const QString& data, const SafePtr<Author>& author, QStandardItem *item = nullptr);
	~AuthorListNode();

	void setAuthor(const SafePtr<Author>& author);
	SafePtr<Author> getAuthor() const;

private:
	SafePtr<Author> m_author;
};

#endif