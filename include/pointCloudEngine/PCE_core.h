#ifndef PCE_CORE_H_
#define PCE_CORE_H_

//******** Name Ideas for the point cloud engine ********
// Light Point Exploit - LiPEx
// Fast Point Exploit - FaPEx
// Efficient Point Exploit - EPEx
// Efficient Point Rendering Exploit - EPRE
// Enhanced Ethereal Point Exploit - EEPE
// Enhanced Point Exploration Engine - EPEE
//*******************************************************

#include "models/pointCloud/tls.h"

#include "stdint.h"
#include <filesystem>

//******* Notes on how to use a TlScan***********
// Use a smart pointer as a TlScanref. If there is no more instance of the smart pointer, the Scanis freed
// A TlScancan be used by:
// - the controller : to get general info, to instruct a viewport to show the scan
// - a viewport : to actually draw the points
// - the streamer : to fetch the points data from the file to GPU memory on demand
// - a compute module [future] : to operate algorithm on the point data
//
// A TlScancan be referenced by a viewport without being drawn. In this case, the whole Scanis not freed but each point data can be freed independently because they are not drawn anymore.
//
//************************************************

bool tlGetScanGuid(const std::filesystem::path& filePath, tls::ScanGuid& scanGuid);

// Force the specified Scan to free all its system resources.
// All other scans housed on the same file are kept alive.
// Do not erase the Scan on the disk.
void tlFreeScan(tls::ScanGuid scanGuid);

// Delete the file on the disk. Free all the resources.
// Asynchronous: we must wait for the processes that use the resources to end.
// The TlScanFile must not be used after this function call.
void tlHardDeleteScanFile(tls::ScanGuid fileGuid);

void tlCopyScanFile(const tls::ScanGuid& scanGuid, const std::filesystem::path& destPath, bool savePath, bool overrideDestination, bool removeSource);

bool tlGetScanHeader(tls::ScanGuid scanGuid, tls::ScanHeader &scanHeader);

bool tlGetCurrentScanPath(tls::ScanGuid scanGuid, std::filesystem::path& currentPath);

bool tlScanLeftToFree();

std::list<tls::ScanHeader> tlScansHeaders();

class TlStreamLock
{
public:
    TlStreamLock();
    ~TlStreamLock();
};

#endif // _PCE_H_