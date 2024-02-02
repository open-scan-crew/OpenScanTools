#ifndef CONVERT_PROPERTIES_H
#define CONVERT_PROPERTIES_H

#include <cstdint>
#include <filesystem>

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
    double                  truncateX;
    double                  truncateY;
    double                  truncateZ;
};

#endif