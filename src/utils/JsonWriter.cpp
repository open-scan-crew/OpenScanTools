#include "utils/JsonWriter.h"
#include "utils/FilesAndFoldersDefinitions.h"

#include <fstream>

bool utils::writeJsonFile(const std::filesystem::path& exportPath, const nlohmann::json& json)
{
	std::filesystem::path backPath = exportPath.wstring() + File_Extension_Backup_Wide;
	try
	{
		if (std::filesystem::exists(exportPath))
		{
			std::filesystem::rename(exportPath, backPath);
		}
	}
	catch (std::exception e)
	{
		assert(false);
		return false;
	}

	std::ofstream outFile(exportPath);
	if (!outFile.is_open() || outFile.bad())
	{
		assert(false);
		return false;
	}

	try
	{
		outFile << std::setw(4) << json << std::endl;
	}
	catch (std::exception e)
	{
		outFile.close();
		assert(false);
		return false;
	}
	outFile.close();

	try
	{
		std::filesystem::remove(backPath);
	}
	catch (std::exception e)
	{
		assert(false);
		return false;
	}

	return true;
}