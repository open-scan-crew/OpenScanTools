#ifndef PROJECT_TYPES_H
#define PROJECT_TYPES_H

#include "models/application/TagTemplate.h"
#include "models/application/List.h"

enum class IndexationMethod
{
	FillMissingIndex,
	HighestIndex
};

struct ProjectTemplate
{
	std::vector<sma::TagTemplate> m_template;
	std::vector<UserList> m_lists;
};

#endif // !_PROJECT_TYPES_H_
