#include <unordered_map>
#include <string>
#include <iostream>
#include "models/application/Author.h"
#include <assert.h>
#include "utils/Utils.h"

static std::vector<std::wstring> s_authorsDefaultName = { L"Alice", L"Bob", L"Camille", L"Damien", L"Elise", L"Fabien", L"Gabriel", L"Hector" };
static std::vector<std::string> s_authorsDefaultId = { "d930dda1-20f4-4c14-9cdf-4089268d677a", "07485898-ee30-4720-8351-c2cf875a5e9b", 
"1c09ad98-1430-4330-b93d-e12455bc61a1", "1b02fba1-25fc-470d-9857-e0658b84d842", 
"24bce657-a520-4f60-b13d-32b64c502332", "e5abc4c0-1153-4445-8728-14289d1801a0", 
"632d97df-3379-4dda-9ace-c1355c54cf51", "a97438cd-c868-44c8-98e9-7a0ebd3bf8aa" };

Author::Author(xg::Guid id)
{
	m_id = id;
	m_name = L"";
}

Author::Author(const std::wstring& name)
{
	m_id = xg::newGuid();
	m_name = name;
}

Author::Author()
{
	assert(s_authorsDefaultName.size() == s_authorsDefaultId.size());
	int random = rand() % s_authorsDefaultName.size();
	m_name = s_authorsDefaultName[random];
	m_id = xg::Guid(s_authorsDefaultId[random]);
}

Author::Author(const Author& author)
{
	m_id = author.m_id;
	m_name = author.m_name;
}

Author::~Author()
{ }

Author Author::createNullAuthor()
{
	Author nullAuth = Author(xg::Guid());
	nullAuth.m_name = L"NO_AUTHOR";

	return nullAuth;
}

void Author::setName(const std::wstring& name)
{
	m_name = name;
}

void Author::setId(const xg::Guid id)
{
	m_id = id;
}

const std::wstring& Author::getName() const
{
	return m_name;
}

xg::Guid Author::getId() const
{
	return m_id;
}

bool Author::operator<(const Author& rhs) const
{
	return this->getId() < rhs.getId();
}

bool Author::operator==(const Author& rhs) const
{
	return this->getId() == rhs.getId();
}

size_t Author::operator()(const Author& auth) const
{
	return std::hash<xg::Guid>()(auth.getId());
}

std::ostream& Author::operator<<(std::ostream& stream) const
{
	return stream << Utils::to_utf8(this->getName());
}

