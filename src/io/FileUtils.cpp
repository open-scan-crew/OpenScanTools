#include "io/FileUtils.h"
#include <unordered_map>

const std::unordered_map<FileType, std::string> FileTypeDictionnary = {
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

const std::unordered_map<std::string, FileType> ExtensionDictionnary = {
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

FileType FileUtils::getType(const std::filesystem::path& _file_path)
{
    std::string strExt = _file_path.extension().string();
    std::transform(strExt.begin(), strExt.end(), strExt.begin(), [](unsigned char c) { return std::tolower(c); });

    if (ExtensionDictionnary.find(strExt) != ExtensionDictionnary.end())
        return ExtensionDictionnary.at(strExt);
    else
        return FileType::MAX_ENUM;
}

std::string FileUtils::getExtension(FileType _file_type)
{
    auto it = FileTypeDictionnary.find(_file_type);
    if (it != FileTypeDictionnary.end())
        return it->second;
    else
        return "";
}
