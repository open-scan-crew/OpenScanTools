#ifndef PROJECT_STRING_SETS_HPP
#define PROJECT_STRING_SETS_HPP

#include <vector>
#include <string>

class ProjectStringSets
{
private:
	static std::unordered_map<std::wstring, std::vector<std::wstring>>& _strset()
	{
		static std::unordered_map<std::wstring, std::vector<std::wstring>> *strSet = new std::unordered_map<std::wstring, std::vector<std::wstring>>();
		return (*strSet);
	}
public:

	static bool registerStringSet(std::wstring name, std::vector<std::wstring> newColor)
	{
		if (_strset().find(name) == _strset().end())
		{
			_strset().insert({ name, newColor });
			return (true);
		}
		return (false);
	};

	static bool changeStringSet(std::wstring name, std::vector<std::wstring> newColor)
	{
		if (_strset().find(name) != _strset().end())
		{
			_strset().find(name)->second = newColor;
			return (true);
		}
		return (false);
	};

	static bool removeStringSet(std::wstring name)
	{
		if (_strset().find(name) != _strset().end())
		{
			_strset().erase(name);
			return (true);
		}
		return (false);
	};

	static std::vector<std::wstring> getStringSet(std::wstring name)
	{
		if (_strset().find(name) != _strset().end())
			return (_strset().find(name)->second);
		else
			return (std::vector<std::wstring>());
	}

	/*static std::string getColorNameFromColor(std::vector<std::string> color)
	{
		std::unordered_map<std::string, std::vector<std::string>>::iterator it;

		for (it = _strset().begin(); it != _strset().end(); it++)
		{
			if (color == it->second)
				return (it->first);
		}
		return ("");
	}*/
};

#endif
