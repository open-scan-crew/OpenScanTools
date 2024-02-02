#include <unordered_map>
#include <string>
#include "utils/Color32.hpp"

#ifndef _PROJECT_COLOR_HPP_
#define _PROJECT_COLOR_HPP_

class ProjectColor
{
private:
	static std::unordered_map<std::string, Color32>& _colors()
	{
		static std::unordered_map<std::string, Color32> *colors = new std::unordered_map<std::string, Color32>();
		return (*colors);
	}
public:

	static bool registerColor(std::string name, Color32 newColor)
	{
		if (_colors().find(name) == _colors().end())
		{
			_colors().insert({ name, newColor });
			return (true);
		}
		return (false);
	};

	static bool changeColor(std::string name, Color32 newColor)
	{
		if (_colors().find(name) != _colors().end())
		{
			_colors().find(name)->second = newColor;
			return (true);
		}
		return (false);
	};

	static bool removeColor(std::string name)
	{
		if (_colors().find(name) != _colors().end())
		{
			_colors().erase(name);
			return (true);
		}
		return (false);
	};

	static Color32 getColor(std::string name)
	{
		if (_colors().find(name) != _colors().end())
			return (_colors().find(name)->second);
		else
			return (Color32::Zero());
	}

	static std::string getColorNameFromColor(Color32 color)
	{
		std::unordered_map<std::string, Color32>::iterator it;

		for (it = _colors().begin(); it != _colors().end(); it++)
		{
			if (color == it->second)
				return (it->first);
		}
		return ("");
	}
};

#endif // !_PROJECT_COLOR_HPP_
