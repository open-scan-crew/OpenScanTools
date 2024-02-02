#ifndef USER_LIST_H_
#define USER_LIST_H_

#include <set>
#include <string>

#include "crossguid/guid.hpp"

typedef xg::Guid listId;
/*! Liste pour construire des élèments utilisées dans OpenScanTools.

*	Eléments tels que :
		- Standard de pipes (List<double>)
		- Utilisateur (List<std::string>)

	Créé pour avoir des attributs supplémentaires tels que Origin, Name et Valid
		
*/

template<class Value>
class List
{
public:
	List();
	List(const std::wstring& name);
	List(const listId& id, const std::wstring& name);
	List(const List<Value>& list);
	~List();

	void mergeList(List<Value> l);

	void setName(const std::wstring& newName);
	void setId(const listId& id);
	void setOrigin(const bool& value = false);
	void setValid(bool valid);
	bool isValid() const;

	listId getId() const;
	const std::wstring& getName() const;
	bool getOrigin() const;
	std::set<Value>& list();
	const std::set<Value>& clist() const;
	bool insertStrValue(const std::wstring& strValue);
	void insertValue(const Value& value);

	std::string toJson(const Value& value);

	bool operator==(const List<Value>& rhs) const;
	bool operator<(const List<Value>& rhs) const;
	size_t operator()(const List<Value>& auth) const;

private:
	bool m_valid;
	/*! Si m_origin est vrai : liste ayant un caractère de "par défaut", sinon faux
		
		Exemple :
			pour les listes de standards de tuyaux (StandardList)
			on a : N.A.,   
		*/
	bool m_origin;
	listId m_id;
	std::wstring m_name;
	std::set<Value> m_elems;
};

#include "List.hxx"

namespace std
{
	template<class Value>
	struct hash<List<Value>>
	{
		std::size_t operator()(List<Value> const& list) const
		{
			return std::hash<xg::Guid>()(list.getId());
		}
	};
}

typedef List<std::wstring> UserList;
enum class StandardType { Pipe, Torus, Sphere };
typedef List<double> StandardList;

std::vector<UserList> generateDefaultLists();
std::vector<StandardList> generateDefaultPipeStandardList();

#endif // !USER_LIST_H_