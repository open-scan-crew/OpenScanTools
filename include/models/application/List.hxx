#include "List.h"

template<class Value>
List<Value>::List()
	: m_elems({})
	, m_origin(false)
	, m_valid(false)
{}

template<class Value>
List<Value>::List(const std::wstring& name)
	: m_elems({})
	, m_origin(false)
	, m_valid(false)
	, m_id(xg::newGuid())
	, m_name(name)
{}



template<class Value>
List<Value>::List(const listId& id, const std::wstring& name)
	: m_elems({})
	, m_origin(false)
	, m_valid(false)
	, m_id(id)
	, m_name(name)
{}

template<class Value>
List<Value>::List(const List<Value>& list)
{
	m_elems = list.m_elems;
	m_origin = list.m_origin;
	m_valid = list.m_valid;
	m_id = list.m_id;
	m_name = list.m_name;
}

template<class Value>
List<Value>::~List()
{
}

template<class Value>
inline void List<Value>::mergeList(List<Value> l)
{
	for (Value v : l.m_elems)
		insertValue(v);
}

template<class Value>
void List<Value>::setId(const listId& id)
{
	m_id = id;
}

template<class Value>
void List<Value>::setName(const std::wstring& newName)
{
	m_name = newName;
}

template<class Value>
void List<Value>::setOrigin(const bool& value)
{
	m_origin = value;
}

template<class Value>
inline void List<Value>::setValid(bool valid)
{
	m_valid = valid;
}

template<class Value>
bool List<Value>::isValid() const
{
	return (m_valid);
}

template<class Value>
listId List<Value>::getId() const
{
	return (m_id);
}

template<class Value>
bool List<Value>::getOrigin() const
{
	return (m_origin);
}

template<class Value>
const std::wstring& List<Value>::getName() const
{
	return (m_name);
}

template<class Value>
std::set<Value>& List<Value>::list()
{
	return (m_elems);
}

template<class Value>
const std::set<Value>& List<Value>::clist() const
{
	return m_elems;
}

template<class Value>
void List<Value>::insertValue(const Value& value)
{
	m_elems.insert(value);
}

template<class Value>
bool List<Value>::operator==(const List<Value>& rhs) const
{
	return this->getId() == rhs.getId();
}

template<class Value>
bool List<Value>::operator<(const List<Value>& rhs) const
{
	return this->getId() < rhs.getId();;
}

template<class Value>
size_t List<Value>::operator()(const List<Value>& list) const
{
	return std::hash<xg::Guid>()(list.getId());
}
