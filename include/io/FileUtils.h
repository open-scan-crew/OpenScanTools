#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <filesystem>

enum FileType {E57, TLS, FARO_LS, FARO_PROJ, RCS, RCP, OBJ, STEP, FBX, IFC, DXF, PTS, MAX_ENUM };
enum class ObjectExportType { OBJ, FBX, DXF, CSV, STEP, OST};

class FileUtils
{
public:
    static FileType getType(const std::filesystem::path& _file_path);

    static std::string getExtension(FileType _file_type);
};

#endif 
