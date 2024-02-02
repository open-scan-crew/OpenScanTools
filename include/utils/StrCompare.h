#ifndef _STRCOMPARE_H_
#define _STRCOMPARE_H_

#include <string>

namespace Utils
{
	inline size_t strCompare(const std::string& oldString, const std::string& newString, const bool& offset = true)
	{
		int index = 0;
		while (index < oldString.size() && index < newString.size())
		{
			if (oldString.at(index) != newString.at(index))
				return (oldString.size() < newString.size()) && offset ? index + 1 : index;
			index++;
		}
		return (offset ? (int)newString.size() : (int)oldString.size());
	}
};

#endif