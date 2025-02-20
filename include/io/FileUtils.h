#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <unordered_map>
#include <filesystem>
#include <cctype>

enum FileType {E57, TLS, FARO_LS, FARO_PROJ, RCS, RCP, OBJ, STEP, FBX, IFC, DXF, PTS, MAX_ENUM };
enum class ObjectExportType { OBJ, FBX, DXF, CSV, STEP, OST};

// TODO - move in cpp
const static std::unordered_map<FileType, std::string> FileTypeDictionnary = {
    {FileType::E57, ".e57"},
    {FileType::TLS, ".tls"},
    {FileType::FARO_LS, ".fls"},
    {FileType::FARO_PROJ, ".lsproj"},
    {FileType::RCS, ".rcs"},
    {FileType::RCP, ".rcp"},
    {FileType::FBX, ".fbx"},
    {FileType::IFC, ".ifc"},
    {FileType::PTS, ".xyz"}
};

// TODO - move in cpp
const static std::unordered_map<std::string, FileType> ExtensionDictionnary = {
    {".e57", FileType::E57},
    {".E57", FileType::E57},
    {".tls", FileType::TLS},
    {".fls", FileType::FARO_LS},
    {".lsproj", FileType::FARO_PROJ},
    {".rcs", FileType::RCS},
    {".rcp", FileType::RCP},
    {".step", FileType::STEP},
    {".stp", FileType::STEP},
    {".ifc", FileType::IFC},
    {".obj", FileType::OBJ},
    {".fbx", FileType::FBX},
    {".pts", FileType::PTS},
    {".xyz", FileType::PTS},
    {".dxf", FileType::DXF}
};

// TODO class FileUtils
// FileType getType(const std::filesystem::path& file_path);
// std::string getExtension(FileType type);
inline FileType getFileType(std::filesystem::path extension)
{
	std::string strExt = extension.string();
	std::transform(strExt.begin(), strExt.end(), strExt.begin(), [](unsigned char c) { return std::tolower(c); });

	if (ExtensionDictionnary.find(strExt) != ExtensionDictionnary.end()) {
		return ExtensionDictionnary.at(strExt);
	}
	else
		return FileType::MAX_ENUM;

}

inline std::string getFileExtension(FileType fileType)
{
    auto it = FileTypeDictionnary.find(fileType);
    if (it != FileTypeDictionnary.end())
        return it->second;
    else
        return "";
}

#endif 
