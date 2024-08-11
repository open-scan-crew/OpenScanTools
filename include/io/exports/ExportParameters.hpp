#ifndef EXPORT_PARAMETERS_H
#define EXPORT_PARAMETERS_H

#include <filesystem>
#include "io/FileUtils.h"
#include "models/pointCloud/TLS.h"
#include "models/OpenScanToolsModelEssentials.h"

// NOTE - The enums are used for combo box indexes
//  (*) They must start at 0 and the values increase "naturally".
//  (*) We need a MAX_ENUM to use them in a "for loop".

class ViewPointNode;

enum class ExportClippingFilter
{
    SELECTED = 0,
    ACTIVE,
    GRIDS,
    NONE,
    MAX_ENUM
};

enum class ExportClippingMethod
{
    SCAN_SEPARATED = 0,
    CLIPPING_SEPARATED,
    CLIPPING_AND_SCAN_MERGED,
    MAX_ENUM
};

struct PrimitivesExportParameters
{
	bool oneFilePerType = false;
    bool openFolderWindowsAfterExport = false;
    std::wstring csvSeparator = L"\t";
    bool exportWithScanImportTranslation = false;
};

enum class VideoAnimationMode
{
    NONE = 0,
    BETWEENVIEWPOINTS,
    ORBITAL,
    MAX_ENUM
};

struct VideoExportParameters
{
    int fps = 60;
    int length = 30;
    bool hdImage = true;
    VideoAnimationMode animMode = VideoAnimationMode::NONE;
    SafePtr<ViewPointNode> start;
    SafePtr<ViewPointNode> finish;
    bool interpolateRenderingBetweenViewpoints = false;
    bool openFolderAfterExport = false;

};

struct ClippingExportParameters
{
    ObjectStatusFilter pointCloudFilter;
    bool exportScans;
    bool exportPCOs;
    ExportClippingFilter clippingFilter;
    ExportClippingMethod method;
    FileType outFileType;
    tls::PrecisionType encodingPrecision;
    std::filesystem::path outFolder;
    std::filesystem::path tempFolder;
    std::filesystem::path fileName;
    double pointDensity;
    int maxScanPerProject; // if <= 0 there is no limit
    bool addOriginCube;
    bool openFolderAfterExport;

    bool exportWithScanImportTranslation = false;
}; 

struct PointCloudObjectParameters
{
    std::wstring fileName;
    double pointDensity;
};

#endif