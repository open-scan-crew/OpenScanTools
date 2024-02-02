#include "pointCloudEngine/PCE_core.h"
#include "pointCloudEngine/TlScanOverseer.h"


bool tlGetScanGuid(const std::filesystem::path& filePath, tls::ScanGuid& scanGuid)
{
    return TlScanOverseer::getInstance().getScanGuid(filePath, scanGuid);
}

void tlFreeScan(tls::ScanGuid scanGuid)
{
    TlScanOverseer::getInstance().freeScan(scanGuid, false);
}

void tlHardDeleteScanFile(tls::ScanGuid scanGuid)
{
    TlScanOverseer::getInstance().freeScan(scanGuid, true);
}

void tlCopyScanFile(const tls::ScanGuid& scanGuid, const std::filesystem::path& destPath, bool savePath, bool overrideDestination, bool removeSource)
{
    TlScanOverseer::getInstance().addCopyScanFile(scanGuid, destPath, savePath, overrideDestination, removeSource);
}

bool tlGetScanHeader(tls::ScanGuid scanGuid, tls::ScanHeader &scanHeader)
{
    return TlScanOverseer::getInstance().getScanHeader(scanGuid, scanHeader);
}

bool tlGetCurrentScanPath(tls::ScanGuid scanGuid, std::filesystem::path& currentPath)
{
    return TlScanOverseer::getInstance().getScanPath(scanGuid, currentPath);
}

bool tlScanLeftToFree()
{
	return TlScanOverseer::getInstance().isScanLeftTofree();
}

tls::TBoundingBox<double> tlScansBoundingBox()
{
    return TlScanOverseer::getInstance().getScansBoundingBox();
}

std::list<tls::ScanHeader> tlScansHeaders()
{
    return TlScanOverseer::getInstance().getScansHeaders();
}

// ***** Stream Lock *****

TlStreamLock::TlStreamLock()
{
    TlScanOverseer::getInstance().haltStream();
}

TlStreamLock::~TlStreamLock()
{
    TlScanOverseer::getInstance().resumeStream();
}