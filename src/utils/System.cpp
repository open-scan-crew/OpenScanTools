#include "utils/System.h"
#include "utils/FilesAndFoldersDefinitions.h"
#include <ctime>
#include <filesystem>
#include <iostream>

#include <ShlObj.h>
#include <KnownFolders.h>
#include <wchar.h>

#include "windows.h"

#include "utils/Logger.h"
#define IOLOG Logger::log(LoggerMode::IOLog)

uint64_t getPhysicalMemoryAvailable(const MEMORYSTATUSEX& memoryInfo)
{
	return memoryInfo.ullAvailPhys;
}

uint64_t getVirtualMemoryAvailable(const MEMORYSTATUSEX& memoryInfo)
{
	return memoryInfo.ullAvailVirtual;
}

uint64_t getPagingMemoryAvailable(const MEMORYSTATUSEX& memoryInfo)
{
	return memoryInfo.ullAvailPageFile;
}

uint64_t getExtendedMemoryAvailable(const MEMORYSTATUSEX& memoryInfo)
{
	return memoryInfo.ullAvailExtendedVirtual;
}

uint64_t getTotalMemoryAvailable(const MEMORYSTATUSEX& memoryInfo)
{
	return memoryInfo.ullAvailPhys + memoryInfo.ullAvailVirtual + memoryInfo.ullAvailExtendedVirtual + memoryInfo.ullAvailPageFile;
}

uint64_t getPhysicalMemoryUsed(const MEMORYSTATUSEX& memoryInfo)
{
	return memoryInfo.ullTotalPhys - memoryInfo.ullAvailPhys;
}

uint64_t getVirtualMemoryUsed(const MEMORYSTATUSEX& memoryInfo)
{
	return memoryInfo.ullTotalVirtual - memoryInfo.ullAvailVirtual;
}

uint64_t getPagingMemoryUsed(const MEMORYSTATUSEX& memoryInfo)
{
	return memoryInfo.ullTotalPageFile - memoryInfo.ullAvailPageFile;
}

uint64_t getPerCentMemoryUsed(const MEMORYSTATUSEX& memoryInfo)
{
	return memoryInfo.dwMemoryLoad;
}

uint64_t getTotalMemoryUsed(const MEMORYSTATUSEX& memoryInfo)
{
	return (memoryInfo.ullTotalPhys - memoryInfo.ullAvailPhys) + (memoryInfo.ullTotalVirtual - memoryInfo.ullAvailVirtual) + (memoryInfo.ullTotalPageFile - memoryInfo.ullAvailPageFile);
}

uint64_t getTotalPysicalMemory(const MEMORYSTATUSEX& memoryInfo)
{
	return memoryInfo.ullTotalPhys;
}

uint64_t getTotalVirtualMemory(const MEMORYSTATUSEX& memoryInfo)
{
	return memoryInfo.ullTotalVirtual;
}

uint64_t getTotalPagedMemory(const MEMORYSTATUSEX& memoryInfo)
{
	return memoryInfo.ullTotalPageFile;
}

uint64_t getTotalMemory(const MEMORYSTATUSEX& memoryInfo)
{
	return memoryInfo.ullTotalPhys + memoryInfo.ullTotalVirtual + memoryInfo.ullTotalPageFile;
}

bool getMemoryStatus(MEMORYSTATUSEX& memoryInfo)
{
	memoryInfo.dwLength = sizeof(memoryInfo);
	return GlobalMemoryStatusEx(&memoryInfo);
}

bool Utils::System::openFileStream(std::string subDir, std::string fileName, bool appendDate, std::string extension, std::ofstream& out)
{
    PWSTR pathDoc = nullptr;

    HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &pathDoc);
    if (SUCCEEDED(hr) == false)
    {
        std::cerr << "Critical error : Cannot find Document directory" << std::endl;
        return false;
    }

    std::filesystem::path documents(pathDoc);
    std::filesystem::path ostDoc = documents / Folder_OpenScanTools;
    std::filesystem::path finalDir = ostDoc / subDir;

    if (std::filesystem::exists(ostDoc) == false)
    {
        std::error_code ec;
        if (std::filesystem::create_directory(ostDoc, ec) == false)
        {
            std::cout << "Error creating directory: " << ec.message();
            return false;
        }
    }

    if (std::filesystem::exists(finalDir) == false)
    {
        std::error_code ec;
        if (std::filesystem::create_directory(finalDir, ec) == false)
        {
            std::cout << "Error creating directory: " << ec.message();
            return false;
        }
    }

    std::string completeName = fileName + "_";

    if (appendDate)
    {
        std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::string str = std::ctime(&time);
        for (int i = 0; i < str.size(); i++)
        {
            if (str[i] == ' ' || str[i] == '\r' || str[i] == '\n' || str[i] == ':')
                str[i] = '_';
        }
        completeName += str;
    }

    completeName += "." + extension;

    std::filesystem::path filePath = finalDir / completeName;

    out.open(filePath);
    if (out.fail())
    {
        std::cerr << "Error: cannot create the file stream " << filePath << std::endl;
        return false;
    }
    return true;
}

uint64_t Utils::System::getPhysicalMemoryAvailable()
{
	MEMORYSTATUSEX memoryInfo;
	if (!getMemoryStatus(memoryInfo))
		return 0;
#ifdef  _DEBUG
	std::cout << " Utils::System::getPhysicalMemoryAvailable: " << memoryInfo.ullAvailPhys << "\n";
#endif //  DEBUG
	return memoryInfo.ullAvailPhys;
}

uint64_t Utils::System::getVirtualMemoryAvailable() 
{
	MEMORYSTATUSEX memoryInfo;
	if (!getMemoryStatus(memoryInfo))
		return 0;
#ifdef  _DEBUG
	std::cout << " Utils::System::getVirtualMemoryAvailable: " << memoryInfo.ullAvailVirtual << "\n";
#endif //  DEBUG
	return memoryInfo.ullAvailVirtual;
}

uint64_t Utils::System::getPagingMemoryAvailable()
{
	MEMORYSTATUSEX memoryInfo;
	if (!getMemoryStatus(memoryInfo))
		return 0;
#ifdef  _DEBUG
	std::cout << " Utils::System::getPagingMemoryAvailable: " << memoryInfo.ullAvailPageFile << "\n";
#endif //  DEBUG
	return memoryInfo.ullAvailPageFile;
}

uint64_t Utils::System::getExtendedMemoryAvailable()
{
	MEMORYSTATUSEX memoryInfo;
	if (!getMemoryStatus(memoryInfo))
		return 0;
#ifdef  _DEBUG
	std::cout << " Utils::System::getExtendedMemoryAvailable: " << memoryInfo.ullAvailExtendedVirtual << "\n";
#endif //  DEBUG
	return memoryInfo.ullAvailExtendedVirtual;
}

uint64_t Utils::System::getTotalMemoryAvailable()
{
	MEMORYSTATUSEX memoryInfo;
	if (!getMemoryStatus(memoryInfo))
		return 0;
#ifdef  _DEBUG
	std::cout << " Utils::System::getTotalMemoryAvailable: " << memoryInfo.ullAvailPhys + memoryInfo.ullAvailVirtual + memoryInfo.ullAvailExtendedVirtual + memoryInfo.ullAvailPageFile << "\n";
#endif //  DEBUG
	return memoryInfo.ullAvailPhys + memoryInfo.ullAvailVirtual + memoryInfo.ullAvailExtendedVirtual + memoryInfo.ullAvailPageFile;
}

uint64_t Utils::System::getPhysicalMemoryUsed()
{
	MEMORYSTATUSEX memoryInfo;
	if (!getMemoryStatus(memoryInfo))
		return 0;
#ifdef  _DEBUG
	std::cout << " Utils::System::getPhysicalMemoryUsed: " << memoryInfo.ullTotalPhys - memoryInfo.ullAvailPhys << "\n";
#endif //  DEBUG
	return memoryInfo.ullTotalPhys - memoryInfo.ullAvailPhys;
}

uint64_t Utils::System::getVirtualMemoryUsed()
{
	MEMORYSTATUSEX memoryInfo;
	if (!getMemoryStatus(memoryInfo))
		return 0;
#ifdef  _DEBUG
	std::cout << " Utils::System::getVirtualMemoryUsed: " << memoryInfo.ullTotalVirtual - memoryInfo.ullAvailVirtual << "\n";
#endif //  DEBUG
	return memoryInfo.ullTotalVirtual - memoryInfo.ullAvailVirtual;
}

uint64_t Utils::System::getPagingMemoryUsed()
{
	MEMORYSTATUSEX memoryInfo;
	if (!getMemoryStatus(memoryInfo))
		return 0;
#ifdef  _DEBUG
	std::cout << " Utils::System::getPagingMemoryUsed: " << memoryInfo.ullTotalPageFile - memoryInfo.ullAvailPageFile << "\n";
#endif //  DEBUG
	return memoryInfo.ullTotalPageFile - memoryInfo.ullAvailPageFile;
}

uint64_t  Utils::System::getPerCentMemoryUsed()
{
	MEMORYSTATUSEX memoryInfo;
	if (!getMemoryStatus(memoryInfo))
		return 0;
#ifdef  _DEBUG
	std::cout << " Utils::System::getPerCentMemoryUsed: " << memoryInfo.dwMemoryLoad << "\n";
#endif //  DEBUG
	return memoryInfo.dwMemoryLoad;
}

uint64_t Utils::System::getTotalMemoryUsed()
{
	MEMORYSTATUSEX memoryInfo;
	if (!getMemoryStatus(memoryInfo))
		return 0;
#ifdef  _DEBUG
	std::cout << " Utils::System::getTotalMemoryUsed: " << (memoryInfo.ullTotalPhys - memoryInfo.ullAvailPhys) + (memoryInfo.ullTotalVirtual - memoryInfo.ullAvailVirtual) + (memoryInfo.ullTotalPageFile - memoryInfo.ullAvailPageFile) << "\n";
#endif //  DEBUG
	return (memoryInfo.ullTotalPhys - memoryInfo.ullAvailPhys) + (memoryInfo.ullTotalVirtual - memoryInfo.ullAvailVirtual) + (memoryInfo.ullTotalPageFile - memoryInfo.ullAvailPageFile);
}

uint64_t Utils::System::getTotalPysicalMemory()
{
	MEMORYSTATUSEX memoryInfo;
	if (!getMemoryStatus(memoryInfo))
		return 0;
	return memoryInfo.ullTotalPhys;
}

uint64_t Utils::System::getTotalVirtualMemory()
{
	MEMORYSTATUSEX memoryInfo;
	if (!getMemoryStatus(memoryInfo))
		return 0;
	return memoryInfo.ullTotalVirtual;
}

uint64_t Utils::System::getTotalPagedMemory()
{
	MEMORYSTATUSEX memoryInfo;
	if (!getMemoryStatus(memoryInfo))
		return 0;
	return memoryInfo.ullTotalPageFile;
}

uint64_t Utils::System::getTotalMemory()
{
	MEMORYSTATUSEX memoryInfo;
	if (!getMemoryStatus(memoryInfo))
		return 0;
	return memoryInfo.ullTotalPhys + memoryInfo.ullTotalVirtual + memoryInfo.ullTotalPageFile;
}

std::filesystem::path Utils::System::getDocumentPath()
{
	PWSTR pathDoc = nullptr;
	std::filesystem::path path = "";

	HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &pathDoc);

	if (SUCCEEDED(hr) == true)
	{
		char buff[1024];

		buff[sprintf(buff, "%ls", pathDoc)] = 0;

		path = buff;
	}
	return (path);
}

std::filesystem::path Utils::System::getOpenScanToolsDocumentPath()
{
    PWSTR pathDoc = nullptr;
    struct stat info;

    HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &pathDoc);

    if (SUCCEEDED(hr) == true)
    {
        char buff[1024];
        std::string dirPath = "";

        buff[sprintf(buff, "%ls", pathDoc)] = 0;
        dirPath += buff;
        dirPath += "\\" + (std::string)Folder_OpenScanTools;

        if (stat(dirPath.c_str(), &info) != 0)
            CreateDirectory(dirPath.c_str(), nullptr);

        dirPath += "\\";

        return (dirPath);
    }
    return ("");
}

std::filesystem::path Utils::System::getAppDataPath()
{
	std::filesystem::path path;
	PWSTR path_tmp;

	/* Attempt to get user's AppData folder
	 *
	 * Microsoft Docs:
	 * https://docs.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shgetknownfolderpath
	 * https://docs.microsoft.com/en-us/windows/win32/shell/knownfolderid
	 */
	auto get_folder_path_ret = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &path_tmp);

	/* Error check */
	if (get_folder_path_ret != S_OK) {
		CoTaskMemFree(path_tmp);
		return "";
	}

	/* Convert the Windows path type to a C++ path */
	path = path_tmp;

	/* Free memory :) */
	CoTaskMemFree(path_tmp);
	return path;
}

std::filesystem::path Utils::System::getOSTAppDataPath()
{
	std::filesystem::path path(getAppDataPath());
	if (path.empty())
		return path;
	path = path / Folder_OpenScanTools;
	std::filesystem::create_directory(path);
	return path;
}

std::filesystem::path Utils::System::getProgramDataPath()
{
	std::filesystem::path path;
	PWSTR path_tmp;

	/* Attempt to get user's AppData folder
	 *
	 * Microsoft Docs:
	 * https://docs.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shgetknownfolderpath
	 * https://docs.microsoft.com/en-us/windows/win32/shell/knownfolderid
	 */
	auto get_folder_path_ret = SHGetKnownFolderPath(FOLDERID_ProgramData, 0, nullptr, &path_tmp);

	/* Error check */
	if (get_folder_path_ret != S_OK) {
		CoTaskMemFree(path_tmp);
		return "";
	}

	/* Convert the Windows path type to a C++ path */
	path = path_tmp;

	/* Free memory :) */
	CoTaskMemFree(path_tmp);
	return path;
}

std::filesystem::path Utils::System::getOSTProgramDataPath()
{
	std::filesystem::path path(getProgramDataPath());
	if (path.empty())
		return path;
	path = path / Folder_OpenScanTools;
	std::filesystem::create_directory(path);
	return path;
}

std::filesystem::path Utils::System::getOSTProgramDataTemplatePath()
{
	std::filesystem::path path(getOSTProgramDataPath());
	if (path.empty())
		return path;
	path = path / Folder_Template;
	std::filesystem::create_directory(path);
	return path;
}

std::filesystem::path Utils::System::getAndCreateDirectory(std::filesystem::path origin, std::string name)
{
	std::filesystem::path path = origin;

	if (std::filesystem::exists(path) == false)
		std::filesystem::create_directory(path);

	path = path / name;

	if (std::filesystem::exists(path) == false)
		std::filesystem::create_directory(path);
	
	return (path);
}

void Utils::System::formatFilename(std::wstring& filename)
{
	for (wchar_t& c : filename)
	{
		if (c == L'?' || c == L'<' || c == L'>' || c == L'|' || c == L':' || c == L'*' || c == L'/' || c == L'\\' || c == L'"')
			c = L'_';
	}

	return;
}

bool Utils::System::createDirectoryIfNotExist(const std::filesystem::path& directory)
{
	if (std::filesystem::exists(directory) == false)
	{
		IOLOG << "Create folder " << directory << LOGENDL;
		try
		{
			if (std::filesystem::create_directories(directory) == false)
			{
				IOLOG << "fail to create project folder" << LOGENDL;
				return false;
			}

			return true;
		}
		catch (std::exception & e) {
			IOLOG << "Exception occured when creating a folder: " << e.what() << LOGENDL;
		}

		return false;
	}
	return true;
}

bool Utils::System::cleanDirectory(const std::filesystem::path& directory)
{
	if (!std::filesystem::exists(directory))
		return (false);
	if(std::filesystem::remove_all(directory))
	{
		IOLOG << "Folder removed " << directory << LOGENDL;
		/*try
		{
			if (std::filesystem::create_directories(directory) == false)
			{
				IOLOG << "fail to create folder: " << directory << LOGENDL;
				return false;
			}
		}
		catch (std::exception& e) {
			IOLOG << "Exception occured when creating a folder: " << e.what() << LOGENDL;
		}
		return createDirectoryIfNotExist(directory);*/
	}
	return true;
}

bool Utils::System::cleanDirectoryFromFiles(const std::filesystem::path& directory, const std::filesystem::path& extension, const bool& deepClean)
{
	if (!std::filesystem::exists(directory))
		return (false);

	for (const std::filesystem::path& p : std::filesystem::directory_iterator(directory))
	{
		try {
			if (std::filesystem::is_directory(p))
			{
				if (deepClean)
					cleanDirectoryFromFiles(p, extension, deepClean);
				else
					continue;
			}
		}
		catch(...){}

		try {
			if (p.extension() == extension)
			{
				IOLOG << "Removing " << p << LOGENDL;
				std::filesystem::remove(p);
			}
		}
		catch (...) {}
	}
	return true;
}


std::vector<std::filesystem::path> Utils::System::getFilesFromDirectory(const std::filesystem::path& directory, const std::filesystem::path& extension, const bool& recursive)
{
	if (!std::filesystem::exists(directory))
		return (std::vector<std::filesystem::path>());

	std::vector<std::filesystem::path> retValue;
	for (const std::filesystem::path& p : std::filesystem::directory_iterator(directory))
	{
		try
		{
			if (!std::filesystem::exists(p))
				continue;
			if (std::filesystem::is_directory(p))
			{
				if (recursive)
				{
					std::vector<std::filesystem::path> values = getFilesFromDirectory(p, extension, recursive);
					retValue.insert(retValue.end(), values.begin(), values.end());
				}
				else
					continue;
			}
			if (p.extension() == extension)
			{
				retValue.push_back(p);
			}
		}
		catch (std::exception e)
		{
			continue;
		}
		
	}
	return retValue;
}

std::vector<std::filesystem::path> Utils::System::getFolderFromDirectory(const std::filesystem::path& directory)
{
	if (!std::filesystem::exists(directory))
		return (std::vector<std::filesystem::path>());

	std::vector<std::filesystem::path> retValue;
	for (const std::filesystem::path& p : std::filesystem::directory_iterator(directory))
	{
		if (std::filesystem::is_directory(p) && !p.filename().empty())
			retValue.push_back(p);
	}
	return retValue;
}