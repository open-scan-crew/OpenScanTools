#ifndef SAVE_LOAD_SYSTEM_HXX
#define SAVE_LOAD_SYSTEM_HXX
#include "io/SaveLoadSystem.h"
#include "nlohmannJson/json.hpp"

#include "io/imports/DataDeserializer.hxx"
#include "io/exports/DataSerializer.h"
#include "io/SerializerKeys.h"


#include "utils/Logger.h"
#include "utils/JsonWriter.h"

template<typename ListType>
std::filesystem::path SaveLoadSystem::ExportCSVList(ListType list, std::filesystem::path path)
{
	std::filesystem::path l_path = path;

	std::wstring name = list.getName() + L".csv";
	std::filesystem::path importPath = l_path / name;

	std::wofstream outStream = std::wofstream(importPath);
	IOLOG << "export csv list at " << importPath << LOGENDL;
	if (outStream.is_open() == false)
		return ("");
	
	bool first(true);
	for (auto iterator : list.list())
	{
		if (first)
			first = false;
		else
			outStream << ";";
		outStream << iterator;
	}
	outStream.close();
	return (importPath);
	IOLOG << "export successfull" << LOGENDL;
}

template<typename ListType>
std::filesystem::path SaveLoadSystem::ExportCSVList(SafePtr<ListType> list, std::filesystem::path path)
{
	ReadPtr<ListType> rList = list.cget();
	if (rList)
		return ExportCSVList<ListType>(*&rList, path);
	else
		return std::filesystem::path();
}

template<typename ListType>
ListType SaveLoadSystem::ImportNewList(const std::filesystem::path& filePath)
{
	std::wifstream listFile(filePath);
	ListType fList;

	if (listFile.is_open() == true)
	{
		fList = ListType(filePath.stem().wstring());
		std::wstring line;
		bool isValid = true;
		while (std::getline(listFile, line))
		{
			IOLOG << "read " << line << LOGENDL;
			std::list<std::wstring> list = splitOnChar(line, L";,:\t");
			for (auto it : list)
				if (!fList.insertStrValue(it))
				{
					isValid = false;
					break;
				}

		}
		fList.setValid(isValid);
		listFile.close();
	}
	return (fList);
}

template<typename ListType>
std::filesystem::path SaveLoadSystem::ExportLists(const std::vector<ListType>& lists, const std::filesystem::path& filePath)
{
	if (lists.empty())
		return filePath;
	std::filesystem::path path = filePath;

	nlohmann::json jsonLists;
	nlohmann::json listsArray = nlohmann::json::array();
	IOLOG << "Export lists at " << path << LOGENDL;
	for (const ListType& list : lists)
	{
		listsArray.push_back(DataSerializer::SerializeList<ListType>(list));
		IOLOG << "export list " << list.getName() << LOGENDL;
	}
	jsonLists[Key_Lists] = listsArray;

	if (!utils::writeJsonFile(path, jsonLists))
	{
		IOLOG << "Error : lists file could not be exported" << LOGENDL;
		assert(false);
	}

	return (path);
}

template<typename ListType>
std::filesystem::path SaveLoadSystem::ExportLists(const std::unordered_set<SafePtr<ListType>>& lists, const std::filesystem::path& filePath)
{
	std::vector<ListType> exportLists;
	for (SafePtr<ListType> list : lists)
	{
		ReadPtr<ListType> rList = list.cget();
		if (rList)
			exportLists.push_back(*&rList);
	}
	return ExportLists<ListType>(exportLists, filePath);
}

template<typename ListType>
std::vector<ListType> SaveLoadSystem::ImportLists(const std::filesystem::path& filePath)
{
	std::filesystem::path path = filePath;
	std::vector<ListType> lists;
	std::filesystem::path importPath = filePath;
	IOLOG << "Loading lists [" << importPath << "]" << LOGENDL;
	std::ifstream fileStream(importPath);
	nlohmann::json jsonLists;
	if (fileStream.good())
	{
		try
		{
			fileStream >> jsonLists;
		}
		catch (std::exception& exception)
		{
			IOLOG << "Error : import failed " << exception.what() << LOGENDL;
			return (lists);
		}
	}
	else
	{
		IOLOG << "Error : Cannot find " << importPath << LOGENDL;
		return (lists);
	}
	fileStream.close();

	if (jsonLists.find(Key_Lists) != jsonLists.end() && jsonLists.at(Key_Lists).is_array())
	{
		for (const nlohmann::json& it : jsonLists.at(Key_Lists))
		{
			ListType userList;
			if (DataDeserializer::DeserializeList<ListType>(it, userList))
			{
				lists.push_back(userList);
				IOLOG << "import list : " << userList.getName() << " with " << userList.list().size() << " element(s)" << LOGENDL;
			}
		}
	}
	IOLOG << "import " << lists.size() << " lists" << LOGENDL;
	return (lists);
}

#endif