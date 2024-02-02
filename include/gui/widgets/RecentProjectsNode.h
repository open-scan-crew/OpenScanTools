#include <QtGui/QStandardItemModel>
#include <QtCore/QDateTime>

#ifndef RECENT_PROJECT_NODE_H_
#define RECENT_PROJECT_NODE_H_

class RecentProjectsNode : public QStandardItem
{
public:
	explicit RecentProjectsNode(QString& data, QString author, QDate date, QStandardItem *item = nullptr);
	~RecentProjectsNode();

	void setDate(QDate date);
	QDate getDate() const;

private:
	QDate m_date;
};

#endif