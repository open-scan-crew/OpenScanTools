#ifndef LISTSNODE_H_
#define LISTSNODE_H_

#include <QtGui/qstandarditemmodel.h>
#include "utils/safe_ptr.h"


template<class ListType>
class ListNode : public QStandardItem
{
public:
	explicit ListNode(const QString& data, SafePtr<ListType> list, bool originList, QStandardItem* item = nullptr) 
		: QStandardItem(data)
		, m_originList(originList)
		, m_list(list)
	{}

	~ListNode()
	{}

	SafePtr<ListType> getList() const
	{
		return (m_list);
	}

	bool isOriginList() const
	{
		return (m_originList);
	}


private:
	bool m_originList;
	SafePtr<ListType> m_list;
};

class ItemNode : public QStandardItem
{
public:
	explicit ItemNode(const QString& data, std::wstring wStrData, QStandardItem* item = nullptr);
	~ItemNode();

	void setWStrData(std::wstring str);
	std::wstring getWStrData() const;

private:
	std::wstring m_wstr;
};

#endif