#ifndef UTILS_SYSTEM_H_
#define UTILS_SYSTEM_H_

#include <filesystem>

namespace Utils
{
    namespace System
    {
        bool openFileStream(std::string subDir, std::string fileName, bool appendDate, std::string extension, std::ofstream& out);

		uint64_t getPhysicalMemoryAvailable();
		uint64_t getVirtualMemoryAvailable();
		uint64_t getPagingMemoryAvailable();
		uint64_t getExtendedMemoryAvailable();
		uint64_t getTotalMemoryAvailable();
		uint64_t getPhysicalMemoryUsed();
		uint64_t getVirtualMemoryUsed();
		uint64_t getPagingMemoryUsed();
		uint64_t getPerCentMemoryUsed();
		uint64_t getTotalMemoryUsed();
		uint64_t getTotalPysicalMemory();
		uint64_t getTotalVirtualMemory();
		uint64_t getTotalPagedMemory();
		uint64_t getTotalMemory();

		std::filesystem::path getDocumentPath();
        std::filesystem::path getOpenScanToolsDocumentPath();
		std::filesystem::path getProgramDataPath();
		std::filesystem::path getOSTProgramDataPath();
		std::filesystem::path getAppDataPath(); 
		std::filesystem::path getOSTAppDataPath();
		std::filesystem::path getOSTProgramDataTemplatePath();
		std::filesystem::path getAndCreateDirectory(std::filesystem::path origin, std::string name);

		void formatFilename(std::wstring& filename);

		bool createDirectoryIfNotExist(const std::filesystem::path& directory);
		bool cleanDirectory(const std::filesystem::path& directory); 
		bool cleanDirectoryFromFiles(const std::filesystem::path& directory, const std::filesystem::path& extension, const bool& deepClean);
		std::vector<std::filesystem::path> getFilesFromDirectory(const std::filesystem::path& directory, const std::filesystem::path& extension, const bool& recursive);
		std::vector<std::filesystem::path> getFolderFromDirectory(const std::filesystem::path& directory);
    }
}

#endif