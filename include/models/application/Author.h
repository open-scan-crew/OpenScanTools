#ifndef AUTHOR_H_
#define AUTHOR_H_

#include <string>
#include <iostream>
#include "crossguid/guid.hpp"

class Author
{
public:
	explicit Author(xg::Guid id);
	explicit Author(const std::wstring& name);

	Author(const Author& author);
	Author();
	~Author();

	static Author createNullAuthor();

	void setName(const std::wstring& name);
	const std::wstring& getName() const;

	void setId(const xg::Guid id);
	xg::Guid getId() const;

	bool operator<(const Author& rhs) const;
	bool operator==(const Author& rhs) const;
	size_t operator()(const Author& auth) const;
	std::ostream& operator<<(std::ostream& stream) const;

private:
	std::wstring m_name;
	xg::Guid m_id;
};

namespace std
{
	template<>
	struct hash<Author>
	{
		std::size_t operator()(Author const& auth) const
		{
			return std::hash<xg::Guid>()(auth.getId());
		}
	};
}

#endif