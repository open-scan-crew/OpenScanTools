#ifndef CONVERT_PROPERTIES_H
#define CONVERT_PROPERTIES_H

#include <cstdint>
#include <filesystem>
#include <glm/glm.hpp>

struct ConvertProperties
{
    uint64_t				mask;
    uint32_t				filePrecision;
    uint32_t				fileFormat;
    bool					isCSV;
    bool					isImport;
    bool                    readFlsColor;
    bool                    overwriteExisting;
    std::filesystem::path	output;
    glm::dvec3              truncate;

    bool                    importAsObject;
    glm::dvec3              positionAsObject;
};

#endif