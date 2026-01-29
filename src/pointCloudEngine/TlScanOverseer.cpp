#include "pointCloudEngine/TlScanOverseer.h"
#include "pointCloudEngine/EmbeddedScan.h"
#include "pointCloudEngine/OctreeRayTracing.h"
#include "pointCloudEngine/MeasureClass.h"
#include "pointCloudEngine/PCE_graphics.h"
#include "models/3d/Measures.h"
#include "utils/Logger.h"
#include <queue>
#include <glm/gtx/quaternion.hpp>
using namespace std::chrono;

#define IOLog LoggerMode::IOLog

constexpr double M_PI = 3.14159265358979323846;  /* pi */

thread_local std::vector<TlScanOverseer::WorkingScanInfo> TlScanOverseer::s_workingScansTransfo = {};

TlScanOverseer::TlScanOverseer()
    : m_haltStream(false)
{}

TlScanOverseer::~TlScanOverseer()
{
    Logger::log(IOLog) << "Destroy TlScanOverseer" << Logger::endl;
}

void TlScanOverseer::init()
{
    Logger::log(IOLog) << "Init TlScanOverseer." << Logger::endl;
}

void TlScanOverseer::shutdown()
{
    std::lock_guard<std::mutex> lock(m_activeMutex);
        
    // Force the deletion of Scanresources even if the safe frame is not reached
    for (auto scan : m_scansToFree)
    {
        delete (scan);
    }
    m_scansToFree.clear();

    for (auto scan : m_activeScans)
    {
        delete (scan.second);
    }
    m_activeScans.clear();
}

void TlScanOverseer::setWorkingScansTransfo(const std::vector<tls::PointCloudInstance>& workingTransfo)
{
    TlScanOverseer& instance = TlScanOverseer::getInstance();
    std::lock_guard<std::mutex> lock(instance.m_activeMutex);
    s_workingScansTransfo.clear();
    // Else find the working scans instance in the active scans
    for (const tls::PointCloudInstance& guid_transfo : workingTransfo)
    {
        auto it_scan = instance.m_activeScans.find(guid_transfo.header.guid);
        if (it_scan == instance.m_activeScans.end())
        {
            Logger::log(VKLog) << "Error: try to view a Scan not present, UUID = " << guid_transfo.header.guid << Logger::endl;
        }
        else
            s_workingScansTransfo.push_back({ *it_scan->second, guid_transfo.transfo, guid_transfo.isClippable });
            //s_workingScansTransfo.emplace_back(*it_scan->second, guid_transfo.tranfo, guid_transfo.isClippable);
    }
}

bool TlScanOverseer::getScanGuid(std::filesystem::path _filePath, tls::ScanGuid& _scanGuid)
{
    EmbeddedScan* newScan = new EmbeddedScan(_filePath);
    xg::Guid nullGuid;

    if (newScan->getGuid() == nullGuid)
    {
        _scanGuid = nullGuid;
        // No memory leak
        delete newScan; 
        return false;
    }

    std::lock_guard<std::mutex> lock(m_activeMutex);
    auto it_scan = m_activeScans.find(newScan->getGuid());
    if (it_scan != m_activeScans.end())
    {
        _scanGuid = it_scan->second->getGuid();
        // No memory leak
        delete newScan;
        return true;
    }

    m_activeScans.insert({ newScan->getGuid(), newScan });
    _scanGuid = newScan->getGuid();

    return true;
}

bool TlScanOverseer::getScanHeader(tls::ScanGuid scanGuid, tls::ScanHeader& info)
{
    std::lock_guard<std::mutex> lock(m_activeMutex);

    auto it_scan = m_activeScans.find(scanGuid);
    if (it_scan != m_activeScans.end())
    {
        it_scan->second->getInfo(info);
        return true;
    }
    else
    {
        Logger::log(IOLog) << "Info: try to get information of a Scan not present, UUID = " << scanGuid << Logger::endl;
        return false;
    }
}

bool TlScanOverseer::getScanPath(tls::ScanGuid scanGuid, std::filesystem::path& scanPath)
{
    std::lock_guard<std::mutex> lock(m_activeMutex);

    auto it_scan = m_activeScans.find(scanGuid);
    if (it_scan != m_activeScans.end())
    {
        scanPath = it_scan->second->getPath();
        return true;
    }
    else
    {
        Logger::log(IOLog) << "Info: try to get information of a Scan not present, UUID = " << scanGuid << Logger::endl;
        return false;
    }
}

bool  TlScanOverseer::isScanLeftTofree()
{
    std::lock_guard<std::mutex> lock(m_activeMutex);
    return !m_scansToFree.empty();
}

void TlScanOverseer::copyScanFile_async(const tls::ScanGuid& scanGuid, const std::filesystem::path& destPath, bool savePath, bool overrideDestination, bool removeSource)
{
    std::lock_guard<std::mutex> lock(m_copyMutex);

    m_waitingCopies.push_back({ scanGuid, destPath, savePath, overrideDestination, removeSource });
}

void TlScanOverseer::freeScan_async(tls::ScanGuid scanGuid, bool deletePhysicalFile)
{
    std::lock_guard<std::mutex> lock(m_activeMutex);

    // Find the Scanfile
    auto it_scan = m_activeScans.find(scanGuid);
    if (it_scan != m_activeScans.end())
    {
        EmbeddedScan* scanFile = it_scan->second;

        scanFile->deleteFileWhenDestroyed(deletePhysicalFile);

        // place the object to the waiting deletion list
        m_scansToFree.push_back(scanFile);

        m_activeScans.erase(it_scan);
    }
    else
    {
        Logger::log(IOLog) << "Info: Try to free a Scanfile not present, UUID = " << scanGuid << Logger::endl;
    }
}

void TlScanOverseer::resourceManagement_sync()
{
    // do all waiting file management
    syncFileCopy();

    // Apply the waiting free command
    freeWaitingResources();
}

void TlScanOverseer::syncFileCopy()
{
    std::lock_guard<std::mutex> lock(m_copyMutex);
    // Apply the waiting copy command
    for (auto copy : m_waitingCopies)
    {
        // FIXME(robin) Il faut faire parvenir le résultat de applyCopy() au controller sinon ça ne sert à rien de renvoyer un booléen.
        if (!doFileCopy(copy))
            continue;
    }
    m_waitingCopies.clear();
}

bool TlScanOverseer::doFileCopy(scanCopyInfo& copyInfo) 
{
    Logger::log(IOLog) << "INFO - try to copy the file {" << copyInfo.guid << "} to " << copyInfo.path << Logger::endl;

    EmbeddedScan* oldScan = nullptr;
    {
        std::lock_guard<std::mutex> lock(m_activeMutex);
        // Find the EmbeddedScan
        auto it_scan = m_activeScans.find(copyInfo.guid);
        if (it_scan != m_activeScans.end())
        {
            oldScan = it_scan->second;
        }
        else
        {
            Logger::log(IOLog) << "ERROR - the tls file {" << copyInfo.guid << "} you try to copy is not registered." << Logger::endl;
            return false;
        }
    }

    try {
        // Check that the destination path is free
        if (!copyInfo.overrideDestination && std::filesystem::exists(copyInfo.path))
        {
            Logger::log(IOLog) << "INFO - the destination path already exists." << Logger::endl;
            // Check if the destination file is the file itself
            if (std::filesystem::equivalent(oldScan->getPath(), copyInfo.path) || std::filesystem::file_size(oldScan->getPath()) == std::filesystem::file_size(copyInfo.path))
            {
                Logger::log(IOLog) << "INFO - the file " << copyInfo.path << " is already at the destination requested." << Logger::endl;
                return true;
            }

            // Check if the destination file is the same but with a different name
            EmbeddedScan* newScanFile = new EmbeddedScan(copyInfo.path);
            if (newScanFile->getGuid() == copyInfo.guid)
            {
                std::lock_guard<std::mutex> lock(m_activeMutex);

                m_activeScans.erase(copyInfo.guid);
                delete oldScan;

                m_activeScans.insert({ copyInfo.guid, newScanFile });
                return true;
            }
            else
                delete newScanFile;

            // The destination file exist but it is not the file we want
            // We cannot do the copy unless we determine a new destination file name
            // TODO - check in the controller before trying to copy
            return false;
        }
        else
        {
            std::filesystem::copy_options options = std::filesystem::copy_options::none;
            options |= copyInfo.overrideDestination ? std::filesystem::copy_options::overwrite_existing : std::filesystem::copy_options::skip_existing;
            std::filesystem::path old_path = oldScan->getPath();

            std::filesystem::copy(old_path, copyInfo.path, options);

            if (copyInfo.savePath || copyInfo.removeSource)
                oldScan->setPath(copyInfo.path);

            if (copyInfo.removeSource)
                std::filesystem::remove(old_path);

            return true;
        }
    }
    catch (std::exception& e)
    {
        Logger::log(IOLog) << "Exception occured when copying a scan file: " << e.what() << Logger::endl;
    }

    return false;
}

void TlScanOverseer::freeWaitingResources()
{
    std::lock_guard<std::mutex> lock(m_activeMutex);

    for (auto it = m_scansToFree.begin(); it != m_scansToFree.end(); )
    {
        if ((*it)->canBeDeleted())
        {
            delete (*it);
            it = m_scansToFree.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

//---------------------- Graphical Processing ------------------------//

bool TlScanOverseer::getScanView(tls::ScanGuid _scanGuid, const TlProjectionInfo& _projInfo, const ClippingAssembly& _clippingAssembly, TlScanDrawInfo& _scanDrawInfo, bool& _needStreaming)
{
    // mutex active scan lookup
    bool globalOk;
    EmbeddedScan* scan;
    {
        std::lock_guard<std::mutex> lock(m_activeMutex);

        auto it_scan = m_activeScans.find(_scanGuid);
        if (it_scan == m_activeScans.end())
        {
            Logger::log(VKLog) << "Error: try to view a scan not present, UUID = " << _scanGuid << Logger::endl;
            return false;
        }
        else
        {
            scan = it_scan->second;
            // The current frame index lock the scan resources before the mutex is unlock
            globalOk = scan->getGlobalDrawInfo(_scanDrawInfo);
        }
    }

    _scanDrawInfo.cellDrawInfo.clear();

    if (_clippingAssembly.clippingUnion.empty() && _clippingAssembly.clippingIntersection.empty() && _clippingAssembly.rampActives.empty())
        _needStreaming = !scan->viewOctree(_scanDrawInfo.cellDrawInfo, _projInfo);
    else
        _needStreaming = !scan->viewOctreeCB(_scanDrawInfo.cellDrawInfo, _scanDrawInfo.cellDrawInfoCB, _projInfo, _clippingAssembly);

    return (globalOk);
}

void TlScanOverseer::haltStream()
{
    m_haltStream.store(true);
}

void TlScanOverseer::resumeStream()
{
    m_haltStream.store(false);
}

void TlScanOverseer::streamScans(uint64_t maxSize, char* stagingBuffer, std::vector<TlStagedTransferInfo>& stageTransfer)
{
    std::unordered_map<tls::ScanGuid, EmbeddedScan*> copyActiveScans;
    {
        std::lock_guard<std::mutex> lock(m_activeMutex);
        copyActiveScans = m_activeScans;
    }

    uint64_t stageOffset = 0;
    for (auto activeScan : copyActiveScans)
    {
        // Skip the frame of streaming if another process asked for an halt
        if (m_haltStream.load() == true)
            return;
        
        if (activeScan.second->startStreamingAll(stagingBuffer, maxSize, stageOffset, stageTransfer) == false)
            break;
    }
}

bool TlScanOverseer::testClippingEffect(tls::ScanGuid _scanGuid, const TransformationModule& _modelMat, const ClippingAssembly& _clippingAssembly)
{
    EmbeddedScan* scan;
    {
        std::lock_guard<std::mutex> lock(m_activeMutex);

        auto it_scan = m_activeScans.find(_scanGuid);
        if (it_scan == m_activeScans.end())
        {
            Logger::log(VKLog) << "Error: try to view a Scan not present, UUID = " << _scanGuid << Logger::endl;
            return false;
        }
        else
        {
            scan = it_scan->second;
        }
    }

    return scan->testPointsClippedOut(_modelMat, _clippingAssembly);
}

bool TlScanOverseer::clipScan(tls::ScanGuid _scanGuid, const TransformationModule& _modelMat, const ClippingAssembly& _clippingAssembly, IScanFileWriter* _outScan, const ProgressCallback& progress)
{
    EmbeddedScan* scan;
    {
        std::lock_guard<std::mutex> lock(m_activeMutex);

        auto it_scan = m_activeScans.find(_scanGuid);
        if (it_scan == m_activeScans.end())
        {
            Logger::log(VKLog) << "Error: try to view a Scan not present, UUID = " << _scanGuid << Logger::endl;
            return false;
        }
        else
        {
            scan = it_scan->second;
        }
    }

    // Send the fileWriter to the TlScan and let it do the points/cells specific job.
    return scan->clipAndWrite(_modelMat, _clippingAssembly, _outScan, progress);
}

bool TlScanOverseer::computeOutlierStats(tls::ScanGuid _scanGuid, const TransformationModule& _modelMat, const ClippingAssembly& _clippingAssembly, int kNeighbors, int samplingPercent, double beta, OutlierStats& stats, const ProgressCallback& progress)
{
    EmbeddedScan* scan;
    {
        std::lock_guard<std::mutex> lock(m_activeMutex);

        auto it_scan = m_activeScans.find(_scanGuid);
        if (it_scan == m_activeScans.end())
        {
            Logger::log(VKLog) << "Error: try to view a Scan not present, UUID = " << _scanGuid << Logger::endl;
            return false;
        }
        else
        {
            scan = it_scan->second;
        }
    }

    return scan->computeOutlierStats(_modelMat, _clippingAssembly, kNeighbors, samplingPercent, beta, stats, progress);
}

bool TlScanOverseer::filterOutliersAndWrite(tls::ScanGuid _scanGuid, const TransformationModule& _modelMat, const ClippingAssembly& _clippingAssembly, int kNeighbors, const OutlierStats& stats, double nSigma, double beta, IScanFileWriter* _outScan, uint64_t& removedPoints, const ProgressCallback& progress)
{
    EmbeddedScan* scan;
    {
        std::lock_guard<std::mutex> lock(m_activeMutex);

        auto it_scan = m_activeScans.find(_scanGuid);
        if (it_scan == m_activeScans.end())
        {
            Logger::log(VKLog) << "Error: try to view a Scan not present, UUID = " << _scanGuid << Logger::endl;
            return false;
        }
        else
        {
            scan = it_scan->second;
        }
    }

    return scan->filterOutliersAndWrite(_modelMat, _clippingAssembly, kNeighbors, stats, nSigma, beta, _outScan, removedPoints, progress);
}

bool TlScanOverseer::colorBalanceAndWrite(tls::ScanGuid scanGuid, const std::vector<tls::ScanGuid>& otherScanGuids, const TransformationModule& modelMat, const ClippingAssembly& clippingAssembly, const ColorBalanceSettings& settings, IScanFileWriter* outScan, uint64_t& adjustedPoints, const ProgressCallback& progress)
{
    EmbeddedScan* scan;
    std::vector<EmbeddedScan*> otherScans;
    size_t activeScanCount = 0;
    {
        std::lock_guard<std::mutex> lock(m_activeMutex);

        activeScanCount = m_activeScans.size();
        auto it_scan = m_activeScans.find(scanGuid);
        if (it_scan == m_activeScans.end())
        {
            Logger::log(VKLog) << "Error: try to view a Scan not present, UUID = " << scanGuid << Logger::endl;
            return false;
        }
        scan = it_scan->second;

        otherScans.reserve(otherScanGuids.size());
        for (const tls::ScanGuid& guid : otherScanGuids)
        {
            auto it_other = m_activeScans.find(guid);
            if (it_other == m_activeScans.end())
                continue;
            if (it_other->second == scan)
                continue;
            otherScans.push_back(it_other->second);
        }
    }

    Logger::log(LoggerMode::FunctionLog) << "Color balance: active scans=" << activeScanCount
                                         << " otherScans=" << otherScans.size() << Logger::endl;

    return scan->colorBalanceAndWrite(modelMat, clippingAssembly, otherScans, settings, outScan, adjustedPoints, progress);
}

//tls::ScanGuid TlScanOverseer::clipNewScan(tls::ScanGuid _scanGuid, const glm::dmat4& _modelMat, const ClippingAssembly& _clippingAssembly, const std::filesystem::path& _outPath, uint64_t& pointsDeletedCount)
//{
//    EmbeddedScan* scan;
//    {
//        std::lock_guard<std::mutex> lock(m_activeMutex);
//
//        auto it_scan = m_activeScans.find(_scanGuid);
//        if (it_scan == m_activeScans.end())
//        {
//            Logger::log(VKLog) << "Error: try to view a Scan not present, UUID = " << _scanGuid << Logger::endl;
//            return xg::Guid();
//        }
//        else
//        {
//            scan = it_scan->second;
//        }
//    }
//
//    OctreeShredder shredder(scan->getPath());
//    pointsDeletedCount = shredder.cutPoints(_modelMat, _clippingAssembly);
//    if (shredder.isEmpty())
//    {
//        // TODO - Maybe there is no points remaining OR maybe this is an error
//        return xg::Guid();
//    }
//    if (pointsDeletedCount == 0)
//    {
//        // On n'a pas besoin de sauvegarder l'OctreeShredder
//        return _scanGuid;
//    }
//
//    tls::ScanHeader scanHeader;
//    scan->getInfo(scanHeader);
//    scanHeader.guid = xg::newGuid();
//    if (shredder.save(_outPath, scanHeader) == false)
//    {
//        return xg::Guid();
//    }
//
//    EmbeddedScan* newScan = new EmbeddedScan(_outPath);
//
//    if (EmbeddedScan::movePointBuffers(*newScan, *scan, shredder.getCorrespCellId()) == false)
//    {
//        Logger::log(VKLog) << "Error: cannot move the point buffers from scan " << _scanGuid << " to " << newScan->getGuid() << Logger::endl;
//    }
//
//    // Store the new scan preloaded
//    {
//        std::lock_guard<std::mutex> lock(m_activeMutex);
//        m_activeScans.insert({ newScan->getGuid(), newScan });
//
//        // This is done with a freeScan()
//        //delete scan;
//        //scan = nullptr;
//        //m_activeScans.erase(_scanGuid);
//    }
//
//    return newScan->getGuid();
//}

bool TlScanOverseer::rayTracing(const glm::dvec3& ray, const glm::dvec3& rayOrigin, glm::dvec3& bestPoint, const double& cosAngleThreshold, const ClippingAssembly& clippingAssembly, const bool& isOrtho, std::string& scanName)
{
    double rayRadius(0.0015);
    std::vector<glm::dvec3> pointList;
    std::vector<std::string> scanNames;
    for (const WorkingScanInfo& _pair : s_workingScansTransfo)
    {
        
        glm::dvec3 currentBest;
        _pair.scan.setComputeTransfo(_pair.transfo.getCenter(), _pair.transfo.getOrientation());
        ClippingAssembly localAssembly;
        if (_pair.isClippable)
        {
            localAssembly = clippingAssembly;
        }
        bool test = _pair.scan.beginRayTracing(ray, rayOrigin, currentBest, cosAngleThreshold, localAssembly, isOrtho);
        if ((test) && (!std::isnan(currentBest.x)))
        {
            pointList.push_back(currentBest);
            tls::ScanHeader info;
            _pair.scan.getInfo(info);
            scanNames.push_back(Utils::to_utf8(info.name));
        }
    }

    if (pointList.size() > 0)
    {
        std::vector<std::vector<glm::dvec3>> pointListFormated;
        pointListFormated.push_back(pointList);
        ClippingAssembly localAssembly;
        bestPoint = OctreeRayTracing::findBestPoint(pointListFormated, ray / glm::length(ray), rayOrigin, cosAngleThreshold, rayRadius, localAssembly, isOrtho);

        //look for scan
        double bestLength(DBL_MAX);
        for (int i = 0; i < pointList.size(); i++)
        {
            if (glm::length(bestPoint - pointList[i]) < bestLength)
            {
                bestLength = glm::length(bestPoint - pointList[i]);
                scanName = scanNames[i];
            }
        }
        return true;
    }

    return false;
}
bool TlScanOverseer::findNeighborsBucketsDirected(const glm::dvec3& globalSeedPoint, const glm::dvec3& directedPoint, const double& radius, std::vector<std::vector<glm::dvec3>>& neighborList, int numberOfBuckets, const ClippingAssembly& clippingAssembly)
{
    bool result = false;

    for (const WorkingScanInfo& _pair : s_workingScansTransfo)
    {
        ClippingAssembly localAssembly;
        if (_pair.isClippable)
        {
            localAssembly = clippingAssembly;
        }
        _pair.scan.setComputeTransfo(_pair.transfo.getCenter(), _pair.transfo.getOrientation());
        result |= _pair.scan.findNeighborsBucketsDirected(globalSeedPoint, directedPoint, radius, neighborList, numberOfBuckets, localAssembly);
    }

    Logger::log(LoggerMode::rayTracingLog) << "NEIGHBORS FINISHED" << Logger::endl;

    return result;
}
bool TlScanOverseer::findNeighborsBuckets(const glm::dvec3& globalSeedPoint, const double& radius, std::vector<std::vector<glm::dvec3>>& neighborList, int numberOfBuckets, const ClippingAssembly& clippingAssembly)
{
    bool result = false;
    for (const WorkingScanInfo& _pair : s_workingScansTransfo)
    {
        ClippingAssembly localAssembly;
        if (_pair.isClippable)
        {
            localAssembly = clippingAssembly;
        }
        _pair.scan.setComputeTransfo(_pair.transfo.getCenter(), _pair.transfo.getOrientation());
        result |= _pair.scan.findNeighborsBuckets(globalSeedPoint, radius, neighborList, numberOfBuckets, localAssembly);
    }
    Logger::log(LoggerMode::rayTracingLog) << "NEIGHBORS FINISHED" << Logger::endl;

    return result;
}

//for testing

bool TlScanOverseer::findNeighborsBucketsTest(const glm::dvec3& globalSeedPoint, const double& radius, std::vector<std::vector<glm::dvec3>>& neighborList, int numberOfBuckets, const ClippingAssembly& clippingAssembly, std::vector<glm::dquat>& rotations, std::vector<glm::dvec3>& positions, std::vector<double>& scales)
{
    bool result = false;
    for (const WorkingScanInfo& _pair : s_workingScansTransfo)
    {
        ClippingAssembly localAssembly;
        if (_pair.isClippable)
        {
            localAssembly = clippingAssembly;
        }
        _pair.scan.setComputeTransfo(_pair.transfo.getCenter(), _pair.transfo.getOrientation());
        result |= _pair.scan.findNeighborsBucketsTest(globalSeedPoint, radius, neighborList, numberOfBuckets, localAssembly, rotations, positions, scales);
    }
    Logger::log(LoggerMode::rayTracingLog) << "NEIGHBORS FINISHED" << Logger::endl;

    return result;
}

bool TlScanOverseer::findNeighbors(const glm::dvec3& globalSeedPoint, const double& radius, std::vector<glm::dvec3>& neighborList, const ClippingAssembly& clippingAssembly)
{
    bool success = false;
    for (const WorkingScanInfo& _pair : s_workingScansTransfo)
    {
        ClippingAssembly localAssembly;
        if (_pair.isClippable)
        {
            localAssembly = clippingAssembly;
        }
        _pair.scan.setComputeTransfo(_pair.transfo.getCenter(), _pair.transfo.getOrientation());
        success |= _pair.scan.findNeighbors(globalSeedPoint, radius, neighborList, localAssembly);
    }
    return true;
}

bool TlScanOverseer::findNeighborsTowardsPoint(const glm::dvec3& globalSeedPoint, const glm::dvec3& targetPoint, const double& radius, std::vector<glm::dvec3>& neighborList, const ClippingAssembly& clippingAssembly)
{
    bool success = false;
    for (const WorkingScanInfo& _pair : s_workingScansTransfo)
    {
        ClippingAssembly localAssembly;
        if (_pair.isClippable)
        {
            localAssembly = clippingAssembly;
        }
        _pair.scan.setComputeTransfo(_pair.transfo.getCenter(), _pair.transfo.getOrientation());
        success |= _pair.scan.findNeighborsTowardsPoint(globalSeedPoint, targetPoint, radius, neighborList, localAssembly);
    }
    return true;
}

bool TlScanOverseer::nearestNeighbor(const glm::dvec3& globalPoint, glm::dvec3& result)
{
    result = glm::dvec3(0.0, 0.0, 0.0);
    glm::dvec3 temp;
    bool success(false);

    for (const WorkingScanInfo& _pair : s_workingScansTransfo)
    {
        _pair.scan.setComputeTransfo(_pair.transfo.getCenter(), _pair.transfo.getOrientation());
        if (_pair.scan.nearestNeighbor(globalPoint, temp))
        {
            success = true;
            if (glm::length(globalPoint - temp) < glm::length(globalPoint - result))
                result = temp;
        }
    }
    return success;
}

bool TlScanOverseer::fitCylinder(const glm::dvec3& globalSeedPoint, const double& radius, const double& threshold, double& cylinderRadius, glm::dvec3& cylinderDirection, glm::dvec3& cylinderCenter, const FitCylinderMode& mode, const ClippingAssembly& clippingAssembly)
{
    std::vector<std::vector<glm::dvec3>> neighborList, secondList;
    std::vector<glm::dvec3> temp;
    glm::dvec3 cCenter, cDirection;
    double cRadius, searchRadius(radius);
    bool doubleCheck(false);
    int numberOfBuckets;
    switch (mode)
    {
    case::FitCylinderMode::fast:
    {
        numberOfBuckets = 2;
        searchRadius = 0.15;
        break;
    }
    case::FitCylinderMode::robust:
    {
        numberOfBuckets = 5;
        break;
    }
    case::FitCylinderMode::multiple:
    {
        numberOfBuckets = 5;
        doubleCheck = true;
    }
    }


    for (int i = 0; i < numberOfBuckets; i++)
    {
        neighborList.push_back(temp);
        secondList.push_back(temp);
    }
    findNeighborsBuckets(globalSeedPoint, searchRadius, neighborList, numberOfBuckets, clippingAssembly);
    if (!OctreeRayTracing::beginCylinderFit(neighborList, threshold, cylinderRadius, cylinderDirection, cylinderCenter, 10000))
        return false;
    if (!doubleCheck)
    {
        return true;
        if (numberOfBuckets > 2)
        {
            glm::dvec3 otherSeed = globalSeedPoint + 0.1*cylinderDirection;
            findNeighborsBuckets(otherSeed, searchRadius, secondList, numberOfBuckets, clippingAssembly);
            for (int i = 0; i < numberOfBuckets; i++)
            {
                for (int j = 0; j < (int)secondList[i].size(); j++)
                    neighborList[i].push_back(secondList[i][j]);
            }
            return OctreeRayTracing::beginCylinderFit(neighborList, threshold, cylinderRadius, cylinderDirection, cylinderCenter, 10000);
        }
        else
            return true;
    }
    if (neighborList[0].size() == 0)
        return false;
    int randomIndex = rand() % (neighborList[0].size());
    findNeighborsBuckets(neighborList[0][randomIndex], searchRadius, secondList, numberOfBuckets, clippingAssembly);
    if (!OctreeRayTracing::beginCylinderFit(secondList, threshold, cRadius, cDirection, cCenter, 10000))
        return false;
    std::vector<glm::dvec3> vecCCenter, vecCDirection;
    std::vector<double> vecCRadius;
    vecCCenter.push_back(cylinderCenter);
    vecCDirection.push_back(cylinderDirection);
    vecCRadius.push_back(cylinderRadius);
    return (isCylinderCloseToPreviousCylinders(vecCCenter, vecCDirection, vecCRadius, cCenter, cDirection, cRadius));
}

//for testing

bool TlScanOverseer::fitCylinderTest(const glm::dvec3& globalSeedPoint, const double& radius, const double& threshold, double& cylinderRadius, glm::dvec3& cylinderDirection, glm::dvec3& cylinderCenter, const FitCylinderMode& mode, const ClippingAssembly& clippingAssembly, std::vector<glm::dquat>& rotations, std::vector<glm::dvec3>& positions, std::vector<double>& scales)
{
    std::vector<std::vector<glm::dvec3>> neighborList, secondList;
    std::vector<glm::dvec3> temp;
    glm::dvec3 cCenter, cDirection;
    double cRadius, searchRadius(radius);
    bool doubleCheck(false);
    int numberOfBuckets;
    switch (mode)
    {
    case::FitCylinderMode::fast:
    {
        numberOfBuckets = 2;
        searchRadius = 0.2;
        break;
    }
    case::FitCylinderMode::robust:
    {
        numberOfBuckets = 5;
        break;
    }
    case::FitCylinderMode::multiple:
    {
        numberOfBuckets = 5;
        doubleCheck = true;
    }
    }


    for (int i = 0; i < numberOfBuckets; i++)
    {
        neighborList.push_back(temp);
        secondList.push_back(temp);
    }

    findNeighborsBucketsTest(globalSeedPoint, searchRadius, neighborList, numberOfBuckets, clippingAssembly,rotations,positions,scales);
    if (!OctreeRayTracing::beginCylinderFit(neighborList, threshold, cylinderRadius, cylinderDirection, cylinderCenter, 10000))
        return false;
    if (!doubleCheck)
    {
        return true;
        if (numberOfBuckets > 2)
        {
            glm::dvec3 otherSeed = globalSeedPoint + 0.1*cylinderDirection;
            findNeighborsBucketsTest(globalSeedPoint, searchRadius, neighborList, numberOfBuckets, clippingAssembly, rotations, positions, scales);
            for (int i = 0; i < numberOfBuckets; i++)
            {
                for (int j = 0; j < (int)secondList[i].size(); j++)
                    neighborList[i].push_back(secondList[i][j]);
            }
            return OctreeRayTracing::beginCylinderFit(neighborList, threshold, cylinderRadius, cylinderDirection, cylinderCenter, 10000);
        }
        else
            return true;
    }
    int randomIndex = rand() % (neighborList[0].size());
    findNeighborsBucketsTest(globalSeedPoint, searchRadius, neighborList, numberOfBuckets, clippingAssembly, rotations, positions, scales);
    if (!OctreeRayTracing::beginCylinderFit(secondList, threshold, cRadius, cDirection, cCenter, 10000))
        return false;
    std::vector<glm::dvec3> vecCCenter, vecCDirection;
    std::vector<double> vecCRadius;
    vecCCenter.push_back(cylinderCenter);
    vecCDirection.push_back(cylinderDirection);
    vecCRadius.push_back(cylinderRadius);
    return (isCylinderCloseToPreviousCylinders(vecCCenter, vecCDirection, vecCRadius, cCenter, cDirection, cRadius));
}

bool TlScanOverseer::fitCylinderMultipleSeeds(const std::vector<glm::dvec3>& globalSeedPoint, const double& radius, const double& threshold, double& cylinderRadius, glm::dvec3& cylinderDirection, glm::dvec3& cylinderCenter, const FitCylinderMode& mode, const ClippingAssembly& clippingAssembly)
{
    if ((int)globalSeedPoint.size() != 2)
        return false;
    std::vector<std::vector<glm::dvec3>> neighborList, secondList;
    std::vector<glm::dvec3> temp;
    glm::dvec3 cCenter, cDirection;
    double cRadius, searchRadius(radius);
    bool doubleCheck(false);
    int numberOfBuckets;
    switch (mode)
    {
    case::FitCylinderMode::fast:
    {
        numberOfBuckets = 2;
        searchRadius = 0.15;
        break;
    }
    case::FitCylinderMode::robust:
    {
        numberOfBuckets = 5;
        break;
    }
    case::FitCylinderMode::multiple:
    {
        numberOfBuckets = 5;
        doubleCheck = true;
    }
    }


    for (int i = 0; i < numberOfBuckets; i++)
    {
        neighborList.push_back(temp);
        secondList.push_back(temp);
    }
    //for(int i=0;i<(int)globalSeedPoint.size();i++)
        //findNeighborsBuckets(globalSeedPoint[i], searchRadius, neighborList, numberOfBuckets, clippingAssembly);
    findNeighborsBucketsDirected(globalSeedPoint[0], globalSeedPoint[1], searchRadius, neighborList, numberOfBuckets, clippingAssembly);
    findNeighborsBucketsDirected(globalSeedPoint[1], globalSeedPoint[0], searchRadius, neighborList, numberOfBuckets, clippingAssembly);

    if (!OctreeRayTracing::beginCylinderFit(neighborList, threshold, cylinderRadius, cylinderDirection, cylinderCenter, 10000))
        return false;
    if (!doubleCheck)
    {
        return true;
        /*if (numberOfBuckets > 2)
        {
            glm::dvec3 otherSeed = globalSeedPoint + 0.1*cylinderDirection;
            findNeighborsBuckets(otherSeed, searchRadius, secondList, numberOfBuckets, clippingAssembly);
            for (int i = 0; i < numberOfBuckets; i++)
            {
                for (int j = 0; j < (int)secondList[i].size(); j++)
                    neighborList[i].push_back(secondList[i][j]);
            }
            return OctreeRayTracing::beginCylinderFit(neighborList, threshold, cylinderRadius, cylinderDirection, cylinderCenter, 10000);
        }
        else
            return true;*/
    }
    if (neighborList[0].size() == 0)
        return false;
    int randomIndex = rand() % (neighborList[0].size());
    findNeighborsBuckets(neighborList[0][randomIndex], searchRadius, secondList, numberOfBuckets, clippingAssembly);
    if (!OctreeRayTracing::beginCylinderFit(secondList, threshold, cRadius, cDirection, cCenter, 10000))
        return false;
    std::vector<glm::dvec3> vecCCenter, vecCDirection;
    std::vector<double> vecCRadius;
    vecCCenter.push_back(cylinderCenter);
    vecCDirection.push_back(cylinderDirection);
    vecCRadius.push_back(cylinderRadius);
    return (isCylinderCloseToPreviousCylinders(vecCCenter, vecCDirection, vecCRadius, cCenter, cDirection, cRadius));
}

/*bool TlScanOverseer::fitBigCylinder2(const glm::dvec3& seedPoint1, const glm::dvec3& seedPoint2, const double& radius, const double& threshold, double& cylinderRadius, glm::dvec3& cylinderDirection, glm::dvec3& cylinderCenter, const ClippingAssembly& clippingAssembly)
{
    std::vector<std::vector<glm::dvec3>> neighborList1, neighborList2, testNeighbors1, testNeighbors2, finalNeighbors;
    std::vector<glm::dvec3> temp;
    finalNeighbors.push_back(temp);
    finalNeighbors.push_back(temp);

    int numberOfBuckets(5);
    for (int i = 0; i < numberOfBuckets; i++)
    {
        neighborList1.push_back(temp);
        neighborList2.push_back(temp);
    }
    testNeighbors1.push_back(temp);
    testNeighbors2.push_back(temp);
    findNeighborsBuckets(seedPoint1, radius, neighborList1, numberOfBuckets);
    findNeighborsBuckets(seedPoint2, radius, neighborList2, numberOfBuckets);
    int region1(5), region2(5);*/
    /*for (int i = 0; i < numberOfBuckets; i++)
    {
        testNeighbors1[0].insert(testNeighbors1[0].begin(),neighborList1[i].begin(),neighborList1[i].end());
        if ((OctreeRayTracing::beginCylinderFit(testNeighbors1, threshold, cylinderRadius, cylinderDirection, cylinderCenter, 10000))||(i==0))
            region1=i;
    }
    for (int i = 0; i < numberOfBuckets; i++)
    {
        testNeighbors2[0].insert(testNeighbors2[0].begin(), neighborList2[i].begin(), neighborList2[i].end());
        if ((OctreeRayTracing::beginCylinderFit(testNeighbors2, threshold, cylinderRadius, cylinderDirection, cylinderCenter, 10000))||(i==0))
            region2=i;
    }*/
    /*for (int i = 0; i < region1; i++)
    {
        finalNeighbors[0].insert(finalNeighbors[0].begin(), neighborList1[i].begin(), neighborList1[i].end());
    }
    for (int i = 0; i < region2; i++)
    {
        finalNeighbors[1].insert(finalNeighbors[1].begin(), neighborList2[i].begin(), neighborList2[i].end());
    }
    int maxNumberOfPoints(30000);
    srand((unsigned int)time(NULL));
    std::vector<std::vector<glm::dvec3>> sampledFinalNeighbors;
    sampledFinalNeighbors.push_back(temp);

    std::vector<glm::dvec3> otherTemp;
    for (int loop = 0; loop < maxNumberOfPoints; loop++)
    {
        int v1 = rand() % 10000;
        int v2 = rand() % (finalNeighbors[0].size() / (int)1000);
        int index = (v1 + 10000 * v2) % finalNeighbors[0].size();

        otherTemp.push_back(finalNeighbors[0][index]);			//insert stuff to pick points at random instead
    }

    sampledFinalNeighbors[0] = otherTemp;
    otherTemp = temp;
    for (int loop = 0; loop < maxNumberOfPoints; loop++)
    {
        int v1 = rand() % 10000;
        int v2 = rand() % (1 + (finalNeighbors[1].size() / (int)1000));
        int index = (v1 + 10000 * v2) % finalNeighbors[1].size();

        otherTemp.push_back(finalNeighbors[1][index]);			//insert stuff to pick points at random instead
    }
    sampledFinalNeighbors[0].insert(sampledFinalNeighbors[0].begin(), otherTemp.begin(), otherTemp.end());


    return OctreeRayTracing::beginCylinderFit(sampledFinalNeighbors, threshold, cylinderRadius, cylinderDirection, cylinderCenter, 60000);
    /////////////////////////////////
    /*std::vector<std::vector<glm::dvec3>> neighborList;
    std::vector<glm::dvec3> temp;
    int numberOfBuckets(10);
    for (int i = 0; i < numberOfBuckets; i++)
    {
        neighborList.push_back(temp);
    }
    findNeighborsBuckets(seedPoint1, radius, neighborList, numberOfBuckets);
    findNeighborsBuckets(seedPoint2, radius, neighborList, numberOfBuckets);

    return OctreeRayTracing::beginCylinderFit(neighborList, cylinderRadius, cylinderDirection, cylinderCenter);*/
//}
bool TlScanOverseer::fitCylinder4Clicks(const std::vector<glm::dvec3>& seedPoints, const double& radius, const double& threshold, double& cylinderRadius, glm::dvec3& cylinderDirection, glm::dvec3& cylinderCenter, const ClippingAssembly& clippingAssembly)
{
    if (!(seedPoints.size() == 4))
        return false;
    int maxNumberOfPoints(5000);
    int numberOfBuckets(5);
    double searchRadius1(0.3),searchRadius2(0.3);
    if (glm::length(seedPoints[0] - seedPoints[1]) < 0.5*searchRadius1)
        searchRadius1 = 0.5*glm::length(seedPoints[0] - seedPoints[1]);
    if (glm::length(seedPoints[2] - seedPoints[3]) < 0.5*searchRadius2)
        searchRadius2 = 0.5*glm::length(seedPoints[2] - seedPoints[3]);
    std::vector<std::vector<glm::dvec3>> neighborList;
    std::vector<glm::dvec3> temp;

    for (int i = 0; i < numberOfBuckets; i++)
    {
        neighborList.push_back(temp);
    }
    findNeighborsBucketsDirected(seedPoints[0], seedPoints[1], searchRadius1, neighborList, numberOfBuckets, clippingAssembly);
    findNeighborsBucketsDirected(seedPoints[1], seedPoints[0], searchRadius1, neighborList, numberOfBuckets, clippingAssembly);
    findNeighborsBucketsDirected(seedPoints[2], seedPoints[3], searchRadius1, neighborList, numberOfBuckets, clippingAssembly);
    findNeighborsBucketsDirected(seedPoints[3], seedPoints[2], searchRadius1, neighborList, numberOfBuckets, clippingAssembly);
    bool success = OctreeRayTracing::beginCylinderFit(neighborList, 0.01, cylinderRadius, cylinderDirection, cylinderCenter, 3 * maxNumberOfPoints);
    return success;
}

bool TlScanOverseer::fitBigCylinder(const glm::dvec3& seedPoint1, const glm::dvec3& seedPoint2, const double& radius, const double& threshold, double& cylinderRadius, glm::dvec3& cylinderDirection, glm::dvec3& cylinderCenter, std::vector<glm::dvec3>& tags, std::vector<std::vector<double>>& planes, const ClippingAssembly& clippingAssembly)
{
    int maxNumberOfPoints(5000);
    int minNumberOfPoints(20);
    int numberOfBuckets(1);
    srand((unsigned int)time(NULL));

    std::vector<double> plane1, plane2;
    if (!fitPlane(seedPoint1, plane1, clippingAssembly))
        return false;
    if (!fitPlane(seedPoint2, plane2, clippingAssembly))
        return false;
    planes.push_back(plane1);
    planes.push_back(plane2);
    glm::dvec3 normal1(plane1[0], plane1[1], plane1[2]);
    glm::dvec3 normal2(plane2[0], plane2[1], plane2[2]);
    normal1 /= glm::length(normal1);
    normal2 /= glm::length(normal2);
    if (glm::dot(seedPoint2 - seedPoint1, normal1) < 0)
        normal1 = -normal1;
    if (glm::dot(seedPoint1 - seedPoint2, normal2) < 0)
        normal2 = -normal2;
    glm::dvec3 pseudoCenter = OctreeRayTracing::findPseudoCenter(seedPoint1, seedPoint2, normal1, normal2);
    glm::dvec3 customSeedPoint;
    double customRadius = 0.5*(glm::length(pseudoCenter - seedPoint1) + glm::length(pseudoCenter - seedPoint2));
    glm::dvec3 customNormal = glm::cross(normal1, glm::cross(normal1, normal2));
    customNormal /= glm::length(customNormal);
    int numberOfTries(5);
    std::vector<std::vector<glm::dvec3>> neighborList1, neighborList2;
    std::vector<glm::dvec3> temp;
    neighborList1.push_back(temp);
    neighborList2.push_back(temp);
    findNeighborsBuckets(seedPoint1, radius, neighborList1, numberOfBuckets, clippingAssembly);
    findNeighborsBuckets(seedPoint2, radius, neighborList2, numberOfBuckets, clippingAssembly);
    tags.push_back(pseudoCenter);
    for (int i = 1; i < (numberOfTries - 1); i++)
    {
        std::vector<std::vector<glm::dvec3>> neighborList3;
        neighborList3.push_back(temp);

        double angle = 2 * M_PI * i / (double)numberOfTries;
        customSeedPoint = pseudoCenter + glm::length(seedPoint1 - pseudoCenter)*(cos(angle)*normal1 + sin(angle)*customNormal);
        tags.push_back(customSeedPoint);
        findNeighborsBuckets(customSeedPoint, radius, neighborList3, 1, clippingAssembly);
        if (neighborList3[0].size() < minNumberOfPoints)
            continue;
        std::vector<std::vector<glm::dvec3>> finalPoints;
        finalPoints.push_back(temp);
        //cylinderFit with 3 seedPoints//
        if ((neighborList1[0].size() == 0) || (neighborList2[0].size() == 0) || (neighborList3[0].size() == 0))
            return false;
        for (int loop = 0; loop < maxNumberOfPoints; loop++)
        {
            std::vector<glm::dvec3> otherTemp;
            int v1 = rand() % 10000;
            int v2 = rand() % (1 + neighborList1[0].size() / (int)1000);
            int index = (v1 + 10000 * v2) % neighborList1[0].size();

            otherTemp.push_back(neighborList1[0][index]);
            finalPoints[0].insert(finalPoints[0].begin(), otherTemp.begin(), otherTemp.end());
        }
        for (int loop = 0; loop < maxNumberOfPoints; loop++)
        {
            std::vector<glm::dvec3> otherTemp;
            int v1 = rand() % 10000;
            int v2 = rand() % (1 + neighborList2[0].size() / (int)1000);
            int index = (v1 + 10000 * v2) % neighborList2[0].size();

            otherTemp.push_back(neighborList2[0][index]);
            finalPoints[0].insert(finalPoints[0].begin(), otherTemp.begin(), otherTemp.end());
        }
        for (int loop = 0; loop < maxNumberOfPoints; loop++)
        {
            std::vector<glm::dvec3> otherTemp;
            int v1 = rand() % 10000;
            int v2 = rand() % (1 + neighborList3[0].size() / (int)1000);
            int index = (v1 + 10000 * v2) % neighborList3[0].size();

            otherTemp.push_back(neighborList3[0][index]);
            finalPoints[0].insert(finalPoints[0].begin(), otherTemp.begin(), otherTemp.end());
        }

        bool success = OctreeRayTracing::beginCylinderFit(finalPoints, 0.01, cylinderRadius, cylinderDirection, cylinderCenter, 3 * maxNumberOfPoints);
        if (success)
            return true;
    }
    return false;
}

bool TlScanOverseer::beamBending(const std::vector<glm::dvec3>& globalEndPoints, glm::dvec3& bendPoint, double& maxBend, double& ratio, bool& reliable, const ClippingAssembly& clippingAssembly)
{
    srand((unsigned int)time(NULL));
    reliable = false;
    bool result = false;
    glm::dvec3 tempPoint(0.0, 0.0, 0.0);
    double tempBend(0);
    int numberOfSteps(50);
    glm::dvec3 normalVector(0.0, 0.0, -1.0);											// initialize normalVector, direction in which the bend is to be computed
    OctreeRayTracing::updateNormalVector(normalVector, globalEndPoints[0], globalEndPoints[1]);
    std::vector<double> plane1(4), plane2(4);
    bool test1 = fitPlaneRegionGrowing(globalEndPoints[0], plane1, clippingAssembly);
    bool test2 = fitPlaneRegionGrowing(globalEndPoints[1], plane2, clippingAssembly);
    std::vector<glm::dvec3> ball1, ball2;
    bool test3 = findNeighbors(globalEndPoints[0], 0.02, ball1, clippingAssembly);
    bool test4 = findNeighbors(globalEndPoints[1], 0.02, ball2, clippingAssembly);
    std::vector<glm::dvec3> otherEnds1, otherEnds2;
    int counter(0), safeCounter(0);
    bool globalTest = (test1&&test2&&test3&&test4);
    while ((counter < 2)&&(safeCounter<1000))
    {
        int v1 = rand() % 10000;
        if ((int)ball1.size() > 0)
        {
            int index = v1 % ball1.size();
            glm::dvec3 tempPoint = ball1[index];
            if (glm::dot(tempPoint - globalEndPoints[0], globalEndPoints[1] - globalEndPoints[0]) && (OctreeRayTracing::pointToPlaneDistance(tempPoint, plane1) < 0.002))
            {
                otherEnds1.push_back(tempPoint);
                counter++;
            }
            else
                safeCounter++;
        }
        else {
            counter = 2;
            globalTest = false;
        }
    }
    if (safeCounter > 1000)
        globalTest = false;
    counter = 0;
    safeCounter = 0;
    while ((counter < 2) && (safeCounter < 1000))
    {
        int v1 = rand() % 10000;
        if ((int)ball2.size() > 0)
        {
            int index = v1 % ball2.size();
            glm::dvec3 tempPoint = ball2[index];
            if (glm::dot(tempPoint - globalEndPoints[1], globalEndPoints[0] - globalEndPoints[1]) && (OctreeRayTracing::pointToPlaneDistance(tempPoint, plane2) < 0.003))
            {
                otherEnds2.push_back(tempPoint);
                counter++;
            }
            else
                safeCounter++;
        }
        else {
            counter = 2;
            globalTest = false;
        }
    }
    if (safeCounter > 1000)
        globalTest = false;
    std::vector<glm::dvec3> discretePoints, discretePoints1, discretePoints2, neighbors, neighbors1, neighbors2;

    discretize(globalEndPoints[0], globalEndPoints[1], numberOfSteps, discretePoints);
    if (((int)otherEnds1.size() < 2) || ((int)otherEnds2.size() < 2))
        globalTest = false;
    

    for (int i = 0; i < numberOfSteps; i++)
    {
        glm::dvec3 temp;
        if (nearestBendNeighbor(discretePoints[i], temp, normalVector))
        {
            neighbors.push_back(temp);
        }
        else
        {
            neighbors.push_back(discretePoints[i]);
        }
    }
    if (!globalTest)
        return OctreeRayTracing::findBiggestBend(neighbors, discretePoints, normalVector, bendPoint, maxBend, ratio);
    discretize(otherEnds1[0], otherEnds2[0], numberOfSteps, discretePoints1);
    discretize(otherEnds1[1], otherEnds2[1], numberOfSteps, discretePoints2);
    if(globalTest)
    {
        for (int i = 0; i < numberOfSteps; i++)
        {
            glm::dvec3 temp;
            if (nearestBendNeighbor(discretePoints1[i], temp, normalVector))
            {
                neighbors1.push_back(temp);
            }
            else
            {
                neighbors1.push_back(discretePoints1[i]);
            }
        }
        for (int i = 0; i < numberOfSteps; i++)
        {
            glm::dvec3 temp;
            if (nearestBendNeighbor(discretePoints2[i], temp, normalVector))
            {
                neighbors2.push_back(temp);
            }
            else
            {
                neighbors2.push_back(discretePoints2[i]);
            }
        }
        glm::dvec3 bendPoint1, bendPoint2;
        double tempMaxBend1, tempMaxBend2, ratio1, ratio2;
        bool otherTest0, otherTest1, otherTest2;
        otherTest0 = OctreeRayTracing::findBiggestBend(neighbors, discretePoints, normalVector, bendPoint, maxBend, ratio);
        otherTest1 = OctreeRayTracing::findBiggestBend(neighbors1, discretePoints1, normalVector, bendPoint1, tempMaxBend1, ratio1);
        otherTest2 = OctreeRayTracing::findBiggestBend(neighbors2, discretePoints2, normalVector, bendPoint2, tempMaxBend2, ratio2);
        if (otherTest0&&otherTest1&&otherTest2)
        {
            double maxVar(glm::length(bendPoint - bendPoint1));
            if (glm::length(bendPoint - bendPoint2) > maxVar)
                maxVar = glm::length(bendPoint - bendPoint2);
            if (glm::length(bendPoint1 - bendPoint2) > maxVar)
                maxVar = glm::length(bendPoint1 - bendPoint2);
            if (maxVar < 0.2*glm::length(globalEndPoints[0] - globalEndPoints[1]))
                reliable = true;
        }
        return otherTest0;
    }
    return false;
}

bool TlScanOverseer::discretize(const glm::dvec3& tip1, const glm::dvec3& tip2, const int& numberOfSteps, std::vector<glm::dvec3>& discretePoints)

{
    std::vector<glm::dvec3> result;
    glm::dvec3 ray(3), dummy(3);
    ray = tip2 - tip1;																// ray holds the line direction from tip1 to tip2
                                                                                    // dummy is just a placeholder used to increment the size of result
    //ray = scaleVector(ray, 1 / mynorm(ray));

    for (int i = 0; i < numberOfSteps; i++)
    {
        result.push_back(dummy);
        result[i][0] = tip1[0] + i * ray[0] / (numberOfSteps - 1);
        result[i][1] = tip1[1] + i * ray[1] / (numberOfSteps - 1);
        result[i][2] = tip1[2] + i * ray[2] / (numberOfSteps - 1);
    }
    discretePoints = result;

    return true;
}

bool TlScanOverseer::nearestBendNeighbor(glm::dvec3 globalPoint, glm::dvec3& result, glm::dvec3 normalVector)
{
    bool success(false);
    result = glm::dvec3(DBL_MAX, DBL_MAX, DBL_MAX);

    for (const WorkingScanInfo& _pair : s_workingScansTransfo)
    {
        _pair.scan.setComputeTransfo(_pair.transfo.getCenter(), _pair.transfo.getOrientation());
        glm::dvec3 currNeighbor;
        if (_pair.scan.nearestBendNeighbor(globalPoint, currNeighbor, normalVector))
        {
            if ((glm::length(globalPoint - currNeighbor) < glm::length(globalPoint - result)))
            {
                result = currNeighbor;
                success = true;
            }
        }
    }
    
    return success;
}

bool TlScanOverseer::columnOffset(const glm::dvec3& camPosition, glm::dvec3& wallPoint1, const glm::dvec3& wallPoint2, const glm::dvec3& columnPoint1, const glm::dvec3& columnPoint2, double& offset, double& ratio)
{
    return OctreeRayTracing::columnOffset(camPosition, wallPoint1, wallPoint2, columnPoint1, columnPoint2, offset, ratio);
}

/*bool TlScanOverseer::pointToPlaneMeasure(const glm::dvec3& seedPoint, const glm::dvec3& planePoint, const ClippingAssembly& clippingAssembly)
{
    double radius(0.2), distance;
    std::vector<std::vector<glm::dvec3>> points;
    if (!findNeighborsBuckets(seedPoint, radius, points, 5, clippingAssembly))
        return false;
    return OctreeRayTracing::pointToPlaneMeasure(points, seedPoint, distance);
}*/

/*bool TlScanOverseer::computeBeamHeight(const glm::dvec3& seedPoint, double& beamHeight, const ClippingAssembly& clippingAssembly)
{
    Logger::log(LoggerMode::rayTracingLog) << "STARTING BEAMHEIGHT... " << Logger::endl;

    double radius(0.03);
    std::vector<glm::dvec3> points;
    if (!findNeighbors(seedPoint, radius, points, clippingAssembly))		//points=smallBall
        return false;
    Logger::log(LoggerMode::rayTracingLog) << "1" << Logger::endl;
    Logger::log(LoggerMode::rayTracingLog) << "points : " << points.size() << Logger::endl;

    std::vector<double> plane;
    if (!OctreeRayTracing::fitPlane(points, plane))
        return false;
    Logger::log(LoggerMode::rayTracingLog) << "plane : " << plane[0] << " " << plane[1] << " " << plane[2] << " " << plane[3] << Logger::endl;
    double MSE = OctreeRayTracing::computeMeanSquaredDistanceToPlane(points, plane);
    Logger::log(LoggerMode::rayTracingLog) << "MSE : " << MSE << Logger::endl;

    radius = 0.15;
    ballCloseToPlane(seedPoint, plane, radius, points, 0.015, clippingAssembly);	//points=planeBall
    Logger::log(LoggerMode::rayTracingLog) << "points : " << points.size() << Logger::endl;

    Logger::log(LoggerMode::rayTracingLog) << "2" << Logger::endl;

    if (!OctreeRayTracing::fitPlane(points, plane))
        return false;
    Logger::log(LoggerMode::rayTracingLog) << "plane : " << plane[0] << " " << plane[1] << " " << plane[2] << " " << plane[3] << Logger::endl;
    MSE = OctreeRayTracing::computeMeanSquaredDistanceToPlane(points, plane);
    Logger::log(LoggerMode::rayTracingLog) << "MSE : " << MSE << Logger::endl;

    glm::dvec3 beamDirection, normalVector;
    normalVector = -glm::dvec3(plane[0], plane[1], plane[2]);
    normalVector /= glm::length(normalVector);
    std::vector<std::vector<double>> xyRange;
    int numberOfDirections(50);
    beamDirection = OctreeRayTracing::findBeamDirection(points, numberOfDirections, xyRange, seedPoint, normalVector);
    Logger::log(LoggerMode::rayTracingLog) << "beamDirection : " << beamDirection[0] << " " << beamDirection[1] << " " << beamDirection[2] << Logger::endl;

    Logger::log(LoggerMode::rayTracingLog) << "3" << Logger::endl;

    glm::dvec3 orthoDirection = glm::cross(beamDirection, normalVector);
    points = ballInBox(seedPoint, beamDirection, orthoDirection, normalVector, xyRange);		//points=boxBall
    Logger::log(LoggerMode::rayTracingLog) << "points : " << points.size() << Logger::endl;

    Logger::log(LoggerMode::rayTracingLog) << "4" << Logger::endl;

    double heightMax(0.5);
    beamHeight = OctreeRayTracing::findBeamHeight(points, seedPoint, normalVector, heightMax);
    Logger::log(LoggerMode::rayTracingLog) << "DONE : height = " << beamHeight << Logger::endl;

    return true;
}*/

void TlScanOverseer::ballCloseToPlane(const glm::dvec3& seedPoint, const std::vector<double>& plane, const double& radius, std::vector<glm::dvec3>& result, const double& distanceThreshold, const ClippingAssembly& clippingAssembly)
{
    result = std::vector<glm::dvec3>(0);
    std::vector<glm::dvec3> ball;
    if (!findNeighbors(seedPoint, radius, ball, clippingAssembly))
        return;
    for (int i = 0; i < (int)ball.size(); i++)
    {
        double distance = OctreeRayTracing::pointToPlaneDistance(ball[i], plane);
        if (distance < distanceThreshold)
            result.push_back(ball[i]);
    }
    return;
}

std::vector<glm::dvec3> TlScanOverseer::ballInBox(const glm::dvec3 seedPoint, const glm::dvec3& beamDirection, const glm::dvec3& orthoDir, const glm::dvec3& normalDir, const std::vector<std::vector<double>>& xyRange, const double& heightMax)
{
    std::vector<glm::dvec3> result(0);

    for (const WorkingScanInfo& _pair : s_workingScansTransfo)
    {
        _pair.scan.setComputeTransfo(_pair.transfo.getCenter(), _pair.transfo.getOrientation());
        std::vector<glm::dvec3> tempResult = _pair.scan.getPointsInBox(seedPoint, beamDirection, orthoDir, normalDir, xyRange, heightMax);
        for (int i = 0; i < tempResult.size(); i++)
            result.push_back(tempResult[i]);
    }
    return result;
}

std::vector<glm::dvec3> TlScanOverseer::pointsInBox(const GeometricBox& box, const ClippingAssembly& clippingAssembly)
{
    std::vector<glm::dvec3> result(0);

    for (const WorkingScanInfo& _pair : s_workingScansTransfo)
    {
        _pair.scan.setComputeTransfo(_pair.transfo.getCenter(), _pair.transfo.getOrientation());
        std::vector<glm::dvec3> tempResult = _pair.scan.getPointsInGeometricBox(box,clippingAssembly);
        for (int i = 0; i < tempResult.size(); i++)
            result.push_back(tempResult[i]);
    }
    return result;
    
}

void TlScanOverseer::estimateNormals()
{
    double threshold(0.01);
    std::vector<uint32_t> nonPlanarLeafIds;

    for (const WorkingScanInfo& _pair : s_workingScansTransfo)
    {
        _pair.scan.setComputeTransfo(_pair.transfo.getCenter(), _pair.transfo.getOrientation());
        // TODO(robin) - Refactor the 'OctreeNormal' constructor
        //OctreeNormal*  octree_ = new OctreeNormal(*scan);
        //octree_->fillSkeleton();
        //octree_->computeCoherentNeighborhoods(threshold, nonPlanarLeafIds);
    }
}

bool TlScanOverseer::fitPlane(const glm::dvec3& seedPoint, std::vector<double>& result, const ClippingAssembly& clippingAssembly)
{
    std::vector<std::vector<glm::dvec3>> ball(5);
    if (!findNeighborsBuckets(seedPoint, 0.3, ball, 5, clippingAssembly))
        return false;
    if (!OctreeRayTracing::fitPlaneBuckets(ball, result))
        return false;
    return true;
}

bool TlScanOverseer::fitPlaneRadius(const glm::dvec3& seedPoint, std::vector<double>& result, const ClippingAssembly& clippingAssembly, const double& radius)
{
    std::vector<std::vector<glm::dvec3>> ball(1);
    if (!findNeighborsBuckets(seedPoint, radius, ball, 1, clippingAssembly))
        return false;
    if (!OctreeRayTracing::fitPlaneBuckets(ball, result))
        return false;
    return true;
}

bool TlScanOverseer::fitPlane3Points(const std::vector<glm::dvec3> points, std::vector<double>& result)
{
    return OctreeRayTracing::fitPlane(points, result);
}
//////////////////////////////////////////
//---------- simple measures -----------//
//////////////////////////////////////////
bool TlScanOverseer::pointToCylinderMeasure(const glm::dvec3& point, const glm::dvec3 cylinderPoint, glm::dvec3& projectedPoint, glm::dvec3& projectedCylinderPoint, double& cylinderRadius, glm::dvec3& cylinderDirection, glm::dvec3& cylinderCenter, const ClippingAssembly& clippingAssembly)
{
    double radius(0.4);
    if (!fitCylinder(cylinderPoint, radius, 0.002, cylinderRadius, cylinderDirection, cylinderCenter, FitCylinderMode::robust, clippingAssembly))
        return false;
    projectedPoint = MeasureClass::projectPointToLine(point, cylinderDirection, cylinderCenter);
    projectedCylinderPoint = MeasureClass::projectPointToLine(cylinderPoint, cylinderDirection, cylinderCenter);
    
    return true;
}

/*
bool TlScanOverseer::pointToPlaneMeasure(const glm::dvec3& point, const glm::dvec3& planePoint, glm::dvec3& projectedPoint, glm::dvec3& normalVector, const ClippingAssembly& clippingAssembly)
{
    std::vector<double> plane;
    if (!fitPlaneRegionGrowing(planePoint, plane, clippingAssembly))
        return false;
    normalVector = glm::dvec3(plane[0], plane[1], plane[2]);
    normalVector /= glm::length(normalVector);
    projectedPoint = MeasureClass::projectPointToPlane(point, plane);
    return true;
}
*/

/*
bool TlScanOverseer::cylinderToPlaneMeasure(const glm::dvec3& cylinderPoint, const glm::dvec3& planePoint, glm::dvec3& cylinderAxisPoint, glm::dvec3& projectedPlanePoint, glm::dvec3& cylinderDirection, double& cylinderRadius, glm::dvec3& normalVector, bool& cylinderFit)
{

    glm::dvec3 cylinderCenter;
    if (!fitCylinder(cylinderPoint, 0.4, cylinderRadius, cylinderDirection, cylinderCenter))
    {
        cylinderFit = false;
        return false;
    }
    cylinderFit = true;
    cylinderAxisPoint = MeasureClass::projectPointToLine(cylinderPoint, cylinderDirection, cylinderCenter);
    std::vector<double> plane;
    if (!fitPlane(planePoint, plane))
        return false;
    normalVector = glm::dvec3(plane[0], plane[1], plane[2]);
    normalVector /= glm::length(normalVector);
    projectedPlanePoint = MeasureClass::projectPointToPlane(cylinderAxisPoint, plane);
    return true;
}

bool TlScanOverseer::cylinderToCylinderMeasure(const glm::dvec3& point1, const glm::dvec3& point2, glm::dvec3& cylinder1AxisPoint, glm::dvec3& cylinder2AxisPoint, glm::dvec3& cylinder1Direction, glm::dvec3 cylinder2Direction, double& cylinder1Radius, double& cylinder2Radius)
{
    glm::dvec3 cylinderCenter1, cylinderCenter2;
    if (!fitCylinder(point1, 0.4, cylinder1Radius, cylinder1Direction, cylinderCenter1))
        return false;
    if (!fitCylinder(point2, 0.4, cylinder2Radius, cylinder2Direction, cylinderCenter2))
        return false;
    cylinder1AxisPoint = MeasureClass::projectPointToLine(point1, cylinder1Direction, cylinderCenter1);
    cylinder2AxisPoint = MeasureClass::projectPointToLine(cylinder1AxisPoint, cylinder2Direction, cylinderCenter2);
    return true;
}
*/

bool TlScanOverseer::fitPlaneRegionGrowing(const glm::dvec3& seedPoint, std::vector<double>& result, const ClippingAssembly& clippingAssembly)
{
    srand((unsigned int)time(NULL));
    std::vector<double> tempResult;
    std::vector<glm::dvec3> smallBall;
    findNeighbors(seedPoint, 0.03, smallBall, clippingAssembly);
    std::vector<double> plane;
    if (!OctreeRayTracing::fitPlane(smallBall, plane))
        return false;
    tempResult = plane;
    int maxIterations(1000);
    if (isPlaneAcceptable(seedPoint, smallBall, 0.03, plane, 0.002))
    {
        result = plane;
        findNeighbors(seedPoint, 0.1, smallBall, clippingAssembly);
        OctreeRayTracing::fitPlane(smallBall, plane);
        if (isPlaneAcceptable(seedPoint, smallBall, 0.1, plane, 0.002))
        {
            //Logger::log(LoggerMode::rayTracingLog) << "extended first pass" << Logger::endl;

            result = plane;
            return true;
        }
        //Logger::log(LoggerMode::rayTracingLog) << "normal first pass" << Logger::endl;

        return true;
    }
    //else, RANSAC mode//f
    if (smallBall.size() > 100)
    {
        for (int i = 0; i < maxIterations; i++)
        {
            //select 2 random points, along with seedPoint
            std::vector<glm::dvec3> testPoints(1, seedPoint);
            if (smallBall.size() > 1000)
            {
                int v1 = rand() % 10000;
                int v2 = rand() % (smallBall.size() / (int)1000);
                int index = (v1 + 10000 * v2) % smallBall.size();			//if smallBall.size is large, one random index is not enough
                testPoints.push_back(smallBall[index]);
                v1 = rand() % 10000;
                v2 = rand() % (smallBall.size() / (int)1000);
                index = (v1 + 10000 * v2) % smallBall.size();
                testPoints.push_back(smallBall[index]);
            }
            else {
                int v1 = rand() % ((int)smallBall.size());
                testPoints.push_back(smallBall[v1]);						//if smallBall.size is small, we use only one random index
                v1 = rand() % ((int)smallBall.size());
                testPoints.push_back(smallBall[v1]);
            }
            if (OctreeRayTracing::fitPlane(testPoints, plane))
            {
                if (OctreeRayTracing::countPointsNearPlane(smallBall, plane, 0.003)*1.8 > smallBall.size())
                {
                    std::vector<glm::dvec3> planePoints = OctreeRayTracing::listPointsNearPlane(smallBall, plane, 0.003);
                    OctreeRayTracing::fitPlane(planePoints, plane);
                    if (isPlaneAcceptable(seedPoint, planePoints, 0.03, plane, 0.003))
                    {
                        result = plane;
                        findNeighbors(seedPoint, 0.1, smallBall, clippingAssembly);
                        planePoints = OctreeRayTracing::listPointsNearPlane(smallBall, plane, 0.003);

                        OctreeRayTracing::fitPlane(planePoints, plane);
                        if ((isPlaneAcceptable(seedPoint, planePoints, 0.1, plane, 0.002)) && (planePoints.size() * 2 > smallBall.size()))
                        {
                            //Logger::log(LoggerMode::rayTracingLog) << "extended pass, iteration : " << i << Logger::endl;

                            result = plane;
                            return true;
                        }
                        //Logger::log(LoggerMode::rayTracingLog) << "normal pass, iteration : " << i << Logger::endl;

                        return true;
                    }
                }
                result = tempResult;
                //Logger::log(LoggerMode::rayTracingLog) << "last resort" << Logger::endl;

                return true;
            }
        }
    }
    return false;
}

bool TlScanOverseer::fitPlaneTripleSeeds(const glm::dvec3& mainSeed, const glm::dvec3& endSeed1, const glm::dvec3& endSeed2, std::vector<double>& result, const ClippingAssembly& clippingAssembly)
{
    std::vector<double> tempResult;
    std::vector<glm::dvec3> smallBall;
    findNeighbors(mainSeed, 0.03, smallBall, clippingAssembly);
    findNeighborsTowardsPoint(endSeed1, endSeed2, 0.03, smallBall, clippingAssembly);
    findNeighborsTowardsPoint(endSeed2, endSeed1, 0.03, smallBall, clippingAssembly);
    if (fitPlaneRegionGrowing(mainSeed, tempResult, clippingAssembly))
    {
        double distance1, distance2;
        distance1 = OctreeRayTracing::pointToPlaneDistance(endSeed1, tempResult);
        distance2 = OctreeRayTracing::pointToPlaneDistance(endSeed2, tempResult);
        if (std::max(distance1, distance2) < 0.005)
        {
            result = tempResult;
            return true;
        }
        result = tempResult;
        return true;
    }
    tempResult = std::vector<double>(4, 0.0);
    result = tempResult;
    return false;
    //return OctreeRayTracing::fitPlane(smallBall, result);
}

bool TlScanOverseer::isPlaneAcceptable(const glm::dvec3& seedPoint, const std::vector<glm::dvec3>& points, const double& pointsRadius, const std::vector<double>& plane, const double& threshold)
{
    int pointsInPlane = OctreeRayTracing::countPointsNearPlane(points, plane, 0.003);
    glm::dvec3 centerOfMass = OctreeRayTracing::computeCenterOfMass(points);
    double distance = glm::length(seedPoint - MeasureClass::projectPointToPlane(seedPoint, plane));

    //plane is acceptable iff all of these conditions hold :
    //more than 80% of points are closer than 0.003 from the plane
    //center of mass is closer than 1/4 of the radius search from the seed point
    //seed point is closer than 0.003 from the plane
    return ((pointsInPlane / (double)points.size() > 0.8) && (glm::length(centerOfMass - seedPoint) < (0.25*pointsRadius)) && (distance < 0.003));
}

bool TlScanOverseer::beginMultipleCylinders(std::vector<glm::dvec3>& cylinderCenters, std::vector<glm::dvec3>& cylinderDirections, std::vector<double>& cylinderRadii, const ClippingAssembly& clippingAssembly, std::vector<glm::dvec3>& seedPoints)
{
    int numberOfPoints(150), skip1(0), skip2(0);
    seedPoints = samplePoints(numberOfPoints, clippingAssembly);
    std::vector<glm::dvec3> realSeeds;
    int consecutiveFails(0), maxFails(40);
    /*for (int i = 0; i < (int)seedPoints.size(); i++)
    {
        cylinderCenters.push_back(seedPoints[i]);
        cylinderDirections.push_back(seedPoints[i]);
        cylinderRadii.push_back(1);
    }*/
    for (int i = 0; i < (int)seedPoints.size(); i++)
    {
        if (consecutiveFails > maxFails)
        {
            Logger::log(LoggerMode::rayTracingLog) << "stopped because consecutive fails, total tries : " << i << " / " << seedPoints.size() << Logger::endl;

            seedPoints = realSeeds;

            Logger::log(LoggerMode::rayTracingLog) << "SKIP1 = " << skip1 << Logger::endl;
            Logger::log(LoggerMode::rayTracingLog) << "SKIP2 = " << skip2 << Logger::endl;
            return true;
        }
        if (!clippingAssembly.testPoint(glm::dvec4(seedPoints[i], 1.0)))
            continue;
        if (OctreeRayTracing::isPointCloseToPreviousCylinders(seedPoints[i], cylinderCenters, cylinderDirections, cylinderRadii, 0.01))
        {
            skip1++;
            consecutiveFails++;
            continue;
        }
        glm::dvec3 cCenter, cDirection;
        double cRadius;
        if (fitCylinder(seedPoints[i], 0.4, 0.001, cRadius, cDirection, cCenter, FitCylinderMode::multiple, clippingAssembly))
        {
            if ((cRadius > 1) || (cRadius < 0.02))
            {
                consecutiveFails++;
                continue;
            }
            if (!isCylinderCloseToPreviousCylinders(cylinderCenters, cylinderDirections, cylinderRadii, cCenter, cDirection, cRadius))
            {
                cylinderCenters.push_back(cCenter);
                cylinderDirections.push_back(cDirection);
                cylinderRadii.push_back(cRadius);
                realSeeds.push_back(seedPoints[i]);
            }
            else
            {
                skip2++;
                consecutiveFails++;
            }
        }
    }
    Logger::log(LoggerMode::rayTracingLog) << "SKIP1 = " << skip1 << Logger::endl;
    Logger::log(LoggerMode::rayTracingLog) << "SKIP2 = " << skip2 << Logger::endl;


    seedPoints = realSeeds;
    return true;
}

bool TlScanOverseer::extendCylinder(const glm::dvec3& seedPoint, const double& radius, const double& threshold, double& cylinderRadius, glm::dvec3& cylinderDirection, glm::dvec3& cylinderCenter, std::vector<double>& heights, const FitCylinderMode& mode, const ClippingAssembly& clippingAssembly)
{
    std::vector<std::vector<glm::dvec3>> neighborList, secondList;
    std::vector<glm::dvec3> temp;
    //glm::dvec3 cCenter, cDirection;
    double searchRadius(radius), heightThreshold(0.005), heightStep(0.2);
    bool doubleCheck(false);
    int numberOfBuckets, totalPoints(30000), extendTestPoints(50);
    numberOfBuckets = 5;
    searchRadius = 0.4;
    switch (mode)
    {
    case::FitCylinderMode::fast:
    {
        numberOfBuckets = 2;
        searchRadius = 0.2;
        break;
    }
    case::FitCylinderMode::robust:
    {
        numberOfBuckets = 5;
        break;
    }
    }

    for (int i = 0; i < numberOfBuckets; i++)
    {
        neighborList.push_back(temp);
        secondList.push_back(temp);
    }
    findNeighborsBuckets(seedPoint, searchRadius, neighborList, numberOfBuckets, clippingAssembly);
    if (!OctreeRayTracing::beginCylinderFit(neighborList, threshold, cylinderRadius, cylinderDirection, cylinderCenter, totalPoints))
        return false;
    heights = computeCylinderHeight2(seedPoint, cylinderRadius, cylinderCenter, cylinderDirection, extendTestPoints, heightThreshold, heightStep, clippingAssembly);
    Logger::log(LoggerMode::rayTracingLog) << "heightMin : " << heights[0] << Logger::endl;
    Logger::log(LoggerMode::rayTracingLog) << "heightMax : " << heights[1] << Logger::endl;
    glm::dvec3 finalDirection = refineDirection(cylinderRadius, cylinderCenter, cylinderDirection,heights,threshold, clippingAssembly);

    cylinderCenter = cylinderCenter + 0.5*(heights[0] + heights[1])*cylinderDirection;
    cylinderDirection = finalDirection;
    return true;
}

glm::dvec3 TlScanOverseer::refineDirection(const double& cylinderRadius, const glm::dvec3& cylinderCenter, const glm::dvec3& cylinderDirection, const std::vector<double>& heights, const double& threshold, const ClippingAssembly& clippingAssembly)
{
    glm::dvec3 seed1, seed2;
    seed1 = cylinderCenter + cylinderDirection * heights[0]*0.95;
    seed2 = cylinderCenter + cylinderDirection * heights[1] * 0.95;
    glm::dvec3 result;
    double searchRadius = std::min(1.1 * cylinderRadius, abs(heights[1] - heights[0]));
    std::vector<glm::dvec3> neighbors1, neighbors2,temp;
    std::vector<std::vector<glm::dvec3>> goodNeighbors;
    goodNeighbors.push_back(temp);
    findNeighbors(seed1, searchRadius, neighbors1, clippingAssembly);
    findNeighbors(seed2, searchRadius, neighbors2, clippingAssembly);
    for (int i = 0; i < (int)neighbors1.size(); i++)
    {
        if (glm::dot(seed2 - seed1, neighbors1[i] - seed1) > 0)
            goodNeighbors[0].push_back(neighbors1[i]);
    }
    for (int i = 0; i < (int)neighbors2.size(); i++)
    {
        if (glm::dot(seed1 - seed2, neighbors2[i] - seed2) > 0)
            goodNeighbors[0].push_back(neighbors2[i]);
    }
    double tempRadius;
    glm::dvec3 tempCenter;
    if (!OctreeRayTracing::beginCylinderFit(goodNeighbors, threshold, tempRadius, result, tempCenter, 10000))
    {
        return cylinderDirection;
    }
    else
    {
        if(abs(glm::dot(result,cylinderDirection))>(0.998*glm::length(result)*glm::length(cylinderDirection)))
            return result;
        else return cylinderDirection;
        //return cylinderDirection;
    }
    

}
bool TlScanOverseer::testHeight(const double& radius, const glm::dvec3& center, const glm::dvec3& direction, const double& height, const int& numberOfTestPoints, const double& threshold)
{
    std::vector<glm::dvec3> sample = sampleHeightPoints(center, radius, direction, height, numberOfTestPoints);
    glm::dvec3 point, gridPoint;
    double testMin(DBL_MAX), testMax(0), distance;
    int goodPoints(0);

    for (int i = 0; i < sample.size(); i++)
    {
        point = sample[i];
        nearestNeighbor(point, gridPoint);
        distance = glm::length(point - gridPoint);
        if (distance > testMax) { testMax = distance; }
        if (distance < testMin) { testMin = distance; }
        if (distance < (threshold*(1 + abs(height) * M_PI / (double)90))) { goodPoints++; }
    }

    bool result;
    double ratio = ((double)goodPoints / (double)sample.size());
    if (ratio > 0.20) { result = true; }
    else { result = false; }

    return result;
}

bool TlScanOverseer::testHeight2(const double& radius, const glm::dvec3& center, const glm::dvec3& direction, const double& height, const int& numberOfTestPoints, const double& threshold, const std::vector<glm::dvec3>& realPoints, double& goodPointsRatio, const double& ratioThreshold)
{
    std::vector<glm::dvec3> sample = sampleHeightPoints(center, radius, direction, height, numberOfTestPoints);
    glm::dvec3 point,realPoint;
    double testMin(DBL_MAX), testMax(0), distance;
    int goodPoints(0);

    for (int i = 0; i < sample.size(); i++)
    {
        point = sample[i];
        double neighborDist(DBL_MAX);
        for (int j = 0; j < (int)realPoints.size(); j++)
        {
            double currDist = glm::length(point - realPoints[j]);
            if (currDist < neighborDist)
            {
                realPoint = realPoints[j];
                neighborDist = currDist;
            }
        }
        distance = glm::length(point - realPoint);
        /*if (distance > testMax) { testMax = distance; }
        if (distance < testMin) { testMin = distance; }*/
        //if height is large, higher threshold in case direction is slightly off
        if (distance < (1.2*threshold*(1 + abs(height) * M_PI / (double)90))) { goodPoints++; }
    }

    bool result;
    double ratio = ((double)goodPoints / (double)sample.size());
    if (ratioThreshold > 0.99)
    {
        if (ratio > 0.3*ratioThreshold) { result = true; goodPointsRatio = ratio; }
        else { result = false; }
    }
    else
    {
        if (ratio > 0.4*ratioThreshold) { result = true; }
        else { result = false; }
    }

    return result;
}
std::vector<double> TlScanOverseer::computeCylinderHeight2(const glm::dvec3& seedPoint, const double& radius, const glm::dvec3& center, const glm::dvec3& direction, const int& numberOfTestPoints, const double& threshold, const double& heightStep, const ClippingAssembly& clippingAssembly)
{

    double heightMin, heightMax, currentHeight, finerCurrentHeight, bigStep(0.6),heightThreshold(0.01),finerHeightStep(0.01),goodPointsRatio(1),currRatio;

    heightMin = computeHeight(center, direction, center);
    heightMax = heightMin;
    currentHeight = heightMin;
    finerCurrentHeight = currentHeight;
    bool test(true);
    bool tempTest(true);
    while (test)
    {
        std::vector<glm::dvec3> sampledPoints;
        samplePointsBetweenHeights(center, radius, direction, center, currentHeight, currentHeight + bigStep, heightThreshold, sampledPoints, clippingAssembly);
        currentHeight += bigStep;
        tempTest = true;
        while (tempTest)
        {
            if (!testHeight2(radius, center, direction, finerCurrentHeight, numberOfTestPoints, threshold, sampledPoints,currRatio,goodPointsRatio))
            {
                heightMax = finerCurrentHeight;
                tempTest = false;
                test = false;
            }
            else
            {
                if (goodPointsRatio > 0.99)
                {
                    goodPointsRatio = currRatio;
                }
                finerCurrentHeight += heightStep;
                if (finerCurrentHeight > (currentHeight - heightThreshold))
                {
                    tempTest = false;
                }
            }
        }
    }
    std::vector<glm::dvec3> sampledPoints1;
    samplePointsBetweenHeights(center, radius, direction, center, heightMax-heightStep, heightMax + heightStep, heightThreshold, sampledPoints1, clippingAssembly);
    tempTest = true;
    finerCurrentHeight = heightMax - heightStep;
    while (tempTest)
    {
        if ((!testHeight2(radius, center, direction, finerCurrentHeight, numberOfTestPoints, threshold, sampledPoints1,currRatio,goodPointsRatio)) || (finerCurrentHeight > heightMax + heightStep))
        {
            heightMax = finerCurrentHeight-finerHeightStep;
            tempTest = false;
        }
        else
            finerCurrentHeight += finerHeightStep;
    }
    test = true;
    tempTest = true;
    currentHeight = heightMin;
    finerCurrentHeight = currentHeight;
    goodPointsRatio = 1;
    while (test)
    {
        std::vector<glm::dvec3> sampledPoints;
        samplePointsBetweenHeights(center, radius, direction, center, currentHeight, currentHeight - bigStep, heightThreshold, sampledPoints, clippingAssembly);
        currentHeight -= bigStep;
        tempTest = true;
        while (tempTest)
        {
            if (!testHeight2(radius, center, direction, finerCurrentHeight, numberOfTestPoints, threshold, sampledPoints,currRatio,goodPointsRatio))
            {
                heightMin = finerCurrentHeight;
                tempTest = false;
                test = false;
            }
            else
            {
                finerCurrentHeight -= heightStep;
                if (goodPointsRatio > 0.99)
                    goodPointsRatio = currRatio;
                if (finerCurrentHeight < (currentHeight + heightThreshold))
                {
                    tempTest = false;
                }
            }
        }
    }
    std::vector<glm::dvec3> sampledPoints2;
    samplePointsBetweenHeights(center, radius, direction, center, heightMin - heightStep, heightMin + heightStep, heightThreshold, sampledPoints2, clippingAssembly);
    tempTest = true;
    finerCurrentHeight = heightMin + heightStep;
    while (tempTest)
    {
        if ((!testHeight2(radius, center, direction, finerCurrentHeight, numberOfTestPoints, threshold, sampledPoints2,currRatio,goodPointsRatio)) || (finerCurrentHeight < heightMin - heightStep))
        {
            heightMin = finerCurrentHeight + finerHeightStep;
            tempTest = false;
        }
        else
            finerCurrentHeight -= finerHeightStep;
    }
    std::vector<double> result;
    result.push_back(heightMin);
    result.push_back(heightMax);

    return result;
    /*while (test)																			//////////////////////problem somewhere there
    {
        currentHeight += heightStep;
        if (!tempTest)
            tempTest = testHeight(radius, center, direction, currentHeight, numberOfTestPoints, threshold);
        else
        {
            test = testHeight(radius, center, direction, currentHeight, numberOfTestPoints, threshold);
            tempTest = test;
        }
        if (test)
            heightMax = currentHeight;

    }
    test = true;
    tempTest = true;
    currentHeight = heightMin;
    while (test)
    {
        heightMin = currentHeight;
        currentHeight -= heightStep;
        if (!tempTest)
            tempTest = testHeight(radius, center, direction, currentHeight, numberOfTestPoints, threshold);
        else
        {
            test = testHeight(radius, center, direction, currentHeight, numberOfTestPoints, threshold);
            tempTest = test;
        }
        if (test)
            heightMin = currentHeight;
    }
    std::vector<double> result;
    result.push_back(heightMin);
    result.push_back(heightMax);

    return result;*/
}

bool TlScanOverseer::samplePointsBetweenHeights(const glm::dvec3& seedPoint, const double& radius, const glm::dvec3& direction, const glm::dvec3& center, const double& height1, const double& height2, const double& heightThreshold, std::vector<glm::dvec3>& sampledPoints, const ClippingAssembly& clippingAssembly)
{
    std::vector<glm::dvec3> testPoints;
    int pointsOnPerimeter(10);
    double minHeight = std::min(height1,height2)-heightThreshold;
    double maxHeight = std::max(height1, height2)+heightThreshold;
    double heightStep = 0.05;
    double currHeight = minHeight;
    while (currHeight < maxHeight)
    {
        glm::dvec3 v, w, radialDirection, point;
        std::vector<glm::dvec3> result;
        OctreeRayTracing::completeVectorToOrthonormalBasis(direction, v, w);
        double angle(0);
        for (int loop = 0; loop < pointsOnPerimeter; loop++)
        {
            angle = (double)loop*M_PI / (double)pointsOnPerimeter;
            radialDirection = cos(angle)*v + sin(angle)*w;
            radialDirection = radialDirection / glm::length(radialDirection);
            point = center + currHeight * direction + radius * radialDirection;

            testPoints.push_back(point);
            radialDirection = -radialDirection;
            point = center + currHeight * direction + radius * radialDirection;
            testPoints.push_back(point);
        }
        currHeight += heightStep;
    }

    for (const WorkingScanInfo& _pair : s_workingScansTransfo)
    {
        _pair.scan.setComputeTransfo(_pair.transfo.getCenter(), _pair.transfo.getOrientation());

        std::vector<uint32_t> leavesId;
        for (int i = 0; i < (int)testPoints.size(); i++)
        {
            uint32_t currCellId;
            if (_pair.scan.pointToCell(_pair.scan.getLocalCoord(testPoints[i]), currCellId))
            {
                // NOTE(robin) - On pourrait utiliser un unordered_set pour faire ce tri.
                bool tempTest = true;
                for (int j = 0; j < (int)leavesId.size(); j++)
                {
                    if (leavesId[j] == currCellId)
                    {
                        tempTest = false;
                        break;
                    }
                }
                if (tempTest)
                    leavesId.push_back(currCellId);
            }
        }
        std::vector<glm::dvec3> tempResult;
        //_pair.scan.getPointsInLeafList(leavesId, tempResult);
        //_pair.scan.samplePointsByStep(0.005f, leavesId, tempResult);
        _pair.scan.samplePointsByQuota(6000, leavesId, tempResult);
        for (int j = 0; j < (int)tempResult.size(); j++)
        {
            if ((clippingAssembly.testPoint(glm::dvec4(tempResult[j], 1.0)))||(!_pair.isClippable))
                sampledPoints.push_back(tempResult[j]);
            //if (OctreeRayTracing::isPointClipped(tempResult[j], clippingBoxes))
            //    sampledPoints.push_back(tempResult[j]);
        }
    }
    /*double testHeightMin(DBL_MAX), testHeightMax(-DBL_MAX);
    for (int i = 0; i < (int)sampledPoints.size(); i++)
    {
        double tempCurrHeight = computeHeight(sampledPoints[i], direction, center);
        if (tempCurrHeight > testHeightMax)
            testHeightMax = tempCurrHeight;
        if (tempCurrHeight < testHeightMin)
            testHeightMin = tempCurrHeight;
    }*/
    return true;
}
// static
std::vector<glm::dvec3> TlScanOverseer::sampleHeightPoints(const glm::dvec3& center, const double& radius, const glm::dvec3& direction, const double& height, const int& numberOfTestPoints)
{
    glm::dvec3 v, w, radialDirection, point;
    std::vector<glm::dvec3> result;
    OctreeRayTracing::completeVectorToOrthonormalBasis(direction, v, w);
    double angle(0);
    for (int loop = 0; loop < numberOfTestPoints; loop++)
    {
        angle = (double)loop*M_PI / (double)numberOfTestPoints;
        radialDirection = cos(angle)*v + sin(angle)*w;
        radialDirection = radialDirection / glm::length(radialDirection);
        point = center + height * direction + radius * radialDirection;

        result.push_back(point);
        radialDirection = -radialDirection;
        point = center + height * direction + radius * radialDirection;
        result.push_back(point);
    }

    return result;
}

// static
double TlScanOverseer::computeHeight(const glm::dvec3& point, const glm::dvec3& direction, const glm::dvec3& center)
{
    return glm::dot((point - center), (direction));
}

std::vector<glm::dvec3> TlScanOverseer::samplePoints(const int& numberOfPoints, const ClippingAssembly& clippingAssembly)
{
    srand((unsigned int)time(NULL));

    std::vector<glm::dvec3> result;
    int maxTry = 10 * numberOfPoints;
    int currentTry(0);

    if (clippingAssembly.clippingUnion.size() > 0)
    {
        while (((int)result.size() < numberOfPoints) && (currentTry < maxTry))
        {
            currentTry++;
            int boxIndex = rand() % ((int)clippingAssembly.clippingUnion.size());
            float r1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            float r2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            float r3 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            glm::dvec4 randomPoint(2 * r1 - 1, 2 * r2 - 1, 2 * r3 - 1, 1.0);

            // QUESTION(robin) - l'ancienne version des matrices fournies pour représenter des clipping contient le scale, --> je ne suis pas sûr de les 2 lignes soient équivalentes.
            //randomPoint = glm::inverse(insideBoxes[boxIndex]) * randomPoint;
            randomPoint = glm::inverse(clippingAssembly.clippingUnion[boxIndex]->matRT_inv) * randomPoint;
            std::vector<std::vector<glm::dvec3>> pointCandidates;
            int totalPoints(0);

            for (const WorkingScanInfo& _pair : s_workingScansTransfo)
            {
                _pair.scan.setComputeTransfo(_pair.transfo.getCenter(), _pair.transfo.getOrientation());

                pointCandidates.push_back(std::vector<glm::dvec3>());
                _pair.scan.getPointsInLeaf(randomPoint, pointCandidates.back());
            }

            for (int i = 0; i < (int)pointCandidates.size(); i++)
            {
                totalPoints += (int)pointCandidates[i].size();
            }
            if (totalPoints == 0)
                continue;
            int pointIndex = rand() % (totalPoints);
            Logger::log(LoggerMode::rayTracingLog) << "totalPoints : " << totalPoints << " ,pointIndex : " << pointIndex << Logger::endl;

            for (int i = 0; i < (int)pointCandidates.size(); i++)
            {
                if (pointIndex < (int)pointCandidates[i].size())
                {
                    glm::dvec3 neighbor = pointCandidates[i][pointIndex];
                    if (clippingAssembly.testPoint(glm::dvec4(neighbor, 1.0)))
                        result.push_back(neighbor);
                    break;
                }
                else
                {
                    totalPoints -= (int)pointCandidates[i].size();
                }
            }
        }
    }
    return result;
}

bool TlScanOverseer::isCylinderCloseToPreviousCylinders(const std::vector<glm::dvec3>& cylinderCenters, const std::vector<glm::dvec3>& cylinderDirections, const std::vector<double>& cylinderRadii, const glm::dvec3& cCenter, const glm::dvec3& cDirection, const double& cRadius)
{
    for (int i = 0; i < (int)cylinderCenters.size(); i++)
    {
        bool dir, cent, rad;
        dir = abs(glm::dot(cylinderDirections[i], cDirection)) > 0.9;
        rad = ((cylinderRadii[i] / cRadius) < 1.2) && ((cylinderRadii[i] / cRadius) > 0.8);
        cent = (glm::length(cylinderCenters[i] - cCenter) < 0.01) || (abs(abs(glm::dot(cylinderCenters[i] - cCenter, cDirection) / glm::length(cylinderCenters[i] - cCenter)) - 1) < 0.2);
        if (dir&&rad&&cent)
            return true;
    }
    return false;
}

bool TlScanOverseer::beamDetection(const glm::dvec3& seedPoint, const ClippingAssembly& clippingAssembly, glm::dvec3& normalVector, double& beamHeight, std::vector<std::vector<double>>& directionRange , const glm::dvec3& camPos, glm::dvec3& beamDirection, glm::dvec3& orthoDir)
{
    std::vector<glm::dvec3> initBall,planePoints,boxPoints;
    double initRadius(0.9),threshold(0.002),hMax=(0.8),planeRadius(0.2);
    int numberOfDirections(100);
    findNeighbors(seedPoint, initRadius, initBall, clippingAssembly);
    std::vector<double> seedPlane;
    if (!fitPlaneRegionGrowing(seedPoint, seedPlane, clippingAssembly))
        return false;
    normalVector = glm::dvec3(seedPlane[0], seedPlane[1], seedPlane[2]);
    normalVector /= glm::length(normalVector);
    if (glm::dot(normalVector, seedPoint - camPos) < 0)
        normalVector = -normalVector;
    for (int i = 0; i < (int)initBall.size(); i++)
    {
        if (OctreeRayTracing::pointToPlaneDistance(initBall[i], seedPlane) < threshold)
        {
            if ((glm::length(initBall[i] - seedPoint)) < planeRadius)
                planePoints.push_back(initBall[i]);
        }		
    }
    beamDirection = OctreeRayTracing::findBeamDirection(planePoints, numberOfDirections, directionRange, seedPoint, normalVector,orthoDir);

    for (int i = 0; i < (int)initBall.size(); i++)
    {
        glm::dvec3 localVec = initBall[i] - seedPoint;
        if ((glm::dot(localVec,normalVector)>0)&&
            (glm::dot(localVec,normalVector)<hMax)&&
            (glm::dot(localVec,beamDirection)>directionRange[0][0])&&
            (glm::dot(localVec,beamDirection)<directionRange[0][1])&&
            (glm::dot(localVec,orthoDir)>directionRange[1][0])&&
            (glm::dot(localVec,orthoDir)<directionRange[1][1]))
            boxPoints.push_back(initBall[i]);
    }

    beamHeight=OctreeRayTracing::findBeamHeight(boxPoints, seedPoint, normalVector, hMax,beamDirection,orthoDir,directionRange[1]);
    //beamHeight = computeClosestStandardBeam(beamHeight);
    //beamDirection = glm::cross(normalVector,beamDirection);
    //beamDirection /= glm::length(beamDirection);
    return true;
    
}

bool TlScanOverseer::beamDetectionManualExtend(const glm::dvec3& seedPoint, const glm::dvec3& endPoint1, const glm::dvec3& endPoint2, const ClippingAssembly& clippingAssembly, glm::dvec3& normalVector, double& beamHeight, std::vector<std::vector<double>>& directionRange, const glm::dvec3& camPos, glm::dvec3& beamDirection, glm::dvec3& orthoDir)
{
    std::vector<glm::dvec3> initBall, planePoints, boxPoints;
    double initRadius(0.9), threshold(0.002), hMax = (0.8), planeRadius(0.2);
    int numberOfDirections(100);
    findNeighbors(seedPoint, initRadius, initBall, clippingAssembly);
    findNeighborsTowardsPoint(endPoint1, endPoint2, initRadius/3, initBall, clippingAssembly);
    findNeighborsTowardsPoint(endPoint2, endPoint1, initRadius/3, initBall, clippingAssembly);

    std::vector<double> seedPlane;
    /*if (!fitPlaneRegionGrowing(seedPoint, seedPlane, clippingAssembly))
        return false;*/
    if (!fitPlaneTripleSeeds(seedPoint, endPoint1, endPoint2, seedPlane, clippingAssembly))
    {
        if (!fitPlaneRegionGrowing(seedPoint, seedPlane, clippingAssembly))
            return false;
    }
    normalVector = glm::dvec3(seedPlane[0], seedPlane[1], seedPlane[2]);
    normalVector /= glm::length(normalVector);
    if (glm::dot(normalVector, seedPoint - camPos) < 0)
        normalVector = -normalVector;
    for (int i = 0; i < (int)initBall.size(); i++)
    {
        if (OctreeRayTracing::pointToPlaneDistance(initBall[i], seedPlane) < threshold)
        {
            if ((glm::length(initBall[i] - seedPoint)) < planeRadius)
                planePoints.push_back(initBall[i]);
        }
    }
    beamDirection = OctreeRayTracing::findBeamDirection(planePoints, numberOfDirections, directionRange, seedPoint, normalVector, orthoDir);

    for (int i = 0; i < (int)initBall.size(); i++)
    {
        glm::dvec3 localVec = initBall[i] - seedPoint;
        if ((glm::dot(localVec, normalVector) > 0) &&
            (glm::dot(localVec, normalVector) < hMax) &&
            (glm::dot(localVec, beamDirection) > directionRange[0][0]) &&
            (glm::dot(localVec, beamDirection) < directionRange[0][1]) &&
            (glm::dot(localVec, orthoDir) > directionRange[1][0]) &&
            (glm::dot(localVec, orthoDir) < directionRange[1][1]))
            boxPoints.push_back(initBall[i]);
    }

    beamHeight = OctreeRayTracing::findBeamHeight(boxPoints, seedPoint, normalVector, hMax, beamDirection, orthoDir, directionRange[1]);
    //beamHeight = computeClosestStandardBeam(beamHeight);
    //beamDirection = glm::cross(normalVector,beamDirection);
    //beamDirection /= glm::length(beamDirection);
    return true;

}

bool TlScanOverseer::fitSphere(const std::vector<glm::dvec3>& seedPoints, glm::dvec3& center, double& radius, const double& threshold, const ClippingAssembly& clippingAssembly, glm::dvec3& centerOfMass)
{
    // algorithm from : https://www.geometrictools.com/Documentation/LeastSquaresFitting.pdf (outdated)

    //refine seedPoint
    /*std::vector<glm::dvec3> planePoints;
    findNeighbors(seedPoint, 0.01, planePoints, clippingAssembly);
    std::vector<double> plane(4);
    OctreeRayTracing::fitPlane(planePoints, plane);*/
    std::vector<std::vector<glm::dvec3>> points;
    double searchRadius = 0.2;
    int numberOfBuckets(5);
    std::vector<glm::dvec3> temp;

    for (int i = 0; i < numberOfBuckets; i++)
    {
        points.push_back(temp);
    }
    if ((int)seedPoints.size() == 1)
    {
        for (int i = 0; i < (int)seedPoints.size(); i++)
        {
            findNeighborsBuckets(seedPoints[i], searchRadius, points, numberOfBuckets, clippingAssembly);
        }
    }
    
    int maxNumberOfPoints(50000);
    std::vector<glm::dvec3> ballPoints;

    for (int tryOut = 0; tryOut < points.size(); tryOut++)
    {
        std::vector<glm::dvec3> totalPoints;
        double minCenterMove(1);
        if ((int)seedPoints.size() == 1)
        {
            OctreeRayTracing::samplePointsUpToBucket(points, (int)points.size() - tryOut, totalPoints);
        }
        else
        {
            totalPoints = seedPoints;
        }
        if (totalPoints.size() < 4)
            continue;
        if (totalPoints.size() > maxNumberOfPoints)
        {
            std::vector<glm::dvec3> otherTemp;
            for (int loop = 0; loop < maxNumberOfPoints; loop++)
            {
                int v1 = rand() % maxNumberOfPoints;
                int v2 = rand() % (totalPoints.size() / (int)maxNumberOfPoints);
                int index = (v1 + maxNumberOfPoints * v2) % totalPoints.size();

                otherTemp.push_back(totalPoints[index]);			//insert stuff to pick points at random instead
            }

            ballPoints = otherTemp;
        }
        else { ballPoints = totalPoints; }

        if ((int)ballPoints.size() < 4)
            continue;

        /*center = OctreeRayTracing::computeCenterOfMass(ballPoints);
        //glm::dvec3 centerOfMass = center;
        centerOfMass = center;
        center = glm::dvec3(221.7, 72.475, 2.027);
        glm::dvec3 previousCenter;
        for (int iteration = 0; iteration < maxIterations; iteration++)
        {
            previousCenter = center;
            double lengthAverage = 0;
            glm::dvec3 directionAverage = glm::dvec3(0, 0, 0);
            for (int i = 0; i < (int)ballPoints.size(); i++)
            {
                glm::dvec3 dir = center - ballPoints[i];
                double lengthDir = glm::length(dir);
                if (lengthDir > 0)
                {
                    lengthAverage += lengthDir;
                    directionAverage -= dir / lengthDir;
                }
            }
            lengthAverage = lengthAverage / (double)ballPoints.size();
            directionAverage = directionAverage / (double)ballPoints.size();
            center = centerOfMass + lengthAverage * directionAverage;
            radius = lengthAverage;
            if (glm::length(center - previousCenter) < minCenterMove)
                minCenterMove = glm::length(center - previousCenter);
            if (glm::length(center - previousCenter) < centerThreshold)
            {
                double meanError = 0;
                for (int i = 0; i < (int)ballPoints.size(); i++)
                {
                    meanError += abs(glm::length(center - ballPoints[i]) - radius);
                }
                meanError = meanError / (double)ballPoints.size();
                if (meanError < threshold)
                    return true;
                else break;
            }
        }
        double meanError = 0;
        for (int i = 0; i < (int)ballPoints.size(); i++)
        {
            meanError += abs(glm::length(center - ballPoints[i]) - radius);
        }
        meanError = meanError / (double)ballPoints.size();
        if (meanError < threshold)
            return true;
    }*/
    /*centerOfMass = OctreeRayTracing::computeCenterOfMass(ballPoints);
    //std::vector<glm::dvec3> centeredBallPoints = OctreeRayTracing::substractCenterOfMass(ballPoints, centerOfMass);
    glm::dvec3 centerGuess = glm::dvec3(221.7, 72.475, 2.027);
    std::vector<glm::dvec3> centeredBallPoints = OctreeRayTracing::substractCenterOfMass(ballPoints, centerGuess);
    glm::dvec3 tempCenterOfMass = OctreeRayTracing::computeCenterOfMass(centeredBallPoints);
    glm::dmat3 A(0);
    glm::dvec3 B(0);
    for (int i = 0; i < (int)ballPoints.size(); i++)
    {
        A[0, 0] += centeredBallPoints[i][0] * (centeredBallPoints[i][0]-tempCenterOfMass[0]);
        A[0, 1] += centeredBallPoints[i][0] * (centeredBallPoints[i][1] - tempCenterOfMass[1]);
        A[0, 2] += centeredBallPoints[i][0] * (centeredBallPoints[i][2] - tempCenterOfMass[2]);

        A[1, 0] += centeredBallPoints[i][1] * (centeredBallPoints[i][0] - tempCenterOfMass[0]);
        A[1, 1] += centeredBallPoints[i][1] * (centeredBallPoints[i][1] - tempCenterOfMass[1]);
        A[1, 2] += centeredBallPoints[i][1] * (centeredBallPoints[i][2] - tempCenterOfMass[2]);

        A[2, 0] += centeredBallPoints[i][2] * (centeredBallPoints[i][0] - tempCenterOfMass[0]);
        A[2, 1] += centeredBallPoints[i][2] * (centeredBallPoints[i][1] - tempCenterOfMass[1]);
        A[2, 2] += centeredBallPoints[i][2] * (centeredBallPoints[i][2] - tempCenterOfMass[2]);

        double L = glm::length(centeredBallPoints[i]);
        L *= L;
        B[0] += L * (centeredBallPoints[i][0] - tempCenterOfMass[0]);
        B[1] += L * (centeredBallPoints[i][1] - tempCenterOfMass[1]);
        B[2] += L * (centeredBallPoints[i][2] - tempCenterOfMass[2]);
    }
    A /= (double)ballPoints.size();
    A *= 2;
    B /= (double)ballPoints.size();
    center = glm::inverse((glm::transpose(A)*A))*glm::transpose(A)*B;
    center += centerGuess;
    radius = 0;
    for (int i = 0; i < (int)ballPoints.size(); i++)
    {
        double L = glm::length(ballPoints[i] - center);
        L *= L;
        radius += L;
    }
    radius /= (double)ballPoints.size();
    radius = sqrt(radius);
    double meanError = 0;
    for (int i = 0; i < (int)ballPoints.size(); i++)
    {
        meanError += abs(glm::length(center - ballPoints[i]) - radius);
    }
    meanError = meanError / (double)ballPoints.size();
    if (meanError < threshold)
        return true;
}
return false;*/

//algorithm from : https://arxiv.org/ftp/arxiv/papers/1506/1506.02776.pdf
        double Sx(0), Sy(0), Sz(0), Sxx(0), Sxy(0), Sxz(0), Syy(0), Syz(0), Szz(0), Sxxx(0), Sxxy(0), Sxxz(0), Sxyy(0),Sxzz(0), Syyy(0), Syyz(0), Syzz(0), Szzz(0);
        for (int i = 0; i < (int)ballPoints.size(); i++)
        {
            double x(ballPoints[i][0]), y(ballPoints[i][1]), z(ballPoints[i][2]);
            Sx += x;
            Sy += y;
            Sz += z;
            Sxx += x * x;
            Sxy += x * y;
            Sxz += x * z;
            Syy += y * y;
            Syz += y * z;
            Szz += z * z;
            Sxxx += x * x*x;
            Sxxy += x * x*y;
            Sxxz += x * x*z;
            Sxyy += x * y*y;
            Sxzz += x * z*z;
            Syyy += y * y*y;
            Syyz += y * y*z;
            Syzz += y * z*z;
            Szzz += z * z*z;
        }
        double N = (double)ballPoints.size();
        double A, a, b, c, d, e, f, g, h, j, k, l, m, delta;
        A = Sxx + Syy + Szz;
        a = 2 * Sx*Sx - 2 * N*Sxx;
        b = 2 * Sx*Sy - 2 * N*Sxy;
        c = 2 * Sx*Sz - 2 * N*Sxz;
        d = -N * (Sxxx + Sxyy + Sxzz) + A * Sx;
        e = 2 * Sx*Sy - 2 * N*Sxy;
        f = 2 * Sy*Sy - 2 * N*Syy;
        g = 2 * Sy*Sz - 2 * N*Syz;
        h = -N * (Sxxy + Syyy + Syzz) + A * Sy;
        j = 2 * Sx*Sz - 2 * N*Sxz;
        k = 2 * Sy*Sz - 2 * N*Syz;
        l = 2 * Sz*Sz - 2 * N*Szz;
        m = -N * (Sxxz + Syyz + Szzz) + A * Sz;
        delta = a * (f*l - g * k) - e * (b*l - c * k) + j * (b*g - c * f);

        double xc, yc, zc;
        xc = (d*(f*l - g * k) - h * (b*l - c * k) + m * (b*g - c * f)) / delta;
        yc = (a*(h*l - m * g) - e * (d*l - m * c) + j * (d*g - h * c)) / delta;
        zc = (a*(f*m - h * k) - e * (b*m - d * k) + j * (b*h - d * f)) / delta;
        center = glm::dvec3(xc, yc, zc);
        radius = sqrt(xc*xc + yc * yc + zc * zc + (A - 2 * (xc*Sx + yc * Sy + zc * Sz)) / N);
        double meanError = 0;
        for (int i = 0; i < (int)ballPoints.size(); i++)
        {
            meanError += abs(glm::length(center - ballPoints[i]) - radius);
        }
        meanError = meanError / (double)ballPoints.size();
        if (meanError < threshold)
            return true;
    }
    return false;
}

bool TlScanOverseer::applyConstraints(std::vector<glm::dvec3>& centers, std::vector<glm::dvec3>& directions, std::vector<double>& lengths, std::vector<double>& angleModifs, std::vector<glm::dvec3>& elbowPoints, std::vector<LineConnectionType>& connectionType, const double& mainRadius, const std::vector<double>& radii, std::vector<std::vector<glm::dvec3>>& elbowEdges, const bool& angleConstraints, std::vector<glm::dvec3>& elbowCenters)
{
    
    std::vector<glm::dvec3> initialDirections = directions;
    std::vector<glm::dvec3> growingCenters, growingDirections;
    std::vector<double> growingLengths, growingRadii;
    for (int i = 0; i < ((int)centers.size()-1); i++)
    {


        bool shouldContinue = false;
        growingCenters.push_back(centers[i]);
        growingDirections.push_back(directions[i]);
        growingLengths.push_back(lengths[i]);
        growingRadii.push_back(radii[i]);
        //look for a stitch connexion (new section is stitched to a main pipe previously processed)
        bool isStitch = lookForStitchConnexion(centers[i+1], directions[i+1], lengths[i+1], growingCenters, growingDirections, growingRadii, growingLengths,std::max(radii[i],radii[i+1]));
        double targetCosAngle = getTargetCosAngle(directions[i], directions[i + 1]);
        bool skipStitch = (targetCosAngle > 0.999);
        bool connectionTypeUpdated(false);
        //something that handles the case where an elbow is created between i and i+1, and i+1 is stitched onto i later (problem is : ith connection is not this one)
        //store for each elbow to which pipe it connects to (maybe not needed because elbow i is always between pipe i and i+1 ?)
        // when stitching, look up these to find all elbows between the stiched pipe and the main pipe (max 2 elbows)
        // 
        //look if this new section makes a previous connexion a stitch to this pipe
        std::vector<int> isSecondaryStitch = lookForSecondaryStitchConnexion(centers[i + 1], directions[i + 1], lengths[i + 1], centers, directions, radii, lengths, radii[i + 1]);

        for (int j = 0; j < (int)isSecondaryStitch.size(); j++)
        {
            /*if (skipStitch)
                break;*/
            int currIndex = isSecondaryStitch[j];
            //not necesseraly the right index, could be also/instead currIndex-1
            if (currIndex < (int)connectionType.size())
            {
                connectionType[currIndex] = LineConnectionType::stitching;
            }
            else if ((currIndex == (int)connectionType.size())&&(!isStitch)&&(!connectionTypeUpdated))
            {
                connectionType.push_back(LineConnectionType::stitching);
                connectionTypeUpdated = true;
            }
            //update cylinder parameters
            glm::dvec3 endPoint = centers[currIndex] + 0.5 * lengths[currIndex] * directions[currIndex];
            glm::dvec3 otherPoint= endPoint - directions[currIndex] * lengths[currIndex];
            //make endPoint be the closest to the othe axis
            if (pointToLineDistance(endPoint, directions[i+1], centers[i+1]) > pointToLineDistance(endPoint - directions[currIndex] * lengths[currIndex], directions[i+1], centers[i+1]))
            {
                otherPoint = endPoint;
                endPoint = endPoint - directions[currIndex] * lengths[currIndex];
            }

            //check if endPoint is between the other two endPoints of the T axis, and directions are roughly orthogonal
                
            endPoint = MeasureClass::projectPointToLine(endPoint, directions[i+1], centers[i+1]);
            directions[currIndex] = endPoint - otherPoint;
            lengths[currIndex] = glm::length(directions[currIndex]);
            directions[currIndex] = directions[currIndex] / glm::length(directions[currIndex]);
            centers[currIndex] = 0.5 * (endPoint + otherPoint);
            
            if (currIndex == i)
            {
                elbowCenters.push_back(glm::dvec3(0.0, 0.0, 0.0));
                elbowEdges.push_back(std::vector<glm::dvec3>(0));
                elbowPoints.push_back(glm::dvec3(0.0, 0.0, 0.0));
                shouldContinue = true;
                if (!connectionTypeUpdated)
                {
                    connectionType.push_back(LineConnectionType::stitching);
                    connectionTypeUpdated = true;
                }
                continue;
            }
        }

        if (shouldContinue)
            continue;
        if ((isStitch)&&(!skipStitch))
        {
            
            elbowCenters.push_back(glm::dvec3(0.0, 0.0, 0.0));
            elbowEdges.push_back(std::vector<glm::dvec3>(0));
            elbowPoints.push_back(glm::dvec3(0.0, 0.0, 0.0));
            if (!connectionTypeUpdated)
            {
                connectionType.push_back(LineConnectionType::stitching);
                connectionTypeUpdated = true;
            }
            continue;
        }

        //check if axes are not coplanar
        if ((lineToLineDistance(directions[i], directions[i+1], centers[i], centers[i+1]) > 0.3*std::max(radii[i],radii[i+1])))
        {
            
            elbowCenters.push_back(glm::dvec3(0.0, 0.0, 0.0));
            elbowEdges.push_back(std::vector<glm::dvec3>(0));
            elbowPoints.push_back(glm::dvec3(0.0, 0.0, 0.0));
            if (!connectionTypeUpdated)
            {
                connectionType.push_back(LineConnectionType::none);
                connectionTypeUpdated = true;
            }
            continue;
        }


        std::vector<glm::dvec3> endPoints, middlePoints;
        getEndAndMiddlePoints(centers[i], directions[i], lengths[i], centers[i+1], directions[i+1], lengths[i+1], endPoints, middlePoints);
        
        double S = glm::dot(directions[i], directions[i+1]);
        glm::dvec3 measurePoint1, measurePoint2,bestPointForCoaxial;
        bestPointForCoaxial = 0.5 * (middlePoints[0] + middlePoints[1]);
        measurePoint1 = centers[i] + directions[i] * ((glm::dot(centers[i+1] - centers[i], directions[i] - S * directions[i+1])) / (1 - S * S));
        measurePoint2 = centers[i+1] + directions[i+1] * ((glm::dot(centers[i+1] - centers[i], S*directions[i] - directions[i+1])) / (1 - S * S));
        middlePoints[0] = measurePoint1;
        middlePoints[1] = measurePoint2;
        
        glm::dvec3 mPoint = 0.5*(middlePoints[0] + middlePoints[1]);
        if (std::fabs(abs(glm::length(mPoint - endPoints[0]))) <= std::numeric_limits<double>::epsilon())
            return false;
        glm::dvec3 dir = (mPoint - endPoints[0]) / glm::length(mPoint - endPoints[0]) + (mPoint - endPoints[1]) / glm::length(mPoint - endPoints[1]);
        dir = dir / glm::length(dir);
        glm::dvec3 bestPoint = mPoint;
        double bestError = abs(targetCosAngle - abs(glm::dot(mPoint - endPoints[0], mPoint - endPoints[1])) / ((glm::length(mPoint - endPoints[0])*(glm::length(mPoint - endPoints[1])))));
        bool globalLoopSuccess = false;

        if ((targetCosAngle > 0.999)||!angleConstraints)
        {
            bestError = abs(glm::dot(mPoint - endPoints[0], middlePoints[0] - endPoints[0]) / (glm::length(mPoint[0] - endPoints[0])*glm::length(middlePoints[0] - endPoints[0])) - glm::dot(mPoint - endPoints[1], middlePoints[1] - endPoints[1]) / (glm::length(mPoint[1] - endPoints[1])*glm::length(middlePoints[1] - endPoints[1])));		
            globalLoopSuccess = true;
        }
        else
        {
            int kMax(100);
            double step = 0.001;
            for (int k = 1; k < kMax; k++)
            {
                bool loopSuccess(false);
                glm::dvec3 currPoint = mPoint + (double)k*step*dir;
                double currError = abs(targetCosAngle - abs(glm::dot(currPoint - endPoints[0], currPoint - endPoints[1])) / ((glm::length(currPoint - endPoints[0])*(glm::length(currPoint - endPoints[1])))));
                if (targetCosAngle > 0.999)
                {
                    currError = abs(glm::dot(mPoint - endPoints[0], middlePoints[0] - endPoints[0]) / (glm::length(mPoint[0] - endPoints[0])*glm::length(middlePoints[0] - endPoints[0])) - glm::dot(mPoint - endPoints[1], middlePoints[1] - endPoints[1]) / (glm::length(mPoint[1] - endPoints[1])*glm::length(middlePoints[1] - endPoints[1])));
                }
                if (currError < bestError)
                {
                    bestError = currError;
                    bestPoint = currPoint;
                    loopSuccess = true;
                }
                currPoint = mPoint - (double)k*step*dir;
                currError = abs(targetCosAngle - abs(glm::dot(currPoint - endPoints[0], currPoint - endPoints[1])) / ((glm::length(currPoint - endPoints[0])*(glm::length(currPoint - endPoints[1])))));
                if (targetCosAngle > 0.999)
                {
                    currError = abs(glm::dot(mPoint - endPoints[0], middlePoints[0] - endPoints[0]) / (glm::length(mPoint[0] - endPoints[0])*glm::length(middlePoints[0] - endPoints[0])) - glm::dot(mPoint - endPoints[1], middlePoints[1] - endPoints[1]) / (glm::length(mPoint[1] - endPoints[1])*glm::length(middlePoints[1] - endPoints[1])));
                }
                if (currError < bestError)
                {
                    bestError = currError;
                    bestPoint = currPoint;
                    loopSuccess = true;
                }
                if (loopSuccess)
                    globalLoopSuccess = true;
                else
                    break;
            }

            //do it again with a smaller step
            mPoint = bestPoint;
            step = 0.000001;
            kMax = 1000;
            for (int k = 1; k < kMax; k++)
            {
                bool loopSuccess(false);
                glm::dvec3 currPoint = mPoint + (double)k*step*dir;
                double currError = abs(targetCosAngle - abs(glm::dot(currPoint - endPoints[0], currPoint - endPoints[1])) / ((glm::length(currPoint - endPoints[0])*(glm::length(currPoint - endPoints[1])))));
                if (targetCosAngle > 0.999)
                {
                    currError = abs(glm::dot(mPoint - endPoints[0], middlePoints[0] - endPoints[0]) / (glm::length(mPoint[0] - endPoints[0])*glm::length(middlePoints[0] - endPoints[0])) - glm::dot(mPoint - endPoints[1], middlePoints[1] - endPoints[1]) / (glm::length(mPoint[1] - endPoints[1])*glm::length(middlePoints[1] - endPoints[1])));
                }
                if (currError < bestError)
                {
                    bestError = currError;
                    bestPoint = currPoint;
                    loopSuccess = true;
                }
                currPoint = mPoint - (double)k*step*dir;
                currError = abs(targetCosAngle - abs(glm::dot(currPoint - endPoints[0], currPoint - endPoints[1])) / ((glm::length(currPoint - endPoints[0])*(glm::length(currPoint - endPoints[1])))));
                if (targetCosAngle > 0.999)
                {
                    currError = abs(glm::dot(mPoint - endPoints[0], middlePoints[0] - endPoints[0]) / (glm::length(mPoint[0] - endPoints[0])*glm::length(middlePoints[0] - endPoints[0])) - glm::dot(mPoint - endPoints[1], middlePoints[1] - endPoints[1]) / (glm::length(mPoint[1] - endPoints[1])*glm::length(middlePoints[1] - endPoints[1])));
                }
                if (currError < bestError)
                {
                    bestError = currError;
                    bestPoint = currPoint;
                    loopSuccess = true;
                }
                if (loopSuccess)
                    globalLoopSuccess = true;
                else
                    break;
            }
        }
        
        if ((!globalLoopSuccess)&&(bestError>0.01)&&(targetCosAngle<0.995))
            return false;
        if ((!globalLoopSuccess) && (targetCosAngle > 0.999) && (bestError > 0.1))
            return false;

        //we now have the correct middle point, we need to update the cylinders parameters accordingly
        
        if (targetCosAngle>0.999)
        {
            //if different radius, don't extend
            if (abs(radii[i] - radii[i+1]) / std::max(radii[i], radii[i+1]) > 0.2)
            {
                directions[i] = bestPoint - endPoints[0];
                directions[i] = directions[i] / glm::length(directions[i]);
                centers[i] = endPoints[0] + directions[i] * lengths[i] * 0.5;
                directions[i+1] = bestPoint - endPoints[1];
                directions[i+1] = directions[i+1] / glm::length(directions[i+1]);
                centers[i+1] = endPoints[1] + directions[i+1] * lengths[i+1] * 0.5;
                if (!connectionTypeUpdated)
                {
                    connectionTypeUpdated = true;
                    connectionType.push_back(LineConnectionType::reduction);
                }
            }
            
            else //else extend like normal
            {
                bestPoint = bestPointForCoaxial;
                centers[i] = (endPoints[0] + bestPoint)*0.5;
                lengths[i] = glm::length(endPoints[0] - bestPoint);
                directions[i] = (bestPoint - endPoints[0]) / lengths[i];
                centers[i+1] = (endPoints[1] + bestPoint)*0.5;
                lengths[i+1] = glm::length(endPoints[1] - bestPoint);
                directions[i+1] = (bestPoint - endPoints[1]) / lengths[i+1];
                if (!connectionTypeUpdated)
                {
                    connectionType.push_back(LineConnectionType::coaxial);
                    connectionTypeUpdated = true;
                }
            }
            elbowPoints.push_back(bestPoint);
            std::vector<glm::dvec3> tempEdges;
            tempEdges.push_back(bestPoint);
            tempEdges.push_back(bestPoint);
            elbowEdges.push_back(tempEdges);
            elbowCenters.push_back(bestPoint);
        }
        else
        {
            
            centers[i] = (endPoints[0] + bestPoint)*0.5;
            lengths[i] = glm::length(endPoints[0] - bestPoint);
            directions[i] = (bestPoint - endPoints[0]) / lengths[i];
            centers[i+1] = (endPoints[1] + bestPoint)*0.5;
            lengths[i+1] = glm::length(endPoints[1] - bestPoint);
            directions[i+1] = (bestPoint - endPoints[1]) / lengths[i+1];
            if (!connectionTypeUpdated)
            {
                connectionType.push_back(LineConnectionType::elbow);
                connectionTypeUpdated = true;
            }
            
            glm::dvec3 u, v, s;
            u = centers[i] - bestPoint;
            u = u / glm::length(u);
            v = centers[i+1] - bestPoint;
            v = v / glm::length(v);
            s = u + v;
            s = s / glm::length(s);
            double t;
            double localRadius = mainRadius * 2 * std::max(radii[i], radii[i+1]);
            t = localRadius / glm::length(s - glm::dot(u, s)*u);
            double halfAlpha = acos(glm::dot(u, s));
            double distBestCenter = localRadius * sqrt(pow(1 / tan(halfAlpha), 2) + 1);
            //t = -0.5*(localRadius + std::max(radii[i], radii[i + 1])) + t;
            glm::dvec3 elbow = bestPoint + (-localRadius + t) * s;
            glm::dvec3 elbowCenter = bestPoint + s * distBestCenter;
            middlePoints[0] = bestPoint + t * u*glm::dot(u, s);
            middlePoints[1] = bestPoint + t * v*glm::dot(v, s);
            //update cylinders parameters again

            centers[i] = (endPoints[0] + middlePoints[0])*0.5;
            lengths[i] = glm::length(endPoints[0] - middlePoints[0]);
            directions[i] = (endPoints[0] - middlePoints[0]) / lengths[i];
            centers[i+1] = (endPoints[1] + middlePoints[1])*0.5;
            lengths[i+1] = glm::length(endPoints[1] - middlePoints[1]);
            directions[i+1] = (endPoints[1] - middlePoints[1]) / lengths[i+1];
            elbowPoints.push_back(elbow);
            elbowCenters.push_back(elbowCenter);
            std::vector<glm::dvec3> tempEdges;
            tempEdges.push_back(middlePoints[0]);
            tempEdges.push_back(middlePoints[1]);
            elbowEdges.push_back(tempEdges);		
        }
    }
    for (int i = 0; i < (int)centers.size(); i++)
    {
        angleModifs.push_back(abs(glm::dot(directions[i], initialDirections[i])));
    }
    return true;
}

void TlScanOverseer::getEndAndMiddlePoints(const glm::dvec3& center1, const glm::dvec3& direction1, const double& length1, const glm::dvec3& center2, const glm::dvec3& direction2, const double& length2, std::vector<glm::dvec3>& endPoints, std::vector<glm::dvec3>& middlePoints)
{
    glm::dvec3 A(center1 + 0.5*direction1*length1), B(center1 - 0.5*direction1*length1), C(center2 + 0.5*direction2*length2), D(center2 - 0.5*direction2*length2);
    glm::dvec3 end1(B), end2(D), middle1(A), middle2(C);
    double dAC(glm::length(A-C)), dAD(glm::length(A-D)),dBC(glm::length(B-C)),dBD(glm::length(B-D));
    //double dMin = dAC;
    if ((dAD <= dAC)&&(dAD<=dBC)&&(dAD<=dBD))
    {
        middle1 = A;
        end1 = B;
        middle2 = D;
        end2 = C;
    }
    if ((dAC<=dAD)&&(dAC<=dBC)&&(dAC<=dBD))
    {
        middle1 = A;
        end1 = B;
        middle2 = C;
        end2 = D;
    }
    if ((dBC<=dAC)&&(dBC<=dAD)&&(dBC<=dBD))
    {
        middle1 = B;
        end1 = A;
        middle2 = C;
        end2 = D;
    }
    if ((dBD <= dAC) && (dBD <= dAD) && (dBD <= dBC))
    {
        middle1 = B;
        end1 = A;
        middle2 = D;
        end2 = C;
    }

    //if coaxial, we use a different criterion
    if (abs(glm::dot(direction1, direction2)) > 0.99)
    {
        glm::dvec3 dir(0.5 * (A + B - C - D));
        glm::dvec3 virtualCenter(0.25 * (A + B + C + D));
        double dA(glm::dot(A - virtualCenter, dir)), dB(glm::dot(B - virtualCenter, dir)), dC(glm::dot(C - virtualCenter, dir)), dD(glm::dot(D - virtualCenter, dir));
        if (dA > dB)
        {
            middle1 = B;
            end1 = A;
        }
        else
        {
            middle1 = A;
            end1 = B;
        }
        if (dC > dD)
        {
            middle2 = C;
            end2 = D;
        }
        else
        {
            middle2 = D;
            end2 = C;
        }
    }

    std::vector<glm::dvec3> resultEndPoints,resultMiddlePoints;
    resultMiddlePoints.push_back(middle1);
    resultMiddlePoints.push_back(middle2);
    middlePoints = resultMiddlePoints;
    resultEndPoints.push_back(end1);
    resultEndPoints.push_back(end2);
    endPoints = resultEndPoints;

    

    return;
}

glm::dvec3 TlScanOverseer::computeElbowPosition(const glm::dvec3& center1, const glm::dvec3& center2, const glm::dvec3& elbowPoint, const double& radius)
{
    glm::dvec3 u, v,s;
    u = center1 - elbowPoint;
    u = u / glm::length(u);
    v = center2 - elbowPoint;
    v = v / glm::length(v);
    s = u + v;
    s = s / glm::length(s);
    double t;
    t = radius / glm::length(v - glm::dot(u, v)*u);
    return elbowPoint + t * s;
}

glm::dvec3 TlScanOverseer::computeIntersectionAxes(const glm::dvec3& center1, const glm::dvec3& center2, const glm::dvec3& dir1, const glm::dvec3& dir2, const double& length1, const double& length2, const bool& angleConstraints)
{
    std::vector<glm::dvec3> endPoints, middlePoints;
    getEndAndMiddlePoints(center1, dir1, length1, center2, dir2, length2, endPoints, middlePoints);
    double S = glm::dot(dir1, dir2);
    glm::dvec3 measurePoint1, measurePoint2;
    measurePoint1 = center1 + dir1 * ((glm::dot(center2 - center1, dir1 - S * dir2)) / (1 - S * S));
    measurePoint2 = center2 + dir2 * ((glm::dot(center2 - center1, S*dir1 - dir2)) / (1 - S * S));
    middlePoints[0] = measurePoint1;
    middlePoints[1] = measurePoint2;
    double targetCosAngle = getTargetCosAngle(dir1, dir2);
    

    glm::dvec3 mPoint = 0.5*(middlePoints[0] + middlePoints[1]);
    if (std::fabs(abs(glm::length(mPoint - endPoints[0]))) <= std::numeric_limits<double>::epsilon())
        return mPoint;
    glm::dvec3 dir = (mPoint - endPoints[0]) / glm::length(mPoint - endPoints[0]) + (mPoint - endPoints[1]) / glm::length(mPoint - endPoints[1]);
    dir = dir / glm::length(dir);
    glm::dvec3 bestPoint = mPoint;
    double bestError = abs(targetCosAngle - abs(glm::dot(mPoint - endPoints[0], mPoint - endPoints[1])) / ((glm::length(mPoint - endPoints[0])*(glm::length(mPoint - endPoints[1])))));
    bool globalLoopSuccess = false;

    if ((targetCosAngle > 0.999) || !angleConstraints)
    {
        bestError = abs(glm::dot(mPoint - endPoints[0], middlePoints[0] - endPoints[0]) / (glm::length(mPoint[0] - endPoints[0])*glm::length(middlePoints[0] - endPoints[0])) - glm::dot(mPoint - endPoints[1], middlePoints[1] - endPoints[1]) / (glm::length(mPoint[1] - endPoints[1])*glm::length(middlePoints[1] - endPoints[1])));
        globalLoopSuccess = true;
    }
    else
    {
        int kMax(100);
        double step = 0.001;
        for (int k = 1; k < kMax; k++)
        {
            bool loopSuccess(false);
            glm::dvec3 currPoint = mPoint + (double)k*step*dir;
            double currError = abs(targetCosAngle - abs(glm::dot(currPoint - endPoints[0], currPoint - endPoints[1])) / ((glm::length(currPoint - endPoints[0])*(glm::length(currPoint - endPoints[1])))));
            if (targetCosAngle > 0.999)
            {
                currError = abs(glm::dot(mPoint - endPoints[0], middlePoints[0] - endPoints[0]) / (glm::length(mPoint[0] - endPoints[0])*glm::length(middlePoints[0] - endPoints[0])) - glm::dot(mPoint - endPoints[1], middlePoints[1] - endPoints[1]) / (glm::length(mPoint[1] - endPoints[1])*glm::length(middlePoints[1] - endPoints[1])));
            }
            if (currError < bestError)
            {
                bestError = currError;
                bestPoint = currPoint;
                loopSuccess = true;
            }
            currPoint = mPoint - (double)k*step*dir;
            currError = abs(targetCosAngle - abs(glm::dot(currPoint - endPoints[0], currPoint - endPoints[1])) / ((glm::length(currPoint - endPoints[0])*(glm::length(currPoint - endPoints[1])))));
            if (targetCosAngle > 0.999)
            {
                currError = abs(glm::dot(mPoint - endPoints[0], middlePoints[0] - endPoints[0]) / (glm::length(mPoint[0] - endPoints[0])*glm::length(middlePoints[0] - endPoints[0])) - glm::dot(mPoint - endPoints[1], middlePoints[1] - endPoints[1]) / (glm::length(mPoint[1] - endPoints[1])*glm::length(middlePoints[1] - endPoints[1])));
            }
            if (currError < bestError)
            {
                bestError = currError;
                bestPoint = currPoint;
                loopSuccess = true;
            }
            if (loopSuccess)
                globalLoopSuccess = true;
            else
                break;
        }

        //do it again with a smaller step
        mPoint = bestPoint;
        step = 0.0001;
        kMax = 10;
        for (int k = 1; k < kMax; k++)
        {
            bool loopSuccess(false);
            glm::dvec3 currPoint = mPoint + (double)k*step*dir;
            double currError = abs(targetCosAngle - abs(glm::dot(currPoint - endPoints[0], currPoint - endPoints[1])) / ((glm::length(currPoint - endPoints[0])*(glm::length(currPoint - endPoints[1])))));
            if (targetCosAngle > 0.999)
            {
                currError = abs(glm::dot(mPoint - endPoints[0], middlePoints[0] - endPoints[0]) / (glm::length(mPoint[0] - endPoints[0])*glm::length(middlePoints[0] - endPoints[0])) - glm::dot(mPoint - endPoints[1], middlePoints[1] - endPoints[1]) / (glm::length(mPoint[1] - endPoints[1])*glm::length(middlePoints[1] - endPoints[1])));
            }
            if (currError < bestError)
            {
                bestError = currError;
                bestPoint = currPoint;
                loopSuccess = true;
            }
            currPoint = mPoint - (double)k*step*dir;
            currError = abs(targetCosAngle - abs(glm::dot(currPoint - endPoints[0], currPoint - endPoints[1])) / ((glm::length(currPoint - endPoints[0])*(glm::length(currPoint - endPoints[1])))));
            if (targetCosAngle > 0.999)
            {
                currError = abs(glm::dot(mPoint - endPoints[0], middlePoints[0] - endPoints[0]) / (glm::length(mPoint[0] - endPoints[0])*glm::length(middlePoints[0] - endPoints[0])) - glm::dot(mPoint - endPoints[1], middlePoints[1] - endPoints[1]) / (glm::length(mPoint[1] - endPoints[1])*glm::length(middlePoints[1] - endPoints[1])));
            }
            if (currError < bestError)
            {
                bestError = currError;
                bestPoint = currPoint;
                loopSuccess = true;
            }
            if (loopSuccess)
                globalLoopSuccess = true;
            else
                break;
        }
    }
    return bestPoint;
}

void TlScanOverseer::getElbowParameters(glm::dvec3& center1, glm::dvec3& center2, glm::dvec3& dir1, glm::dvec3& dir2, double& length1, double& length2, const double& radius1, const double& radius2, double& mainAngle, const double& mainRadius, const glm::dvec3& elbowRealCenter, glm::dvec3& elbowPosResult, TransformationModule& mod, const double& targetCosAngle, const bool& angleConstraints)
{
    // updates two cylinders, and gives the elbow parameters
    if (!angleConstraints)
    {
mainAngle = acos(abs(glm::dot(dir1, dir2)));
    }
    else mainAngle = acos(targetCosAngle);
    double radius = std::max(radius1, radius2);
    elbowPosResult = computeElbowPosition(center1, center2, elbowRealCenter, radius);
    glm::dvec3 u, v, s;
    u = center1 - elbowRealCenter;
    u = u / glm::length(u);
    v = center2 - elbowRealCenter;
    v = v / glm::length(v);
    s = u + v;
    s = s / glm::length(s);
    double t;
    double localRadius = mainRadius * 2 * radius;
    t = localRadius / glm::length(s - glm::dot(u, s) * u);
    //			glm::dvec3 elbow = bestPoint + (-1*(localRadius + 0*std::max(radii[i], radii[i + 1])) + t) * s;
    //glm::dvec3 elbow = axesIntersection + (-1 * (localRadius + 0 * radius) + t) * s;
    glm::dvec3 middlePoint1 = elbowRealCenter + (t)*u * glm::dot(u, s);
    glm::dvec3 middlePoint2 = elbowRealCenter + (t)*v * glm::dot(v, s);
    //update cylinders parameters again
    std::vector<glm::dvec3> endPoints, middlePoints;
    getEndAndMiddlePoints(center1, dir1, length1, center2, dir2, length2, endPoints, middlePoints);

    center1 = (endPoints[0] + middlePoints[0]) * 0.5;
    length1 = glm::length(endPoints[0] - middlePoints[0]);
    dir1 = (endPoints[0] - middlePoints[0]) / length1;
    center2 = (endPoints[1] + middlePoints[1]) * 0.5;
    length2 = glm::length(endPoints[1] - middlePoints[1]);
    dir2 = (endPoints[1] - middlePoints[1]) / length2;

    mod.setPosition(elbowPosResult);
    mod.setScale(glm::dvec3(0.5 * (mainRadius + radius), 0.5 * (mainRadius + radius), radius));
    double sign1, sign2;
    if (glm::dot(dir1, center1 - middlePoint1) > 0)
    {
        sign1 = -1;
    }
    else { sign1 = +1; }
    if (glm::dot(dir2, center2 - middlePoint2) > 0)
    {
        sign2 = 1;
    }
    else { sign2 = -1; }


    glm::dquat quat1 = glm::rotation(glm::dvec3(0.0, 1.0, 0.0), sign1 * dir1);
    glm::dvec3 up = glm::normalize(glm::cross(sign1 * dir1, sign2 * dir2));
    glm::dvec3 newUp = quat1 * glm::dvec3(0.0, 0.0, 1.0);
    glm::dquat quat2 = glm::rotation(newUp, up);
    mod.setRotation(quat2 * quat1);
}

double TlScanOverseer::getTargetCosAngle(const glm::dvec3& direction1, const glm::dvec3& direction2)
{
    double S = abs(glm::dot(direction1, direction2));
    if (S < 0.259)
        return 0;
    if (S < 0.609)
        return 0.5;
    if (S < 0.793)
        return (0.5 * sqrt(2));
    if (S < 0.96)
        return (0.5 * sqrt(3));
    else return 1;
}

bool TlScanOverseer::lookForStitchConnexion(glm::dvec3& center, glm::dvec3& direction, double& length, const std::vector<glm::dvec3>& cylCenters, const std::vector<glm::dvec3> cylDirections, const std::vector<double> cylRadii, const std::vector<double>& lengths, const double& radius)
{
    //this checks if a given cylinder is stitched onto a main tube


    for (int i = 0; i < (int)cylCenters.size(); i++)
    {
        double axesThreshold(std::max(radius, cylRadii[i]) + 0.4 * std::min(radius, cylRadii[i]));

        //check if coplanar
        if (lineToLineDistance(direction, cylDirections[i], center, cylCenters[i]) > axesThreshold)
            continue;
        glm::dvec3 endPoint = center + direction * 0.5 * length;
        glm::dvec3 otherPoint = center - direction * 0.5 * length;

        glm::dvec3 endLinePoint, otherLinePoint;
        getTotalCoaxialLine(endLinePoint, otherLinePoint, cylCenters, cylDirections, lengths, cylCenters[i], cylDirections[i], lengths[i]);
        glm::dvec3 currDir = endLinePoint - otherLinePoint;
        currDir = currDir / glm::length(currDir);
        glm::dvec3 currCenter = 0.5 * (endLinePoint + otherLinePoint);
        //make endPoint be the closest to the othe axis
        if (pointToLineDistance(endPoint, currDir, currCenter) > pointToLineDistance(endPoint - direction * length, currDir, currCenter))
        {
            otherPoint = endPoint;
            endPoint = endPoint - direction * length;
        }

        //check if endPoint is between the other two endPoints of the T axis, and directions are roughly orthogonal

        if ((glm::dot(endPoint - endLinePoint, otherLinePoint - endLinePoint) > 0) && (glm::dot(endPoint - otherLinePoint, endLinePoint - otherLinePoint) > 0)  && (abs(glm::dot(direction, currDir)) < 0.9))
        {
            
            bool condition1, condition2;
            // the last two cylinders are coplanar
            condition1 = lineToLineDistance(cylDirections[cylDirections.size() - 1], direction, cylCenters[cylCenters.size() - 1], center) < axesThreshold;
            // the previous cylinder is the same axis as the stitching section
            condition2 = (abs(glm::dot(cylDirections[cylDirections.size() - 1], currDir) > 0.99) && (lineToLineDistance(cylDirections[cylDirections.size() - 1], currDir, cylCenters[cylCenters.size() - 1], currCenter) < axesThreshold));

            //if previous section is coplanar and is not the same axis, then ignore because it should be an elbow
            if (condition1 && !condition2)
                continue;
            /*if (((abs(glm::dot(cylDirections[cylDirections.size() - 1], currDir))) < 0.99) || (pointToLineDistance(cylCenters[cylCenters.size() - 1], currDir, currCenter) > 0.01))
            {
                continue;
            }*/
            
            endPoint = MeasureClass::projectPointToLine(endPoint, currDir, currCenter);
            direction = endPoint - otherPoint;
            length = glm::length(direction);
            direction = direction / glm::length(direction);
            center = 0.5*(endPoint + otherPoint);
            return true;
        }
    }
    return false;
}



std::vector<int> TlScanOverseer::lookForSecondaryStitchConnexion(const glm::dvec3& center, const glm::dvec3& direction, const double& length, const std::vector<glm::dvec3>& cylCenters, const std::vector<glm::dvec3> cylDirections, const std::vector<double>& cylRadii, const std::vector<double>& lengths, const double& radius)
{
    //this checks if other cylinders are stitched onto this one, and return indices of them
    //this should give indices of potential elbows to be removed
    std::vector<int> result;
    glm::dvec3 endPoint1, endPoint2;
    std::vector<int> coaxialIndices;
    coaxialIndices=getTotalCoaxialLine(endPoint1, endPoint2, cylCenters, cylDirections, lengths, center, direction, length);
    glm::dvec3 currDir = endPoint2 - endPoint1;
    currDir = currDir / glm::length(currDir);
    glm::dvec3 currCenter = 0.5 * (endPoint1 + endPoint2);
    double currLength = glm::length(endPoint2 - endPoint1);
    for (int i = 0; i < (int)cylCenters.size(); i++)
    {
        double axesThreshold(std::max(radius,cylRadii[i])+0.4*std::min(radius,cylRadii[i]));

        double condition1(lineToLineDistance(currDir, cylDirections[i], currCenter, cylCenters[i])), condition2(abs(glm::dot(currDir, cylDirections[i])));
        //is current cylinder a good candidate for a stitched tube
        if ((lineToLineDistance(currDir, cylDirections[i], currCenter, cylCenters[i]) < axesThreshold) && (abs(glm::dot(currDir, cylDirections[i])) < 0.9))
        {
            //axes are coplanar and orthogonal
            glm::dvec3 stitchingPoint = cylCenters[i] + 0.5 * lengths[i] * cylDirections[i];
            if (pointToLineDistance(stitchingPoint, currDir, currCenter) > pointToLineDistance(stitchingPoint - lengths[i] * cylDirections[i], currDir, currCenter))
                stitchingPoint = stitchingPoint - lengths[i] * cylDirections[i];
            if (glm::dot(stitchingPoint - endPoint1, stitchingPoint - endPoint2) < 0)
            {
                //here is a stitching pipe
                for (int j = 0; j < (int)coaxialIndices.size(); j++)
                {
                    //add to result either/both of the connections indices to the main pipe
                    
                    if ((i + 1) == coaxialIndices[j])
                        result.push_back(i);
                    if ((i - 1) == coaxialIndices[j])
                        result.push_back(i - 1);
                }
            }
        }
    }
    return result;
}
std::vector<int> TlScanOverseer::getTotalCoaxialLine(glm::dvec3& endPoint1, glm::dvec3& endPoint2, const std::vector<glm::dvec3>& centers, const std::vector<glm::dvec3>& directions, const std::vector<double>& lengths, const glm::dvec3& currCenter, const glm::dvec3& currDir, const double& currLength)
{
    //returns indices of coaxial pipes
    std::vector<int> result;
    endPoint1 = currCenter - 0.5 * currLength * currDir;
    endPoint2 = currCenter + 0.5 * currLength * currDir;
    for (int i = 0; i < (int)centers.size(); i++)
    {
        //check if axes are coaxial
        double lineDistanceThreshold(0.05), lineDirectionThreshold(0.99);
        glm::dvec3 tempDir = endPoint2 - endPoint1;
        tempDir = tempDir / glm::length(tempDir);
        glm::dvec3 tempCenter = 0.5 * (endPoint1 + endPoint2);
        double condition1(lineToLineDistance(tempDir, directions[i], tempCenter, centers[i])), condition2(abs(glm::dot(tempDir, directions[i])));
        if ((lineToLineDistance(tempDir, directions[i], tempCenter, centers[i]) < lineDistanceThreshold) && (abs(glm::dot(tempDir, directions[i])) > lineDirectionThreshold))
        {
            //axes are coaxial
            result.push_back(i);
            glm::dvec3 currEndPoint1(centers[i] - 0.5 * lengths[i] * directions[i]), currEndPoint2(centers[i] + 0.5 * lengths[i] * directions[i]);
            std::vector<glm::dvec3> candidateEndPoints;
            candidateEndPoints.push_back(endPoint1);
            candidateEndPoints.push_back(endPoint2);
            candidateEndPoints.push_back(centers[i] - 0.5 * lengths[i] * directions[i]);
            candidateEndPoints.push_back(centers[i] + 0.5 * lengths[i] * directions[i]);
            endPoint1 = getPointFurthestAway(tempCenter, tempDir, candidateEndPoints);
            endPoint2 = getPointFurthestAway(tempCenter, -tempDir, candidateEndPoints);
        }
    }
    return result;
}

glm::dvec3 TlScanOverseer::getPointFurthestAway(const glm::dvec3& center, const glm::dvec3& direction, const std::vector<glm::dvec3> points)
{
    glm::dvec3 result(glm::dvec3(0.0, 0.0, 0.0));
    for (int i = 0; i < (int)points.size(); i++)
    {
        if (i == 0)
            result = points[0];
        else
        {
            if (glm::dot(points[i] - center, direction) > glm::dot(result - center, direction))
                result = points[i];
        }
    }
    return result;
}

double TlScanOverseer::pointToLineDistance(const glm::dvec3& point, const glm::dvec3& direction, const glm::dvec3& linePoint)
{
    return glm::length(glm::cross(linePoint - point, direction)) / glm::length(direction);
}

bool TlScanOverseer::arrangeCylindersInLine(std::vector<glm::dvec3>& centers, std::vector<glm::dvec3>& directions, std::vector<double>& lengths, std::vector<double>& radii, std::vector<int>& order)
{
    //intialize parameters
    int size = (int)centers.size();
    std::vector<bool> alreadyAdded;
    std::vector<std::vector<glm::dvec3>> extremities;
    std::vector<glm::dvec3> centerResult(centers), directionResult(directions);
    std::vector<double> lengthResult(lengths), radiusResult(radii), leftDistances, rightDistances;
    std::vector<std::vector<int>> links;
    double threshold(0.02),worstExtr(0);
    int startLine(0);
    order = std::vector<int>(0);
    for (int i = 0; i < size; i++)
    {
        alreadyAdded.push_back(false);
        std::vector<glm::dvec3> temp;
        temp.push_back(centers[i] + directions[i] * lengths[i] * 0.5);
        temp.push_back(centers[i] - directions[i] * lengths[i] * 0.5);
        extremities.push_back(temp);
        std::vector<int> tempLink;
        tempLink.push_back(i);
        tempLink.push_back(i);
        links.push_back(tempLink);
        leftDistances.push_back(1);
        rightDistances.push_back(1);
        std::vector<int> order;
    }

    for (int i = 0; i < size; i++)
    {
        bool leftSuccess(false), rightSuccess(false);
        int bestLeft(i), bestRight(i);
        double bestLeftDist(1), bestRightDist(1);
        double dMinLeft(100), dMinRight(100),dMinLeftExtr(100),dMinRightExtr(100);
        for (int j = 0; j < size; j++)
        {			
            if ((i != j)&&(!alreadyAdded[j]))
            {
                //distance from i to j
                double dist = lineToLineDistance(directions[i], directions[j], centers[i], centers[j]);
                //left or right
                double leftEstimate = std::min(glm::length(extremities[i][0] - extremities[j][0]), glm::length(extremities[i][0] - extremities[j][1]));
                double rightEstimate = std::min(glm::length(extremities[i][1] - extremities[j][0]), glm::length(extremities[i][1] - extremities[j][1]));
                if ((leftEstimate < rightEstimate) && (dist < dMinLeft+ threshold))
                {
                    dMinLeft = dist;
                    if (leftEstimate < dMinLeftExtr)
                    {
                        bestLeft = j;
                        leftSuccess = true;
                        dMinLeftExtr = leftEstimate;
                    }
                }
                if ((rightEstimate < leftEstimate) && (dist < dMinRight+ threshold))
                {
                    dMinRight = dist;
                    if (rightEstimate < dMinRightExtr)
                    {
                        bestRight = j;
                        rightSuccess = true;
                        dMinRightExtr = rightEstimate;
                    }					
                }
            }
        }
        links[i][0] = bestLeft;
        links[i][1] = bestRight;
        leftDistances[i] = dMinLeftExtr;
        rightDistances[i] = dMinRightExtr;
        if (dMinLeftExtr > worstExtr)
        {
            worstExtr = dMinLeftExtr;
            startLine = i;
        }
        if (dMinRightExtr > worstExtr)
        {
            worstExtr = dMinRightExtr;
            startLine = i;
        }
    }
    order.push_back(startLine);
    alreadyAdded[startLine] = true;
    for (int counter = 1; counter < size; counter++)
    {
        bool shouldbreak(false);
        std::vector<int> currentLink = links[order[order.size() - 1]];
        if ((!alreadyAdded[currentLink[0]]) && (alreadyAdded[currentLink[1]]))
        {
            order.push_back(currentLink[0]);
            alreadyAdded[currentLink[0]] = true;
        }
        else if ((alreadyAdded[currentLink[0]]) && (!alreadyAdded[currentLink[1]]))
        {
            order.push_back(currentLink[1]);
            alreadyAdded[currentLink[1]] = true;
        }
        else
        {
            //add everything not yet added in any order
            shouldbreak = true;
            for (int t = 0; t < (int)alreadyAdded.size(); t++)
            {
                if (!alreadyAdded[t])
                    order.push_back(t);
            }
        }
        if (shouldbreak)
            break;
            
    }
    for (int i = 0; i < size; i++)
    {
        centerResult[i] = centers[order[i]];
        directionResult[i] = directions[order[i]];
        radiusResult[i] = radii[order[i]];
        lengthResult[i] = lengths[order[i]];
    }
    centers = centerResult;
    directions = directionResult;
    radii = radiusResult;
    lengths = lengthResult;
    return true;
}

double TlScanOverseer::lineToLineDistance(const glm::dvec3& dir1, const glm::dvec3& dir2, const glm::dvec3& point1, const glm::dvec3& point2)
{
    /*glm::dvec3 A = point2 - point1;
    double mixte = A.x*dir1.y*dir2.z + A.y*dir1.z*dir2.x + A.z*dir1.x*dir2.y - A.x*dir1.z*dir2.y - A.y*dir1.x*dir2.z - A.z*dir1.y*dir2.x;
    return abs(mixte / glm::length(glm::cross(dir1, dir2)));*/
    return abs(glm::dot(glm::cross(dir1, dir2), point1 - point2) / glm::length(glm::cross(dir1, dir2)));
}

glm::dvec3 TlScanOverseer::computeAreaOfPolyline(std::vector<Measure> measures)
{
    std::vector<glm::dvec3> points;
    for (int i = 0; i < (int)measures.size(); i++)
    {
        points.push_back(measures[i].origin);
    }
    if (points.size() < 3)		//not enough points to compute area
        return glm::dvec3(0.0,0.0,0.0);
    else
    {
        points.push_back(points[0]);
        double areaX(0), areaY(0), areaZ(0);
        for (int i = 0; i < (int)points.size()-1; i++)
        {
            areaX += 0.5*(points[i].y * points[i+1].z - points[i+1].y * points[i].z);
            areaY += 0.5*(points[i].x * points[i+1].z - points[i+1].x * points[i].z);
            areaZ += 0.5*(points[i+1].x * points[i].y - points[i+1].y * points[i].x);
        }
        return glm::dvec3(abs(areaX), abs(areaY), abs(areaZ));
    }

}

std::vector<double> TlScanOverseer::getBeamStandardList()
{
    return { 74.8, 88.0, 90.0, 94.3, 106.0,
            109.0, 113.7, 124.5, 128.0, 133.1,
            143.0, 147.0, 152.6, 161.5, 166.0,
            172.0, 180.0, 185.0, 191.5, 199.0,
            204.0, 210.8, 218.0, 223.0, 230.2,
            237.5, 242.5, 257.0, 259.8, 262.0,
            276.0, 281.0, 289.3, 294.5, 299.5,
            313.5, 318.5, 332.5, 337.5, 347.3,
            371.0, 376.0, 386.5, 419.0, 424.0,
            435.4, 467.0, 472.0, 484.0, 516.0,
            521.0, 532.8, 565.0, 570.0, 581.0,
            614.0, 619.0, 663.0, 668.0, 762.0,
            767.0, 860.0, 865.0, 959.0, 964.0
    };
}

double TlScanOverseer::computeClosestStandardBeam(const double& height)
{
    std::vector<double> standard = getBeamStandardList();
    int size = (int)standard.size();
    if (1000*height < standard[0])
        return standard[0]/1000;
    for (int i = 1; i < size; i++)
    {
        if (1000 * height > standard[i])
            continue;
        // here height is between standard[i-1] and standard[i]
        else if (standard[i] - 1000 * height > 1000 * height - standard[i - 1])
            return standard[i - 1]/1000;
        else return standard[i]/1000;
    }
    return standard[size - 1]/1000; //only happens if height is > than every standard
}

bool TlScanOverseer::fitSlab(const glm::dvec3& seedPoint,glm::dvec3& boxCenter, glm::dvec3& boxDirectionX, glm::dvec3& boxDirectionY, glm::dvec3& boxDirectionZ, const ClippingAssembly& clippingAssembly)
{
    std::vector<std::vector<glm::dvec3>> points;
    double searchRadius = 0.2;
    int numberOfBuckets(5);
    std::vector<glm::dvec3> temp;

    for (int i = 0; i < numberOfBuckets; i++)
    {
        points.push_back(temp);
    }
    findNeighborsBuckets(seedPoint, searchRadius, points, numberOfBuckets, clippingAssembly);

    

    int maxNumberOfPoints(50000);
    std::vector<glm::dvec3> ballPoints,planePoints1,planePoints2,planePoints3;
    glm::dvec3 normal1, normal2, normal3;
    int planesFound(0);

    for (int tryOut = 0; tryOut < points.size(); tryOut++)
    {
        std::vector<glm::dvec3> totalPoints;
        double minCenterMove(1);

        OctreeRayTracing::samplePointsUpToBucket(points, (int)points.size() - tryOut, totalPoints);


        if (totalPoints.size() > maxNumberOfPoints)
        {
            std::vector<glm::dvec3> otherTemp;
            for (int loop = 0; loop < maxNumberOfPoints; loop++)
            {
                int v1 = rand() % maxNumberOfPoints;
                int v2 = rand() % (totalPoints.size() / (int)maxNumberOfPoints);
                int index = (v1 + maxNumberOfPoints * v2) % totalPoints.size();

                otherTemp.push_back(totalPoints[index]);			//insert stuff to pick points at random instead
            }

            ballPoints = otherTemp;
        }
        else { ballPoints = totalPoints; }

        if ((int)ballPoints.size() < 4)
            continue;
        int maxIterations(1000);
        std::vector<double> plane1, plane2, plane3;
        for (int i = 0; i < maxIterations; i++)
        {
            //select 3 random points
            std::vector<glm::dvec3> testPoints(1, seedPoint);
            if (ballPoints.size() > 1000)
            {
                int v1 = rand() % 10000;
                int v2 = rand() % (ballPoints.size() / (int)1000);
                int index = (v1 + 10000 * v2) % ballPoints.size();			//if smallBall.size is large, one random index is not enough
                testPoints.push_back(ballPoints[index]);
                v1 = rand() % 10000;
                v2 = rand() % (ballPoints.size() / (int)1000);
                index = (v1 + 10000 * v2) % ballPoints.size();
                testPoints.push_back(ballPoints[index]);
                v1 = rand() % 10000;
                v2 = rand() % (ballPoints.size() / (int)1000);
                index = (v1 + 10000 * v2) % ballPoints.size();
                testPoints.push_back(ballPoints[index]);
            }
            else {
                int v1 = rand() % ((int)ballPoints.size());
                testPoints.push_back(ballPoints[v1]);						//if smallBall.size is small, we use only one random index
                v1 = rand() % ((int)ballPoints.size());
                testPoints.push_back(ballPoints[v1]);
                v1 = rand() % ((int)ballPoints.size());
                testPoints.push_back(ballPoints[v1]);
            }
            if (planesFound == 0)
            {
                if (OctreeRayTracing::fitPlane(testPoints, plane1))
                {
                    if ((OctreeRayTracing::countPointsNearPlane(ballPoints, plane1, 0.003) * 5.0) > ballPoints.size())
                    {
                        planePoints1 = OctreeRayTracing::listPointsNearPlane(ballPoints, plane1, 0.003);
                        OctreeRayTracing::fitPlane(planePoints1, plane1);
                        planesFound = 1;
                        normal1 = glm::dvec3(plane1[0], plane1[1], plane1[2]);
                    }
                }
            }
            if (planesFound == 1)
            {
                if (OctreeRayTracing::fitPlane(testPoints, plane2))
                {
                    normal2 = glm::dvec3(plane2[0], plane2[1], plane2[2]);
                    if (abs(glm::dot(normal1, normal2)) > 0.1)
                        continue;
                    if ((OctreeRayTracing::countPointsNearPlane(ballPoints, plane2, 0.003) * 5.0) > ballPoints.size())
                    {
                        planePoints2 = OctreeRayTracing::listPointsNearPlane(ballPoints, plane2, 0.003);
                        OctreeRayTracing::fitPlane(planePoints2, plane2);
                        planesFound = 2;
                    }
                }
            }
            if (planesFound == 2)
            {
                if (OctreeRayTracing::fitPlane(testPoints, plane3))
                {
                    normal3 = glm::dvec3(plane3[0], plane3[1], plane3[2]);
                    if (std::max(abs(glm::dot(normal1, normal3)),abs(glm::dot(normal2,normal3))) > 0.1)
                        continue;
                    if ((OctreeRayTracing::countPointsNearPlane(ballPoints, plane3, 0.003) * 5.0) > ballPoints.size())
                    {
                        planePoints3 = OctreeRayTracing::listPointsNearPlane(ballPoints, plane3, 0.003);
                        OctreeRayTracing::fitPlane(planePoints3, plane3);
                        planesFound = 3;
                    }
                }
            }
            if (planesFound == 3)
                break;
        }
        if (planesFound < 3)
            return false;
        else
        {
            //set Box parameters
            glm::dvec3 centerOfMass1=OctreeRayTracing::computeCenterOfMass(planePoints1);
            glm::dvec3 centerOfMass2 = OctreeRayTracing::computeCenterOfMass(planePoints2);
            glm::dvec3 centerOfMass3 = OctreeRayTracing::computeCenterOfMass(planePoints3);
            glm::dvec3 corner = computeCorner(plane1, plane2, plane3);

            return true;
        }
    }
    return true;
}

glm::dvec3 TlScanOverseer::computeCorner(const std::vector<double>& plane1, const std::vector<double>& plane2, const std::vector<double>& plane3)
{
    glm::dmat3 A;
    A[0][0] = plane1[0];
    A[0][1] = plane1[1];
    A[0][2] = plane1[2];

    A[1][0] = plane2[0];
    A[1][1] = plane2[1];
    A[1][2] = plane2[2];

    A[2][0] = plane3[0];
    A[2][1] = plane3[1];
    A[2][2] = plane3[2];
    glm::dvec3 b(-plane1[3], -plane2[3], -plane3[3]);
    A = glm::transpose(A);
    glm::dmat3 Ainverse = glm::inverse(A);
    glm::dvec3 result = Ainverse * b;
    return result;

}

bool TlScanOverseer::fitSlabTest(const glm::dvec3& seedPoint, std::vector<std::vector<double>>& planes, std::vector<glm::dvec3>& centers, glm::dvec3& corner, const ClippingAssembly& clippingAssembly, glm::dvec3& scale, glm::dvec3& normal1, glm::dvec3& normal2, glm::dvec3& normal3, glm::dvec3& trueCenter, const bool& extend)
{
    std::vector<std::vector<glm::dvec3>> points;
    double searchRadius = 0.15;
    int numberOfBuckets(5);
    std::vector<glm::dvec3> temp;

    for (int i = 0; i < numberOfBuckets; i++)
    {
        points.push_back(temp);
    }
    findNeighborsBuckets(seedPoint, searchRadius, points, numberOfBuckets, clippingAssembly);

    Logger::log(LoggerMode::rayTracingLog) << "here1" << Logger::endl;


    int maxNumberOfPoints(50000);
    std::vector<glm::dvec3> ballPoints, planePoints1, planePoints2, planePoints3;
    int planesFound(0);

    for (int tryOut = 0; tryOut < points.size(); tryOut++)
    {
        std::vector<glm::dvec3> totalPoints;
        double minCenterMove(1);

        OctreeRayTracing::samplePointsUpToBucket(points, (int)points.size() - tryOut, totalPoints);


        if (totalPoints.size() > maxNumberOfPoints)
        {
            std::vector<glm::dvec3> otherTemp;
            for (int loop = 0; loop < maxNumberOfPoints; loop++)
            {
                int v1 = rand() % maxNumberOfPoints;
                int v2 = rand() % (totalPoints.size() / (int)maxNumberOfPoints);
                int index = (v1 + maxNumberOfPoints * v2) % totalPoints.size();

                otherTemp.push_back(totalPoints[index]);			//insert stuff to pick points at random instead
            }

            ballPoints = otherTemp;
        }
        else { ballPoints = totalPoints; }
        Logger::log(LoggerMode::rayTracingLog) << "here2" << Logger::endl;

        if ((int)ballPoints.size() < 4)
            continue;
        int maxIterations(200);
        std::vector<double> plane1, plane2, plane3;

        for (int i = 0; i < maxIterations; i++)
        {
            /* //select 3 random points
            std::vector<glm::dvec3> testPoints(1, seedPoint);
            if (ballPoints.size() > 1000)
            {
                int v1 = rand() % 10000;
                int v2 = rand() % (ballPoints.size() / (int)1000);
                int index = (v1 + 10000 * v2) % ballPoints.size();			//if smallBall.size is large, one random index is not enough
                testPoints.push_back(ballPoints[index]);
                v1 = rand() % 10000;
                v2 = rand() % (ballPoints.size() / (int)1000);
                index = (v1 + 10000 * v2) % ballPoints.size();
                testPoints.push_back(ballPoints[index]);
                v1 = rand() % 10000;
                v2 = rand() % (ballPoints.size() / (int)1000);
                index = (v1 + 10000 * v2) % ballPoints.size();
                testPoints.push_back(ballPoints[index]);
            }
            else {
                int v1 = rand() % ((int)ballPoints.size());
                testPoints.push_back(ballPoints[v1]);						//if smallBall.size is small, we use only one random index
                v1 = rand() % ((int)ballPoints.size());
                testPoints.push_back(ballPoints[v1]);
                v1 = rand() % ((int)ballPoints.size());
                testPoints.push_back(ballPoints[v1]);
            }*/
            glm::dvec3 testSeedPoint;
            if (ballPoints.size() == 0)
            {
                continue;
            }
            else if (ballPoints.size() > 10000)
            {
                int v1 = rand() % 10000;
                int v2 = rand() % (ballPoints.size() / (int)1000);
                int index = (v1 + 10000 * v2) % ballPoints.size();
                testSeedPoint = ballPoints[index];
            }
            else
            {
                int v1 = rand() % ballPoints.size();
                testSeedPoint = ballPoints[v1];
            }
            if (planesFound == 0)
            {
                if (fitPlaneRegionGrowing(testSeedPoint, plane1,clippingAssembly))
                {
                    normal1 = glm::dvec3(plane1[0], plane1[1], plane1[2]);

                    planePoints1 = OctreeRayTracing::listPointsNearPlane(ballPoints, plane1, 0.001);
                    if (abs(normal1[2]) > 0.2)
                        continue;
                    planesFound = 1;
                    Logger::log(LoggerMode::rayTracingLog) << "normal1 : "<<normal1[0]<<" "<<normal1[1]<<" "<<normal1[2] << Logger::endl;
                    Logger::log(LoggerMode::rayTracingLog) << "iteration : "<<i << Logger::endl;

                    //if ((OctreeRayTracing::countPointsNearPlane(ballPoints, plane1, 0.003) * 5.0) > ballPoints.size())
                    //{
                        //planePoints1 = OctreeRayTracing::listPointsNearPlane(ballPoints, plane1, 0.003);
                        //OctreeRayTracing::fitPlane(planePoints1, plane1);
                        
                    //}
                }
            }
            if (planesFound == 1)
            {
                if (fitPlaneRegionGrowing(testSeedPoint, plane2, clippingAssembly))
                {
                    normal2 = glm::dvec3(plane2[0], plane2[1], plane2[2]);
                    if ((abs(glm::dot(normal1, normal2)) > 0.1) || (abs(normal2[2]) > 0.2))
                    {
                        if ((i % 30) == 29)
                        {
                            planesFound = 0;
                        }
                        continue;
                    }
                    
                    planesFound = 2;
                    Logger::log(LoggerMode::rayTracingLog) << "normal2 : " << normal2[0] << " " << normal2[1] << " " << normal2[2] << Logger::endl;
                    Logger::log(LoggerMode::rayTracingLog) << "iteration : " << i << Logger::endl;
                    planePoints2 = OctreeRayTracing::listPointsNearPlane(ballPoints, plane2, 0.001);
                    /*if ((OctreeRayTracing::countPointsNearPlane(ballPoints, plane2, 0.003) * 8.0) > ballPoints.size())
                    {
                        planePoints2 = OctreeRayTracing::listPointsNearPlane(ballPoints, plane2, 0.003);
                        OctreeRayTracing::fitPlane(planePoints2, plane2);
                        
                    }*/
                }
            }
            /*if (planesFound == 2)
            {
                if (fitPlaneRegionGrowing(testSeedPoint, plane3, clippingAssembly))
                {
                    normal3 = glm::dvec3(plane3[0], plane3[1], plane3[2]);
                    if (std::max(abs(glm::dot(normal1, normal3)), abs(glm::dot(normal2, normal3))) > 0.3)
                        continue;
                    planesFound = 3;
                    planePoints3 = OctreeRayTracing::listPointsNearPlane(ballPoints, plane3, 0.001);
                    if ((OctreeRayTracing::countPointsNearPlane(ballPoints, plane3, 0.003) * 12.0) > ballPoints.size())
                    {
                        planePoints3 = OctreeRayTracing::listPointsNearPlane(ballPoints, plane3, 0.003);
                        OctreeRayTracing::fitPlane(planePoints3, plane3);
                        planesFound = 3;
                    }
                }
            }*/
            if (planesFound == 2)
            {
                Logger::log(LoggerMode::rayTracingLog) << "last iteration : " << i << Logger::endl;

                break;
            }
        }
        if (planesFound < 2)
            return false;
        else
        {
            //set Box parameters
            
            std::vector<glm::dvec3> centerOfMasses(0);
            centerOfMasses.push_back(OctreeRayTracing::computeCenterOfMass(planePoints1));
            centerOfMasses.push_back(OctreeRayTracing::computeCenterOfMass(planePoints2));
            //centerOfMasses.push_back(OctreeRayTracing::computeCenterOfMass(planePoints3));
            int indexZ(0);
            glm::dvec3 boxDirectionX, boxDirectionY, boxDirectionZ;
            /*if (abs(plane1[2]) < 0.7)
            {
                verticalizePlane(plane1, centerOfMasses[0]);
            }
            if (abs(plane2[2]) < 0.7)
            {
                verticalizePlane(plane2, centerOfMasses[1]);
            }
            if (abs(plane3[2]) < 0.7)
            {
                verticalizePlane(plane3, centerOfMasses[2]);
            }*/
            if (!verticalizePlane(plane1, centerOfMasses[0]))
                continue;
            if (!verticalizePlane(plane2, centerOfMasses[1]))
                continue;
            /*std::vector<glm::dvec3> boxPoints;
            std::vector<double> temp;
            std::vector<std::vector<double>> xyRange;
            temp.push_back(-0.1);
            temp.push_back(0.1);
            xyRange.push_back(temp);
            xyRange.push_back(temp);
            boxPoints=ballInBox(centerOfMasses[0], normal2, normal3, normal1, xyRange, 0.1);
            std::vector<glm::dvec3> planePoints;
            for (int j = 0; j < (int)boxPoints.size(); j++)
            {
                if (OctreeRayTracing::pointToPlaneDistance(boxPoints[j], plane1) < 0.03)
                {
                    planePoints.push_back(boxPoints[j]);
                }
            }
            OctreeRayTracing::fitPlane(planePoints, plane1);
            boxPoints = ballInBox(centerOfMasses[1], normal1, normal3, normal2, xyRange, 0.1);
            planePoints = std::vector<glm::dvec3>(0);
            for (int j = 0; j < (int)boxPoints.size(); j++)
            {
                if (OctreeRayTracing::pointToPlaneDistance(boxPoints[j], plane2) < 0.03)
                {
                    planePoints.push_back(boxPoints[j]);
                }
            }
            OctreeRayTracing::fitPlane(planePoints, plane2);
            boxPoints = ballInBox(centerOfMasses[2], normal3, normal1, normal2, xyRange, 0.1);
            planePoints = std::vector<glm::dvec3>(0);
            for (int j = 0; j < (int)boxPoints.size(); j++)
            {
                if (OctreeRayTracing::pointToPlaneDistance(boxPoints[j], plane3) < 0.03)
                {
                    planePoints.push_back(boxPoints[j]);
                }
            }
            OctreeRayTracing::fitPlane(planePoints, plane3);*/
            planes = std::vector<std::vector<double>>(0);
            planes.push_back(plane1);
            planes.push_back(plane2);
            std::vector<double> topPlane;
            topPlane.push_back(0);
            topPlane.push_back(0);
            topPlane.push_back(-1);
            topPlane.push_back(seedPoint[2]);
            plane3 = topPlane;
            planes.push_back(topPlane);
            normal1 = glm::dvec3(plane1[0], plane1[1], plane1[2]);
            normal2 = glm::dvec3(plane2[0], plane2[1], plane2[2]);
            //normal3 = glm::dvec3(plane3[0], plane3[1], plane3[2]);
            normal3 = glm::dvec3(0.0, 0.0, -1.0);
            corner = computeCorner(plane1, plane2, plane3);
            //make normals point towards interior of the box
            if (glm::dot(normal1, centerOfMasses[1] - corner) < 0)
            {
                normal1 = -normal1;
            }
            if (glm::dot(normal2, centerOfMasses[0] - corner) < 0)
            {
                normal2 = -normal2;
            }
            /*if (glm::dot(normal3, centerOfMasses[0] - corner) < 0)
            {
                normal3 = -normal3;
            }*/
            //double maxZ(centerOfMasses[0][2]);
            double maxZ = seedPoint[2];
            /*double z1(centerOfMasses[1][2]), z2(centerOfMasses[2][2]);
            if (z1 > maxZ)
            {
                maxZ = centerOfMasses[1][2];
                indexZ = 1;
            }
            if (z2 > maxZ)
            {
                maxZ = centerOfMasses[2][2];
                indexZ = 2;
            }*/
            /*if (indexZ == 0)
            {
                boxDirectionZ = normal1;
                if (abs(plane2[0]) > abs(plane3[0]))
                {
                    boxDirectionX = normal2;
                    boxDirectionY = normal3;
                }
                else
                {
                    boxDirectionY = normal2;
                    boxDirectionX = normal3;
                }
            }
            if (indexZ == 1)
            {
                boxDirectionZ = normal2;
                if (abs(plane1[0]) > abs(plane3[0]))
                {
                    boxDirectionX = normal1;
                    boxDirectionY = normal3;
                }
                else
                {
                    boxDirectionY = normal1;
                    boxDirectionX = normal3;
                }
            }
            if (indexZ == 2)
            {
                boxDirectionZ = normal3;
                if (abs(plane2[0]) > abs(plane1[0]))
                {
                    boxDirectionX = normal2;
                    boxDirectionY = normal1;
                }
                else
                {
                    boxDirectionY = normal2;
                    boxDirectionX = normal1;
                }
            }*/
            boxDirectionZ = glm::dvec3(0.0, 0.0, -1.0);
            if (abs(plane1[0]) > abs(plane2[0]))
            {
                boxDirectionX = normal1;
                boxDirectionY = normal2;
            }
            else
            {
                std::vector<double> tempPlane;
                tempPlane = plane1;
                plane1 = plane2;
                plane2 = tempPlane;
                boxDirectionX = normal2;
                boxDirectionY = normal1;
            }
            normal1 = boxDirectionX;
            normal2 = boxDirectionY;
            normal3 = boxDirectionZ;
            
            double scale1, scale2;
            boxDirectionX = normal1;
            boxDirectionY = normal2;
            boxDirectionZ = normal3;
            extendSlab2(plane1, corner, -boxDirectionZ, boxDirectionY, boxDirectionX, scale1, clippingAssembly, 0);
            extendSlab2(plane2, corner, -boxDirectionZ, boxDirectionX, boxDirectionY, scale2, clippingAssembly, 0);
            plane3[3] = corner[2] + std::max(scale1, scale2);
            corner = computeCorner(plane1, plane2, plane3);

            //check if there is a top Plane

            std::vector<glm::dvec3> neighborhood;
            findNeighbors(corner + 0.1 * boxDirectionX + 0.1 * boxDirectionY-0.02*boxDirectionZ, 0.03, neighborhood, clippingAssembly);
            if ((int)neighborhood.size() > 30)
            {
                double meanZ = 0;
                for (int k = 0; k < (int)neighborhood.size(); k++)
                {
                    meanZ += neighborhood[k][2];
                }
                meanZ /= (double)neighborhood.size();
                double meanError(0);
                for (int k = 0; k < (int)neighborhood.size(); k++)
                {
                    meanError += abs(meanZ - neighborhood[k][2]);
                }
                meanError /= (double)neighborhood.size();
                if (meanError < 0.0008)
                {
                    plane3[3] = meanZ;
                    corner=computeCorner(plane1, plane2, plane3);
                }
            }


            if (!extend)
            {
                trueCenter = corner + 0.25 * (normal1 + normal2 + normal3);
                centers.push_back(corner + 0.25 * (normal2 + normal3));
                centers.push_back(corner + 0.25 * (normal1 + normal3));
                centers.push_back(corner + 0.25 * (normal1 + normal2));
                scale[0] = 0.25;
                scale[1] = 0.25;
                scale[2] = 0.25;
            }
            else {
                double scale12, scale13, scale23, scale21, scale31, scale32;
                /*extendSlab(plane1, corner, boxDirectionX, boxDirectionY, boxDirectionZ, scale12, clippingAssembly);
                extendSlab(plane1, corner, boxDirectionX, boxDirectionZ, boxDirectionY, scale13, clippingAssembly);
                extendSlab(plane2, corner, boxDirectionY, boxDirectionX, boxDirectionZ, scale21, clippingAssembly);
                extendSlab(plane2, corner, boxDirectionY, boxDirectionZ, boxDirectionX, scale23, clippingAssembly);
                extendSlab(plane3, corner, boxDirectionZ, boxDirectionX, boxDirectionY, scale31, clippingAssembly);
                extendSlab(plane3, corner, boxDirectionZ, boxDirectionY, boxDirectionX, scale32, clippingAssembly);*/
                double heightStart(0.03);
                double heightStartZ = std::max(0.0, glm::dot(boxDirectionZ, seedPoint - corner));
                extendSlab2(plane2, corner, boxDirectionX, boxDirectionZ, boxDirectionY, scale12, clippingAssembly, heightStart);
                extendSlab2(plane3, corner, boxDirectionX, boxDirectionY, boxDirectionZ, scale13, clippingAssembly, heightStart);
                extendSlab2(plane1, corner, boxDirectionY, boxDirectionZ, boxDirectionX, scale21, clippingAssembly, heightStart);
                extendSlab2(plane3, corner, boxDirectionY, boxDirectionX, boxDirectionZ, scale23, clippingAssembly, heightStart);
                extendSlab2(plane1, corner, boxDirectionZ, boxDirectionY, boxDirectionX, scale31, clippingAssembly, heightStartZ);
                extendSlab2(plane2, corner, boxDirectionZ, boxDirectionX, boxDirectionY, scale32, clippingAssembly, heightStartZ);
                /*scale[0] = std::max(scale12, scale13);
                scale[1] = std::max(scale13, scale31);
                scale[2] = std::max(scale12, scale21);*/
                scale[0] = 0.5 * std::max(scale12, scale13);
                scale[1] = 0.5 * std::max(scale21, scale23);
                scale[2] = 0.5 * std::max(scale31, scale32);
                centers.push_back(corner + scale[0] * (boxDirectionY + boxDirectionZ));
                centers.push_back(corner + scale[1] * (boxDirectionX + boxDirectionZ));
                centers.push_back(corner + scale[2] * (boxDirectionX + boxDirectionY));
                trueCenter = corner + (scale[0] * boxDirectionX + scale[1] * boxDirectionY + scale[2] * boxDirectionZ);
            }
            glm::dvec3 offset = corner - trueCenter;
            return true;
        }
    }
    return false;
}
bool TlScanOverseer::extendSlab(const std::vector<double>& plane, const glm::dvec3& corner, const glm::dvec3& normal, const glm::dvec3& dir, const glm::dvec3& orthoDir, double& scale, const ClippingAssembly& clippingAssembly)
{
    scale = 0.02;
    std::vector<double> temp;
    std::vector<std::vector<double>> xyRange;
    temp.push_back(0.01);
    temp.push_back(0.05);
    xyRange.push_back(temp);
    temp[0] = -0.05;
    temp[1] = 0.05;
    xyRange.push_back(temp);
    std::vector<glm::dvec3> boxPoints = ballInBox(corner, dir, orthoDir, normal, xyRange,1);
    double step = 0.02;
    int numberOfSteps = (int)(1.0 / step);
    for (int i = 5; i < numberOfSteps; i++)
    {
        std::vector<glm::dvec3> currPoints;
        for (int j = 0; j < (int)boxPoints.size(); j++)
        {
            double height = computeHeight(boxPoints[j], normal, corner);
            if ((height > ((i - 1) * step)) && (height < ((i + 1) * step)))
            {
                currPoints.push_back(boxPoints[j]);
            }
        }
        int count = OctreeRayTracing::countPointsNearPlane(currPoints, plane, 0.01);
        Logger::log(LoggerMode::rayTracingLog) << "count : " << count << Logger::endl;

        if (count < 10)
            return true;
        double ratio = (double)count / (double)currPoints.size();
        Logger::log(LoggerMode::rayTracingLog) << "ratio : "<<ratio <<", step : "<<i << Logger::endl;

        if ((10.0 * count) > (int)currPoints.size())
        {
            scale = i * step;
        }
        else
            return true;
    }
    return true;
    
}

bool TlScanOverseer::extendSlab2(const std::vector<double>& plane, const glm::dvec3& corner, const glm::dvec3& normal, const glm::dvec3& dir, const glm::dvec3& orthoDir, double& scale, const ClippingAssembly& clippingAssembly, const double& heightStart)
{
    scale = heightStart;
    double step = 0.005;
    double bigStep = 0.2;
    int failCount(0);
    int endBigStepIndex(0);

    for (int i = 0; i < 10; i++)
    {
        glm::dvec3 seedPoint;
        if (failCount == 0)
        {
            seedPoint = corner + 0.03 * dir + (heightStart+(double)i * bigStep) * normal;
        }
        else
        {
            seedPoint = corner + 0.1 * dir + (heightStart+(double)i * bigStep) * normal;

        }
        std::vector<double> tempPlane;
        if (fitPlaneRegionGrowing(seedPoint, tempPlane, clippingAssembly))
        {
            glm::dvec3 tempNormal;
            tempNormal[0] = tempPlane[0];
            tempNormal[1] = tempPlane[1];
            tempNormal[2] = tempPlane[2];
            double testPointPlaneDist = OctreeRayTracing::pointToPlaneDistance(corner, tempPlane);
            glm::dvec3 planeNormal(glm::dvec3(plane[0], plane[1], plane[2]));
            double testPlanePlaneDist = abs(glm::dot(planeNormal, tempNormal));
            if ((testPointPlaneDist < 0.02) && (testPlanePlaneDist > 0.99))
            {
                failCount = 0;
                scale = (double)i * bigStep+heightStart;
                continue;
            }
            else
                 
            {
                if (failCount == 0)
                {
                    failCount = 1;
                    i = i - 1;
                    Logger::log(LoggerMode::rayTracingLog) << "1B bad plane found" << Logger::endl;
                }
                else
                {
                    Logger::log(LoggerMode::rayTracingLog) << "2B bad plane found" << Logger::endl;
                    endBigStepIndex = i-1;
                    break;
                }
            }
        }
        else
        {
            if (failCount == 0)
            {
                failCount = 1;
                i = i - 1;
                Logger::log(LoggerMode::rayTracingLog) << "1B no plane found" << Logger::endl;
                continue;
            }
            else
            {
                Logger::log(LoggerMode::rayTracingLog) << "2B no plane found" << Logger::endl;
                endBigStepIndex = i - 1;
                break;
            }
        }
    }
    failCount = 0;
    for (int i = 0; i < (int)(2*bigStep/step); i++)
    {
        glm::dvec3 seedPoint;
        if (failCount == 0)
        {
            seedPoint = corner + 0.03 * dir + ((double)endBigStepIndex * bigStep + (double)i*step+heightStart)* normal;
        }
        else
        {
            seedPoint = corner + 0.1 * dir + ((double)endBigStepIndex * bigStep + (double)i * step+heightStart) * normal;

        }
        std::vector<double> tempPlane;
        if (fitPlaneRegionGrowing(seedPoint, tempPlane, clippingAssembly))
        {
            glm::dvec3 tempNormal;
            tempNormal[0] = tempPlane[0];
            tempNormal[1] = tempPlane[1];
            tempNormal[2] = tempPlane[2];
            double testPointPlaneDist = OctreeRayTracing::pointToPlaneDistance(corner, tempPlane);
            glm::dvec3 planeNormal(glm::dvec3(plane[0], plane[1], plane[2]));
            double testPlanePlaneDist = abs(glm::dot(planeNormal, tempNormal));
            if ((testPointPlaneDist < 0.02) && (testPlanePlaneDist > 0.99))
            {
                failCount = 0;
                scale = (double)endBigStepIndex * bigStep + (double)i * step+heightStart;
                continue;
            }
            else

            {
                if (failCount == 0)
                {
                    failCount = 1;
                    i = i - 1;
                    Logger::log(LoggerMode::rayTracingLog) << "1s bad plane found : i=" << i << Logger::endl;
                }
                else
                {
                    scale = std::max(scale, 0.001);
                    Logger::log(LoggerMode::rayTracingLog) << "2s bad plane found : i=" << i << Logger::endl;
                    return true;
                }
            }
        }
        else
        {
            if (failCount == 0)
            {
                failCount = 1;
                i = i - 1;
                Logger::log(LoggerMode::rayTracingLog) << "1s no plane found : i=" << i << Logger::endl;
                continue;
            }
            else
            {
                Logger::log(LoggerMode::rayTracingLog) << "2s no plane found : i=" << i << Logger::endl;
                scale = std::max(scale, 0.001);
                return true;
            }
        }
    }
    scale = std::max(scale, 0.001);
    return true;
}


bool TlScanOverseer::fitSlab3Clicks(const std::vector<glm::dvec3>& seedPoints, glm::dvec3& boxCenter, glm::dvec3& boxDirectionX, glm::dvec3& boxDirectionY, glm::dvec3& boxDirectionZ, const ClippingAssembly& clippingAssembly, std::vector<std::vector<double>>& planes, std::vector<glm::dvec3>& centers, glm::dvec3& scale, const bool& extend)
{
    int indexZ(0);
    double maxZ(seedPoints[0][2]);
    double z1(seedPoints[1][2]), z2(seedPoints[2][2]);
    if (z1 > maxZ)
    {
        maxZ = seedPoints[1][2];
        indexZ = 1;
    }
    if (z2 > maxZ)
    {
        maxZ = seedPoints[2][2];
        indexZ = 2;
    }

    std::vector<double> plane1(4,0), plane2(4,0), plane3(4,0);

    if (indexZ!=0)
        fitPlaneRegionGrowing(seedPoints[0], plane1, clippingAssembly);
    if(indexZ!=1)
        fitPlaneRegionGrowing(seedPoints[1], plane2, clippingAssembly);
    if (indexZ!=2)
        fitPlaneRegionGrowing(seedPoints[2], plane3, clippingAssembly);
    std::vector<double> topPlane;
    topPlane.push_back(0);
    topPlane.push_back(0);
    topPlane.push_back(1);
    topPlane.push_back(-maxZ);
    if (indexZ == 0)
        plane1 = topPlane;
    if (indexZ == 1)
        plane2 = topPlane;
    if (indexZ == 2)
        plane3 = topPlane;

    if (indexZ != 0)
    {
        if (!verticalizePlane(plane1, seedPoints[0]))
            return false;
    }
    if (indexZ != 1)
    {
        if (!verticalizePlane(plane2, seedPoints[1]))
            return false;
    }
    if (indexZ != 2)
    {
        if (!verticalizePlane(plane3, seedPoints[2]))
            return false;
    }
    /*std::vector<double> tempPlane;
    int totalSuccess(0);
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            for (int k = 0; k < 10; k++)
            {
                if (fitPlaneRadius(seedPoints[0] + glm::dvec3(0.001 * i, 0.001 * j, 0.001 * k), tempPlane, clippingAssembly, 0.015))
                {
                    totalSuccess++;
                    plane1[0] += tempPlane[0];
                    plane1[1] += tempPlane[1];
                    plane1[2] += tempPlane[2];
                    plane1[3] += tempPlane[3];
                }

            }
        }
    }
    plane1[0] /= (double)totalSuccess;
    plane1[1] /= (double)totalSuccess;
    plane1[2] /= (double)totalSuccess;
    plane1[3] /= (double)totalSuccess;
    totalSuccess = 0;
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            for (int k = 0; k < 10; k++)
            {
                if (fitPlaneRadius(seedPoints[1] + glm::dvec3(0.001 * i, 0.001 * j, 0.001 * k), tempPlane, clippingAssembly, 0.015))
                {
                    totalSuccess++;
                    plane2[0] += tempPlane[0];
                    plane2[1] += tempPlane[1];
                    plane2[2] += tempPlane[2];
                    plane2[3] += tempPlane[3];
                }

            }
        }
    }
    plane2[0] /= (double)totalSuccess;
    plane2[1] /= (double)totalSuccess;
    plane2[2] /= (double)totalSuccess;
    plane2[3] /= (double)totalSuccess;
    totalSuccess = 0; 
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            for (int k = 0; k < 10; k++)
            {
                if (fitPlaneRadius(seedPoints[2] + glm::dvec3(0.001 * i, 0.001 * j, 0.001 * k), tempPlane, clippingAssembly, 0.015))
                {
                    totalSuccess++;
                    plane3[0] += tempPlane[0];
                    plane3[1] += tempPlane[1];
                    plane3[2] += tempPlane[2];
                    plane3[3] += tempPlane[3];
                }

            }
        }
    }
    plane3[0] /= (double)totalSuccess;
    plane3[1] /= (double)totalSuccess;
    plane3[2] /= (double)totalSuccess;
    plane3[3] /= (double)totalSuccess;*/
    
    /*fitPlaneRadius(seedPoints[0], plane1, clippingAssembly, 0.012);
    fitPlaneRadius(seedPoints[1], plane2, clippingAssembly, 0.012);
    fitPlaneRadius(seedPoints[2], plane3, clippingAssembly, 0.012);*/
    planes.push_back(plane1);
    planes.push_back(plane2);
    planes.push_back(plane3);
    glm::dvec3 corner = computeCorner(plane1, plane2, plane3);
    glm::dvec3 normal1, normal2, normal3;
    normal1 = glm::dvec3(plane1[0], plane1[1], plane1[2]);
    normal2 = glm::dvec3(plane2[0], plane2[1], plane2[2]);
    normal3 = glm::dvec3(plane3[0], plane3[1], plane3[2]);
    glm::dvec3 someInsidePoint = (seedPoints[0] + seedPoints[1] + seedPoints[2]) ;
    someInsidePoint /= 3;
    //make normals point towards interior of the box
    if (glm::dot(normal1, someInsidePoint - corner) < 0)
    {
        normal1 = -normal1;
    }
    if (glm::dot(normal2, someInsidePoint - corner) < 0)
    {
        normal2 = -normal2;
    }
    if (glm::dot(normal3, someInsidePoint - corner) < 0)
    {
        normal3 = -normal3;
    }
    if (indexZ == 0)
    {
        boxDirectionZ = normal1;
        if (abs(plane2[0]) > abs(plane3[0]))
        {
            boxDirectionX = normal2;
            boxDirectionY = normal3;
            std::vector<double> tempPlane1(plane1), tempPlane2(plane2),tempPlane3(plane3);
            
            plane3 = tempPlane1;
            plane1 = tempPlane2;
            plane2 = tempPlane3;
        }
        else
        {
            boxDirectionY = normal2;
            boxDirectionX = normal3;
            std::vector<double> tempPlane1(plane1), tempPlane2(plane2), tempPlane3(plane3);

            plane3 = tempPlane1;
            plane1 = tempPlane3;
            plane2 = tempPlane2;
        }
    }
    if (indexZ == 1)
    {
        boxDirectionZ = normal2;
        if (abs(plane1[0]) > abs(plane3[0]))
        {
            boxDirectionX = normal1;
            boxDirectionY = normal3;
            std::vector<double> tempPlane1(plane1), tempPlane2(plane2), tempPlane3(plane3);

            plane3 = tempPlane2;
            plane1 = tempPlane1;
            plane2 = tempPlane3;
        }
        else
        {
            boxDirectionY = normal1;
            boxDirectionX = normal3;
            std::vector<double> tempPlane1(plane1), tempPlane2(plane2), tempPlane3(plane3);

            plane3 = tempPlane2;
            plane1 = tempPlane3;
            plane2 = tempPlane1;
        }
    }
    if (indexZ == 2)
    {
        boxDirectionZ = normal3;
        if (abs(plane2[0]) > abs(plane1[0]))
        {
            boxDirectionX = normal2;
            boxDirectionY = normal1;
            std::vector<double> tempPlane1(plane1), tempPlane2(plane2), tempPlane3(plane3);

            plane3 = tempPlane3;
            plane1 = tempPlane2;
            plane2 = tempPlane1;
        }
        else
        {
            boxDirectionY = normal2;
            boxDirectionX = normal1;
        }
    }
    if (!extend)
    {
        boxCenter = corner + 0.25 * (normal1 + normal2 + normal3);
        centers.push_back(corner + 0.25 * (normal2 + normal3));
        centers.push_back(corner + 0.25 * (normal1 + normal3));
        centers.push_back(corner + 0.25 * (normal1 + normal2));
        scale[0] = 0.25;
        scale[1] = 0.25;
        scale[2] = 0.25;
    }
    else {
        double scale12, scale13, scale23, scale21, scale31, scale32;
        /*extendSlab(plane1, corner, boxDirectionX, boxDirectionY, boxDirectionZ, scale12, clippingAssembly);
        extendSlab(plane1, corner, boxDirectionX, boxDirectionZ, boxDirectionY, scale13, clippingAssembly);
        extendSlab(plane2, corner, boxDirectionY, boxDirectionX, boxDirectionZ, scale21, clippingAssembly);
        extendSlab(plane2, corner, boxDirectionY, boxDirectionZ, boxDirectionX, scale23, clippingAssembly);
        extendSlab(plane3, corner, boxDirectionZ, boxDirectionX, boxDirectionY, scale31, clippingAssembly);
        extendSlab(plane3, corner, boxDirectionZ, boxDirectionY, boxDirectionX, scale32, clippingAssembly);*/
        double heightStart1, heightStart2, heightStart3;
        heightStart1 = std::max(glm::dot(boxDirectionX, seedPoints[0] - corner), std::max(glm::dot(boxDirectionX, seedPoints[1] - corner), glm::dot(boxDirectionX, seedPoints[2] - corner)));
        heightStart2 = std::max(glm::dot(boxDirectionY, seedPoints[0] - corner), std::max(glm::dot(boxDirectionY, seedPoints[1] - corner), glm::dot(boxDirectionY, seedPoints[2] - corner)));
        heightStart3 = std::max(glm::dot(boxDirectionZ, seedPoints[0] - corner), std::max(glm::dot(boxDirectionZ, seedPoints[1] - corner), glm::dot(boxDirectionZ, seedPoints[2] - corner)));

        extendSlab2(plane2, corner, boxDirectionX, boxDirectionZ, boxDirectionY, scale12, clippingAssembly, heightStart1);
        extendSlab2(plane3, corner, boxDirectionX, boxDirectionY, boxDirectionZ, scale13, clippingAssembly, heightStart1);
        extendSlab2(plane1, corner, boxDirectionY, boxDirectionZ, boxDirectionX, scale21, clippingAssembly, heightStart2);
        extendSlab2(plane3, corner, boxDirectionY, boxDirectionX, boxDirectionZ, scale23, clippingAssembly, heightStart2);
        extendSlab2(plane1, corner, boxDirectionZ, boxDirectionY, boxDirectionX, scale31, clippingAssembly, heightStart3);
        extendSlab2(plane2, corner, boxDirectionZ, boxDirectionX, boxDirectionY, scale32, clippingAssembly, heightStart3);
        /*scale[0] = std::max(scale12, scale13);
        scale[1] = std::max(scale13, scale31);
        scale[2] = std::max(scale12, scale21);*/
        scale[0] = 0.5*std::max(scale12, scale13);
        scale[1] = 0.5 * std::max(scale21, scale23);
        scale[2] = 0.5 * std::max(scale31, scale32);
        centers.push_back(corner + scale[0] * (boxDirectionY + boxDirectionZ));
        centers.push_back(corner + scale[1] * (boxDirectionX + boxDirectionZ));
        centers.push_back(corner + scale[2] * (boxDirectionX + boxDirectionY));
        boxCenter = corner + (scale[0] * boxDirectionX + scale[1] * boxDirectionY + scale[2] * boxDirectionZ);
    }

    
    //trueCenter = corner + scale[0] * normal1 + scale[1] * normal2 + scale[2] * normal3;
    return true;
}

bool TlScanOverseer::fitSlab2Clicks(const std::vector<glm::dvec3>& seedPoints, glm::dvec3& boxCenter, glm::dvec3& boxDirectionX, glm::dvec3& boxDirectionY, glm::dvec3& boxDirectionZ, const ClippingAssembly& clippingAssembly, std::vector<std::vector<double>>& planes, std::vector<glm::dvec3>& centers, glm::dvec3& scale, const bool& extend)
{
    double zMid;
    zMid = 0.5 * (seedPoints[0][2] + seedPoints[1][2]);

    std::vector<double> plane1(4, 0), plane2(4, 0), plane3(4, 0);

    
    std::vector<double> topPlane;
    topPlane.push_back(0);
    topPlane.push_back(0);
    topPlane.push_back(-1);
    topPlane.push_back(zMid);
    
    if (!fitPlaneRegionGrowing(seedPoints[0], plane1, clippingAssembly))
    {
        return false;
    }
    if (!fitPlaneRegionGrowing(seedPoints[1], plane2, clippingAssembly))
    {
        return false;
    }
    
    if (!verticalizePlane(plane1, seedPoints[0]))
        return false;
    if (!verticalizePlane(plane2, seedPoints[1]))
        return false;
    plane3 = topPlane;
    
    glm::dvec3 corner = computeCorner(plane1, plane2, plane3);
    glm::dvec3 normal1, normal2, normal3;
    normal1 = glm::dvec3(plane1[0], plane1[1], plane1[2]);
    normal2 = glm::dvec3(plane2[0], plane2[1], plane2[2]);
    normal3 = glm::dvec3(plane3[0], plane3[1], plane3[2]);
    glm::dvec3 someInsidePoint = (seedPoints[0] + seedPoints[1]);
    someInsidePoint /= 2;
    //make normals point towards interior of the box
    if (glm::dot(normal1, someInsidePoint - corner) < 0)
    {
        normal1 = -normal1;
    }
    if (glm::dot(normal2, someInsidePoint - corner) < 0)
    {
        normal2 = -normal2;
    }
    
    
    //swap XY directions
    bool swapXY(false);
    if (abs(normal2[0]) > abs(normal1[0]))
    {
        swapXY = true;
        std::vector<double> tempPlane = plane1;
        plane1 = plane2;
        plane2 = tempPlane;
        glm::dvec3 tempNormal = normal1;
        normal1 = normal2;
        normal2 = tempNormal;
    }

    planes.push_back(plane1);
    planes.push_back(plane2);
    planes.push_back(plane3);

    //extend to top

    double scale1, scale2;
    boxDirectionX = normal1;
    boxDirectionY = normal2;
    boxDirectionZ = normal3;
    extendSlab2(plane1, corner, -boxDirectionZ, boxDirectionY, boxDirectionX, scale1, clippingAssembly, 0);
    extendSlab2(plane2, corner, -boxDirectionZ, boxDirectionX, boxDirectionY, scale2, clippingAssembly, 0);
    plane3[3] = corner[2]+std::max(scale1, scale2);
    corner = computeCorner(plane1, plane2, plane3);

    //check if there is a top Plane

    std::vector<glm::dvec3> neighborhood;
    findNeighbors(corner + 0.1 * boxDirectionX + 0.1 * boxDirectionY-0.02*boxDirectionZ, 0.03, neighborhood, clippingAssembly);
    if ((int)neighborhood.size() > 30)
    {
        double meanZ = 0;
        for (int k = 0; k < (int)neighborhood.size(); k++)
        {
            meanZ += neighborhood[k][2];
        }
        meanZ /= (double)neighborhood.size();
        double meanError(0);
        for (int k = 0; k < (int)neighborhood.size(); k++)
        {
            meanError += abs(meanZ - neighborhood[k][2]);
        }
        meanError /= (double)neighborhood.size();
        if (meanError < 0.0008)
        {
            plane3[3] = meanZ;
            corner=computeCorner(plane1, plane2, plane3);
        }
    }
    

    if (!extend)
    {
        boxCenter = corner + 0.25 * (normal1 + normal2 + normal3);
        centers.push_back(corner + 0.25 * (normal2 + normal3));
        centers.push_back(corner + 0.25 * (normal1 + normal3));
        centers.push_back(corner + 0.25 * (normal1 + normal2));
        scale[0] = 0.25;
        scale[1] = 0.25;
        scale[2] = 0.25;
    }
    else {
        double scale12, scale13, scale23, scale21, scale31, scale32;
        /*extendSlab(plane1, corner, boxDirectionX, boxDirectionY, boxDirectionZ, scale12, clippingAssembly);
        extendSlab(plane1, corner, boxDirectionX, boxDirectionZ, boxDirectionY, scale13, clippingAssembly);
        extendSlab(plane2, corner, boxDirectionY, boxDirectionX, boxDirectionZ, scale21, clippingAssembly);
        extendSlab(plane2, corner, boxDirectionY, boxDirectionZ, boxDirectionX, scale23, clippingAssembly);
        extendSlab(plane3, corner, boxDirectionZ, boxDirectionX, boxDirectionY, scale31, clippingAssembly);
        extendSlab(plane3, corner, boxDirectionZ, boxDirectionY, boxDirectionX, scale32, clippingAssembly);*/
        double heightStart1, heightStart2, heightStart3;
        heightStart1 = std::max(0.0,std::max(glm::dot(boxDirectionX, seedPoints[0] - corner),glm::dot(boxDirectionX, seedPoints[1] - corner)));
        heightStart2 = std::max(0.0,std::max(glm::dot(boxDirectionY, seedPoints[0] - corner), glm::dot(boxDirectionY, seedPoints[1] - corner)));
        heightStart3 = std::max(0.0,std::max(glm::dot(boxDirectionZ, seedPoints[0] - corner), glm::dot(boxDirectionZ, seedPoints[1] - corner)));

        extendSlab2(plane2, corner, boxDirectionX, boxDirectionZ, boxDirectionY, scale12, clippingAssembly, heightStart1);
        extendSlab2(plane3, corner, boxDirectionX, boxDirectionY, boxDirectionZ, scale13, clippingAssembly, heightStart1);
        extendSlab2(plane1, corner, boxDirectionY, boxDirectionZ, boxDirectionX, scale21, clippingAssembly, heightStart2);
        extendSlab2(plane3, corner, boxDirectionY, boxDirectionX, boxDirectionZ, scale23, clippingAssembly, heightStart2);
        extendSlab2(plane1, corner, boxDirectionZ, boxDirectionY, boxDirectionX, scale31, clippingAssembly, heightStart3);
        extendSlab2(plane2, corner, boxDirectionZ, boxDirectionX, boxDirectionY, scale32, clippingAssembly, heightStart3);
        /*scale[0] = std::max(scale12, scale13);
        scale[1] = std::max(scale13, scale31);
        scale[2] = std::max(scale12, scale21);*/
        scale[0] = 0.5 * std::max(scale12, scale13);
        scale[1] = 0.5 * std::max(scale21, scale23);
        scale[2] = 0.5 * std::max(scale31, scale32);

        //use fitPlaneAutoextend to push it further
        /*std::vector<glm::dvec3> corners1(0), corners2(0), corners3(0);
        corners1.push_back(corner);
        corners1.push_back(corner + scale[1] * boxDirectionY);
        corners1.push_back(corner + scale[2] * boxDirectionZ);
        corners1.push_back(corner + scale[1] * boxDirectionY + scale[2] * boxDirectionZ);

        corners2.push_back(corner);
        corners2.push_back(corner + scale[0] * boxDirectionX);
        corners2.push_back(corner + scale[2] * boxDirectionZ);
        corners2.push_back(corner + scale[0] * boxDirectionX + scale[2] * boxDirectionZ);

        corners3.push_back(corner);
        corners3.push_back(corner + scale[0] * boxDirectionX);
        corners3.push_back(corner + scale[1] * boxDirectionY);
        corners3.push_back(corner + scale[0] * boxDirectionX + scale[1] * boxDirectionY);

        glm::dvec3 center1, center2, center3;
        center1 = 0.25 * (corners1[0] + corners1[1] + corners1[2] + corners1[3]);
        center2 = 0.25 * (corners2[0] + corners2[1] + corners2[2] + corners2[3]);
        center3 = 0.25 * (corners3[0] + corners3[1] + corners3[2] + corners3[3]);

        RectangularPlane rectPlane1(glm::dvec3(plane1[0], plane1[1], plane1[2]), corners1, center1, PlaneType::vertical);
        RectangularPlane rectPlane2(glm::dvec3(plane2[0], plane2[1], plane2[2]), corners2, center2, PlaneType::vertical);
        RectangularPlane rectPlane3(glm::dvec3(plane3[0], plane3[1], plane3[2]), corners3, center3, PlaneType::horizontal);
        fitPlaneAutoExtend(clippingAssembly, center1, rectPlane1);
        fitPlaneAutoExtend(clippingAssembly, center2, rectPlane2);
        fitPlaneAutoExtend(clippingAssembly, center3, rectPlane3);

        double l1, l2, l3, l4, l5, l6;
        l1 = glm::length(rectPlane2.m_corners[1] - rectPlane2.m_corners[0]);
        l2 = glm::length(rectPlane3.m_corners[1] - rectPlane3.m_corners[0]);
        l3 = glm::length(rectPlane1.m_corners[1] - rectPlane1.m_corners[0]);
        l4 = glm::length(rectPlane3.m_corners[2] - rectPlane3.m_corners[0]);
        l5 = glm::length(rectPlane1.m_corners[2] - rectPlane1.m_corners[0]);
        l6 = glm::length(rectPlane2.m_corners[2] - rectPlane2.m_corners[0]);
        scale[0] = std::max(scale[0], 0.5 * std::max(glm::length(rectPlane2.m_corners[1] - rectPlane2.m_corners[0]), glm::length(rectPlane3.m_corners[1] - rectPlane3.m_corners[0])));
        scale[1] = std::max(scale[1], 0.5 * std::max(glm::length(rectPlane1.m_corners[1] - rectPlane1.m_corners[0]), glm::length(rectPlane3.m_corners[2] - rectPlane3.m_corners[0])));
        scale[2] = std::max(scale[2], 0.5 * std::max(glm::length(rectPlane1.m_corners[2] - rectPlane1.m_corners[0]), glm::length(rectPlane2.m_corners[2] - rectPlane2.m_corners[0])));
        */
        centers.push_back(corner + scale[0] * (boxDirectionY + boxDirectionZ));
        centers.push_back(corner + scale[1] * (boxDirectionX + boxDirectionZ));
        centers.push_back(corner + scale[2] * (boxDirectionX + boxDirectionY));
        boxCenter = corner + (scale[0] * boxDirectionX + scale[1] * boxDirectionY + scale[2] * boxDirectionZ);
    }


    //trueCenter = corner + scale[0] * normal1 + scale[1] * normal2 + scale[2] * normal3;
    return true;
}

bool TlScanOverseer::verticalizePlane(std::vector<double>& plane, const glm::dvec3& seedPoint)
{
    plane[2] = 0;
    double normalizer = sqrt(plane[0] * plane[0] + plane[1] * plane[1]);
    plane[0] /= normalizer;
    plane[1] /= normalizer;
    if (std::isnan(plane[0]))
        return false;
    plane[3] = -plane[0] * seedPoint[0] - plane[1] * seedPoint[1];
    return true;
}

glm::dquat TlScanOverseer::computeRotation(const glm::dvec3& u, const glm::dvec3& v)
{
    //https://fr.wikipedia.org/wiki/Quaternions_et_rotation_dans_l%27espace#:~:text=Les%20quaternions%20unitaires%20fournissent%20une,probl%C3%A8me%20du%20blocage%20de%20cardan.
    glm::dvec3 w = glm::cross(u, v);
    glm::dquat result;
    glm::dquat q1 = glm::angleAxis(acos(glm::dot(u, glm::dvec3(1.0, 0.0, 0.0))), glm::dvec3(0.0,0.0,1.0));
    
    if ((1 + u[0] + v[1] + w[2]) < 0)
    {
        bool fail = true;
        return q1;
    }
    double r = 0.5 * sqrt(1 + u[0] + v[1] + w[2]);
    if (r <= std::numeric_limits<double>::epsilon())
    {
        bool fail = true;
        return q1;
    }

    glm::dquat qTest;
    qTest.w = r;
    qTest.x = (v[2] - w[1]) / (4 * r);
    qTest.y = (w[0] - u[2]) / (4 * r);
    qTest.z = (u[1] - v[0]) / (4 * r);
    result = qTest;
    
    //glm::dquat q3=

    return result;
}



bool TlScanOverseer::computePlaneExtensionTowardsLine(PolygonalPlane& polygonalPlane, const glm::dvec3& lineDirection, const glm::dvec3 linePoint)
{
    std::vector<glm::dvec3> vertices = polygonalPlane.getVertices();
    if ((int)vertices.size() == 0)
        return false;
    double heightMin(glm::dot(lineDirection, vertices[0] - linePoint)), heightMax(glm::dot(lineDirection, vertices[0] - linePoint));
    for (int i = 1; i < (int)vertices.size(); i++)
    {
        double currHeight = glm::dot(lineDirection, vertices[i] - linePoint);
        if (currHeight > heightMax)
        {
            heightMax = currHeight;
        }
        if (currHeight < heightMin)
        {
            heightMin = currHeight;
        }
    }
    polygonalPlane.addVertex(linePoint + heightMax * lineDirection);
    polygonalPlane.addVertex(linePoint + heightMin * lineDirection);
    return true;
}

PolygonalPlane::PolygonalPlane()
{}

PolygonalPlane::~PolygonalPlane()
{}

glm::dvec3 PolygonalPlane::getCenter()
{
    return m_center;
}

glm::dvec3 PolygonalPlane::getDirection()
{
    return m_referenceDirection;
}

glm::dvec3 PolygonalPlane::getNormal()
{
    return m_normal;
}

std::vector<glm::dvec3> PolygonalPlane::getVertices()
{
    return m_vertices;
}

void PolygonalPlane::setCenter(const glm::dvec3& center)
{
    m_center = center;
}

void PolygonalPlane::setDirection(const glm::dvec3& direction)
{
    m_referenceDirection = direction;
}

void PolygonalPlane::setNormal(const glm::dvec3& normal)
{
    m_normal = normal;
}

void PolygonalPlane::addVertex(const glm::dvec3& vertex)
{
    std::vector<double> plane;
    plane.push_back(m_normal[0]);
    plane.push_back(m_normal[1]);
    plane.push_back(m_normal[2]);
    plane.push_back(-m_normal[0] * m_center[0] - m_normal[1] * m_center[1] - m_normal[2] * m_center[2]);
    glm::dvec3 projectedVertex = MeasureClass::projectPointToPlane(m_center, plane);

    glm::dvec3 vertexDirection = vertex - m_center;
    vertexDirection /= glm::length(vertexDirection);
    glm::dvec3 otherDir = glm::cross(m_normal, m_referenceDirection);

    double cosAngle = glm::dot(m_referenceDirection, vertexDirection);
    bool sinSign = (glm::dot(vertexDirection, otherDir) > 0);
    for (int i = 0; i < (int)m_vertices.size(); i++)
    {
        glm::dvec3 currDirection = m_vertices[i] - m_center;
        currDirection /= glm::length(currDirection);
        double currCosAngle = glm::dot(m_referenceDirection, currDirection);
        bool currSinSign = (glm::dot(currDirection, otherDir) > 0);
        if (sinSign && currSinSign)
        {
            if (cosAngle < currCosAngle)
            {
                continue;
            }
            else
            {
                //insert just before

                std::vector<glm::dvec3>::iterator it = m_vertices.begin();
                m_vertices.insert(it + i, projectedVertex);
                return;
            }
        }
        else if (!sinSign && currSinSign)
        {
            continue;
        }
        else if (sinSign && !currSinSign)
        {
            //insert just before
            std::vector<glm::dvec3>::iterator it = m_vertices.begin();
            m_vertices.insert(it + i, projectedVertex);
            return;
        }
        else
        {
            if (cosAngle > currCosAngle)
            {
                continue;
            }
            else
            {
                //insert just before
                std::vector<glm::dvec3>::iterator it = m_vertices.begin();
                m_vertices.insert(it + i, projectedVertex);
                return;
            }
        }

        //insert at the end
        m_vertices.push_back(projectedVertex);
        return;
    }
}

bool PolygonalPlane::deleteVertex(const int& index)
{
    if (index > (int)m_vertices.size())
    {
        return false;
    }
    else
    {
        m_vertices.erase(m_vertices.begin() + index);
        return true;
    }
}

bool PolygonalPlane::isInterior(const int& index)
{
    int size = (int)m_vertices.size();
    if (index < size)
        return false;
    else
    {
        for (int i = 0; i < size; i++)
        {
            for (int j = i + 1; j < size; j++)
            {
                for (int k = j + 1; k < size; k++)
                {
                    if ((index == i) || (index == j) || (index == k))
                        continue;
                    else
                    {
                        glm::dvec3 dir1, dir2, dir3, normal;
                        normal = glm::cross(m_vertices[j] - m_vertices[i], m_vertices[k] - m_vertices[i]);
                        dir1 = glm::cross(m_vertices[j] - m_vertices[i], normal);
                        dir2 = glm::cross(m_vertices[k] - m_vertices[i], normal);
                        dir3 = glm::cross(m_vertices[k] - m_vertices[j], normal);
                        if (glm::dot(dir1, m_vertices[k] - m_vertices[i]) * glm::dot(dir1, m_vertices[index] - m_vertices[i]) < 0)
                            continue;
                        if (glm::dot(dir2, m_vertices[j] - m_vertices[i]) * glm::dot(dir2, m_vertices[index] - m_vertices[i]) < 0)
                            continue;
                        if (glm::dot(dir3, m_vertices[i] - m_vertices[j]) * glm::dot(dir1, m_vertices[index] - m_vertices[j]) < 0)
                            continue;
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

void PolygonalPlane::clearInterior()
{
    int size = (int)m_vertices.size();
    int k = 0;
    while (k < size)
    {
        if (isInterior(k))
        {
            deleteVertex(k);
            size = size - 1;
        }
        else
        {
            k += 1;
        }
    }
    return;
}

bool TlScanOverseer::createVoxelGrid(VoxelGrid& voxelGrid, const ClippingAssembly& clippingAssembly)
{
    bool result = false;
    int scanNumber(0);
    for (const WorkingScanInfo& _pair : s_workingScansTransfo)
    {
        /*ClippingAssembly localAssembly;
        if (_pair.isClippable)
        {
            localAssembly = clippingAssembly;
        }*/
        _pair.scan.setComputeTransfo(_pair.transfo.getCenter(), _pair.transfo.getOrientation());
        voxelGrid.m_numberOfScans++;
        result |= _pair.scan.updateVoxelGrid(clippingAssembly, voxelGrid,scanNumber);
        scanNumber++;
        Logger::log(LoggerMode::rayTracingLog) << "voxelGrid Populated for scan : " << scanNumber << Logger::endl;

    }
    return result;
}

int TlScanOverseer::getNumberOfScans()
{
    int result = 0;
    for (const WorkingScanInfo& _pair : s_workingScansTransfo)
    {
        result++;
    }
    return result;
}

bool TlScanOverseer::createOctreeVoxelGrid(OctreeVoxelGrid& octreeVoxelGrid)
{
    bool result = false;
    int scanNumber(0);
    for (const WorkingScanInfo& _pair : s_workingScansTransfo)
    {
        _pair.scan.setComputeTransfo(_pair.transfo.getCenter(), _pair.transfo.getOrientation());
        result |= _pair.scan.updateOctreeVoxelGrid(octreeVoxelGrid, scanNumber);
        scanNumber++;
        Logger::log(LoggerMode::rayTracingLog) << "voxelGrid Populated for scan : " << scanNumber << Logger::endl;

    }
    return result;
}

bool TlScanOverseer::displayVoxelGrid(VoxelGrid& voxelGrid)
{
    std::vector<int> occupiedVoxelsByScan(voxelGrid.m_numberOfScans,0);
    for (int k = 0; k < voxelGrid.m_numberOfScans; k++)
    {
        for (int x = 0; x < voxelGrid.m_sizeX; x++)
        {
            for (int y = 0; y < voxelGrid.m_sizeY; y++)
            {
                for (int z = 0; z < voxelGrid.m_sizeZ; z++)
                {
                    if (voxelGrid.isVoxelOccupied(x, y, z, k))
                    {
                        occupiedVoxelsByScan[k] += 1;
                        //Logger::log(LoggerMode::rayTracingLog) << "cell occupied :" << x << " " << y << " " << z << Logger::endl;

                    }
                }
            }
        }
    }
    return true;
}

bool TlScanOverseer::displayOctreeVoxelGrid(const OctreeVoxelGrid& octreeVoxelGrid)
{
    for (int k = 0; k < octreeVoxelGrid.m_numberOfScans; k++)
    {
        int numberOfOccupiedVoxels(0);
        for (int x = 0; x < (1<<octreeVoxelGrid.m_maxDepth); x++)
        {
            for (int y = 0; y < (1 << octreeVoxelGrid.m_maxDepth); y++)
            {
                for (int z = 0; z < (1 << octreeVoxelGrid.m_maxDepth); z++)
                {
                    if (octreeVoxelGrid.isVoxelOccupied(x, y, z, k))
                    {
                        numberOfOccupiedVoxels++;
                        //Logger::log(LoggerMode::rayTracingLog) << "cell occupied :" << x << " " << y << " " << z << Logger::endl;

                    }
                }
            }
        }
        Logger::log(LoggerMode::rayTracingLog) << "this scan has : " << numberOfOccupiedVoxels << " occupied voxels"<< Logger::endl;
    }
    return true;
}

bool TlScanOverseer::classifyVoxels(VoxelGrid& voxelGrid, const ClippingAssembly& clippingAssembly, std::vector<std::vector<std::vector<bool>>>& dynamicVoxels)
{
    Logger::log(LoggerMode::rayTracingLog) << "start classification" << Logger::endl;

    bool result = false;
    int scanNumber(0);
    std::vector<std::vector<std::vector<std::vector<bool>>>> dynamicPerScan;
    for (const WorkingScanInfo& _pair : s_workingScansTransfo)
    {
        std::vector<bool> temp(voxelGrid.m_sizeZ, false);
        std::vector<std::vector<bool>> temp1(voxelGrid.m_sizeY, temp);
        std::vector<std::vector<std::vector<bool>>> tempDynamic(voxelGrid.m_sizeX, temp1);
        ClippingAssembly localAssembly;
        if (_pair.isClippable)
        {
            localAssembly = clippingAssembly;
        }
        _pair.scan.setComputeTransfo(_pair.transfo.getCenter(), _pair.transfo.getOrientation());
        tls::ScanHeader info;
        _pair.scan.getInfo(info);
        Logger::log(LoggerMode::rayTracingLog) << "this scan has a total of " << info.pointCount<< " points" << Logger::endl;
        result |= _pair.scan.classifyVoxelsByScan(voxelGrid, clippingAssembly, tempDynamic, scanNumber);
        dynamicPerScan.push_back(tempDynamic);
        scanNumber++;
    }

    // classify as dynamic if at least one scan did, override already happened
    //Logger::log(LoggerMode::rayTracingLog) << "done classifying, start the merging : " << Logger::endl;

    for (int i = 0; i < voxelGrid.m_sizeX; i++)
        for (int j = 0; j < voxelGrid.m_sizeY; j++)
            for (int k = 0; k < voxelGrid.m_sizeZ; k++)			
                for (int t = 0; t < (int)dynamicPerScan.size(); t++)				
                    if (dynamicPerScan[t][i][j][k])
                        dynamicVoxels[i][j][k] = true;

    Logger::log(LoggerMode::rayTracingLog) << "done classifying" << Logger::endl;

    //fillDynamicVoxelsBucketed(voxelGrid, clippingAssembly, dynamicVoxels, dynamicVoxelsBucketed);

    //Logger::log(LoggerMode::rayTracingLog) << "dynamicVoxelsBucketed" << Logger::endl;

    return result;
}

void TlScanOverseer::copyOctreeIntoGrid(const OctreeVoxelGrid& octreeVoxelGrid, VoxelGrid& voxelGrid)
{
    //it's probably better to go through the octree instead of through the grid
    Logger::log(LoggerMode::rayTracingLog) << "start copy" << Logger::endl;
    voxelGrid.m_numberOfScans = octreeVoxelGrid.m_numberOfScans;
    for (int i=0;i<voxelGrid.m_sizeX;i++)
        for (int j=0;j<voxelGrid.m_sizeY;j++)
            for (int k=0;k<voxelGrid.m_sizeZ;k++)
                for (int t = 0; t < voxelGrid.m_numberOfScans; t++)
                {
                    if (octreeVoxelGrid.isVoxelOccupied(i,j,k,t))
                        voxelGrid.m_grid[i][j][k] += 1 << t;
                }
    Logger::log(LoggerMode::rayTracingLog) << "end copy" << Logger::endl;
    return;

}

bool TlScanOverseer::classifyOctreeVoxels(OctreeVoxelGrid& octreeVoxelGrid, std::vector<std::vector<std::vector<bool>>>& dynamicVoxels)
{
    bool result = false;
    int scanNumber(0);
    std::vector<std::vector<std::vector<std::vector<bool>>>> dynamicPerScan;
    for (const WorkingScanInfo& _pair : s_workingScansTransfo)
    {
        std::vector<bool> temp((1ull << octreeVoxelGrid.m_maxDepth), false);
        std::vector<std::vector<bool>> temp1((1ull << octreeVoxelGrid.m_maxDepth), temp);
        std::vector<std::vector<std::vector<bool>>> tempDynamic((1ull << octreeVoxelGrid.m_maxDepth), temp1);
        
        _pair.scan.setComputeTransfo(_pair.transfo.getCenter(), _pair.transfo.getOrientation());
        result |= _pair.scan.classifyOctreeVoxelsByScan(octreeVoxelGrid, tempDynamic, scanNumber);
        dynamicPerScan.push_back(tempDynamic);
        scanNumber++;
    }

    // classify as dynamic if at least one scan did, override already happened
    //Logger::log(LoggerMode::rayTracingLog) << "done classifying, start the merging : " << Logger::endl;

    for (int i = 0; i < (1ull << octreeVoxelGrid.m_maxDepth); i++)
        for (int j = 0; j < (1ull << octreeVoxelGrid.m_maxDepth); j++)
            for (int k = 0; k < (1ull << octreeVoxelGrid.m_maxDepth); k++)
                for (int t = 0; t < (int)dynamicPerScan.size(); t++)
                    if (dynamicPerScan[t][i][j][k])
                        dynamicVoxels[i][j][k] = true;

    Logger::log(LoggerMode::rayTracingLog) << "done classifying" << Logger::endl;

    return result;
}
bool TlScanOverseer::fillDynamicVoxelsBucketed(VoxelGrid& voxelGrid, const ClippingAssembly& clippingAssembly, std::vector<std::vector<std::vector<bool>>>& dynamicVoxels, std::vector<std::vector<std::vector<int>>>& dynamicVoxelsBucketed)
{
    //outdated
    // 
    //setup dynamicVoxelsBucketed
    
    dynamicVoxelsBucketed = std::vector<std::vector<std::vector<int>>>(4);
    
    //go through everything to fill the buckets
    for (int i=0;i<voxelGrid.m_sizeX;i++)
        for (int j=0;j<voxelGrid.m_sizeY;j++)
            for (int k = 0; k < voxelGrid.m_sizeZ; k++)
            {
                //count point in voxel (i,j,k) if voxel is dynamic
                if (dynamicVoxels[i][j][k])
                {
                    double voxelSize = voxelGrid.m_voxelSize;
                    std::vector<glm::dvec3> boxCorners;
                    boxCorners.push_back(glm::dvec3(voxelGrid.m_xMin + i * voxelSize, voxelGrid.m_yMin + j * voxelSize, voxelGrid.m_zMin + k * voxelSize));
                    boxCorners.push_back(glm::dvec3(voxelGrid.m_xMin + (i + 1) * voxelSize, voxelGrid.m_yMin + j * voxelSize, voxelGrid.m_zMin + k * voxelSize));
                    boxCorners.push_back(glm::dvec3(voxelGrid.m_xMin + i * voxelSize, voxelGrid.m_yMin + (j + 1) * voxelSize, voxelGrid.m_zMin + k * voxelSize));
                    boxCorners.push_back(glm::dvec3(voxelGrid.m_xMin + i * voxelSize, voxelGrid.m_yMin + j * voxelSize, voxelGrid.m_zMin + (k + 1) * voxelSize));
                    boxCorners.push_back(glm::dvec3(voxelGrid.m_xMin + (i + 1) * voxelSize, voxelGrid.m_yMin + (j + 1) * voxelSize, voxelGrid.m_zMin + k * voxelSize));
                    boxCorners.push_back(glm::dvec3(voxelGrid.m_xMin + (i + 1) * voxelSize, voxelGrid.m_yMin + j * voxelSize, voxelGrid.m_zMin + (k + 1) * voxelSize));
                    boxCorners.push_back(glm::dvec3(voxelGrid.m_xMin + i * voxelSize, voxelGrid.m_yMin + (j + 1) * voxelSize, voxelGrid.m_zMin + (k + 1) * voxelSize));
                    boxCorners.push_back(glm::dvec3(voxelGrid.m_xMin + (i + 1) * voxelSize, voxelGrid.m_yMin + (j + 1) * voxelSize, voxelGrid.m_zMin + (k + 1) * voxelSize));
                    GeometricBox gridBox(boxCorners);
                    int numberOfPoints(0);
                    for (const WorkingScanInfo& _pair : s_workingScansTransfo)
                    {
                        ClippingAssembly localAssembly;
                        if (_pair.isClippable)
                        {
                            localAssembly = clippingAssembly;
                        }
                        _pair.scan.setComputeTransfo(_pair.transfo.getCenter(), _pair.transfo.getOrientation());
                        
                        numberOfPoints += _pair.scan.countPointsInBox(gridBox,clippingAssembly);
                    }
                    std::vector<int> coord;
                    coord.push_back(i);
                    coord.push_back(j);
                    coord.push_back(k);
                    if (numberOfPoints > 10000)
                        dynamicVoxelsBucketed[0].push_back(coord);
                    else if (numberOfPoints > 1000)
                        dynamicVoxelsBucketed[1].push_back(coord);
                    else if (numberOfPoints > 500)
                        dynamicVoxelsBucketed[2].push_back(coord);
                    else if (numberOfPoints > 0)
                        dynamicVoxelsBucketed[3].push_back(coord);
                }
            }
    return true;
}

void TlScanOverseer::mergeDynamicVoxelsVertically(const VoxelGrid& voxelGrid, const std::vector<std::vector<std::vector<bool>>>& dynamicVoxels, std::vector<glm::dvec3>& centers, std::vector<int>& sizes)
{
    double voxelSize = voxelGrid.m_voxelSize;
    for (int i = 0; i < voxelGrid.m_sizeX; i++)
    {
        for (int j = 0; j < voxelGrid.m_sizeY; j++)
        {
            int k = 0;
            bool startBox(false);
            int startBoxIndex(0),endBoxIndex(0);
            while (k < voxelGrid.m_sizeZ)
            {
                if (dynamicVoxels[i][j][k])
                {
                    if (!startBox)
                    {
                        startBox = true;
                        startBoxIndex = k;
                    }
                    k++;
                }
                else if (startBox)
                {
                    startBox = false;
                    endBoxIndex = k - 1;
                    glm::dvec3 position = glm::dvec3(voxelGrid.m_xMin + i * voxelSize, voxelGrid.m_yMin+j * voxelSize, voxelGrid.m_zMin + 0.5 * (startBoxIndex + endBoxIndex) * voxelSize);
                    position = position + glm::dvec3(0.5 * voxelSize, 0.5 * voxelSize, 0.5 * voxelSize);
                    centers.push_back(position);
                    sizes.push_back(endBoxIndex - startBoxIndex+1);
                    k++;
                }
                else k++;
            }
            if (startBox)
            {
                //if dynamic all the way to the top of the voxel grid
                glm::dvec3 position = glm::dvec3(voxelGrid.m_xMin + i * voxelSize, j * voxelSize, voxelGrid.m_zMin + 0.5 * (startBoxIndex + voxelGrid.m_sizeZ-1) * voxelSize);
                position = position + glm::dvec3(0.5 * voxelSize, 0.5 * voxelSize, 0.5 * voxelSize);
                centers.push_back(position);
                sizes.push_back(voxelGrid.m_sizeZ - startBoxIndex);
            }
        }
    }
}

void TlScanOverseer::createClustersOfDynamicVoxels(const VoxelGrid& voxelGrid, const std::vector<std::vector<std::vector<bool>>>& dynamicVoxels, std::vector<std::vector<std::vector<int>>>& clusterLabels, int& numberOfClusters, std::vector<ClusterInfo>& clusterInfo, const int& threshold)
{
    clusterInfo.clear();
    //intitialize labels
    int sizeX((int)dynamicVoxels.size()), sizeY((int)dynamicVoxels[0].size()), sizeZ((int)dynamicVoxels[0][0].size());
    std::vector<int> temp(sizeZ, 0);
    std::vector<std::vector<int>> temp1(sizeY, temp);
    clusterLabels = std::vector<std::vector<std::vector<int>>>(sizeX, temp1);
    int currentLabel = 1;

    // label dynamicVoxels in cluster, using breadthFirstSearch
    for (int i = 0; i < sizeX; i++) {
        for (int j = 0; j < sizeY; j++) {
            for (int k = 0; k < sizeZ; k++) {
                if ((dynamicVoxels[i][j][k]) && (clusterLabels[i][j][k] <1)) {
                    ClusterInfo tempInfo(sizeX, sizeY, sizeZ, currentLabel);
                    breadthFirstSearch(i, j, k, dynamicVoxels, clusterLabels, currentLabel,tempInfo, threshold, voxelGrid);
                    clusterInfo.push_back(tempInfo);
                }			
            }
        }
    }	
    numberOfClusters = currentLabel - 1;
    Logger::log(LoggerMode::rayTracingLog) << "numberOfClusters found : " << numberOfClusters << Logger::endl;

}

void TlScanOverseer::createClustersOfDynamicOctreeVoxels(const OctreeVoxelGrid& octreeVoxelGrid, const std::vector<std::vector<std::vector<bool>>>& dynamicVoxels, std::vector<std::vector<std::vector<int>>>& clusterLabels, int& numberOfClusters, std::vector<ClusterInfo>& clusterInfo, const int& threshold)
{
    clusterInfo.clear();
    //intitialize labels
    std::vector<int> temp((1ull << octreeVoxelGrid.m_maxDepth), 0);
    std::vector<std::vector<int>> temp1((1ull << octreeVoxelGrid.m_maxDepth), temp);
    clusterLabels = std::vector<std::vector<std::vector<int>>>((1ull << octreeVoxelGrid.m_maxDepth), temp1);
    int currentLabel = 1;

    // label dynamicVoxels in cluster, using breadthFirstSearch
    for (int i = 0; i < (1 << octreeVoxelGrid.m_maxDepth); i++) {
        for (int j = 0; j < (1 << octreeVoxelGrid.m_maxDepth); j++) {
            for (int k = 0; k < (1 << octreeVoxelGrid.m_maxDepth); k++) {
                if ((dynamicVoxels[i][j][k]) && (clusterLabels[i][j][k] < 1)) {
                    ClusterInfo tempInfo((1 << octreeVoxelGrid.m_maxDepth), (1 << octreeVoxelGrid.m_maxDepth), (1 << octreeVoxelGrid.m_maxDepth), currentLabel);
                    breadthFirstSearchOctree(i, j, k, dynamicVoxels, clusterLabels, currentLabel, tempInfo, threshold, octreeVoxelGrid);
                    clusterInfo.push_back(tempInfo);
                }
            }
        }
    }
    numberOfClusters = currentLabel - 1;
    Logger::log(LoggerMode::rayTracingLog) << "numberOfClusters found : " << numberOfClusters << Logger::endl;

}

void TlScanOverseer::mergeNearClustersTogether(std::vector<std::vector<std::vector<int>>>& clusterLabels, const int& numberOfClusters, std::vector<int>& holesInLabels, std::vector<ClusterInfo>& clusterInfo, const int& threshold)
{
    Logger::log(LoggerMode::rayTracingLog) << "clusterInfo size : " << clusterInfo.size() << Logger::endl;

    holesInLabels.clear();
    int sizeX((int)clusterLabels.size()), sizeY((int)clusterLabels[0].size()), sizeZ((int)clusterLabels[0][0].size());
    bool loop(true);
    std::vector<int> clustersWorthTrying, clustersNotMerged;
    for (int i=1;i<numberOfClusters+1;i++)
    {
        clustersWorthTrying.push_back(i);
    }
    while ((int)clustersWorthTrying.size()>0)
    {
        std::vector<int> clustersToTryNext;
        int numberOfMerges(0);
        bool firstMerge(false);
        for (int i=0;i<(int)clustersWorthTrying.size();i++)
        {
            bool clusteredAddedForNextLoop(false);
            int c = clustersWorthTrying[i];
            //test is c is in a hole
            for (int t = 0; t < (int)holesInLabels.size(); t++)
                if (c == holesInLabels[t])
                    continue;
            for (int j = i + 1; j < (int)clustersWorthTrying.size(); j++)
            {
                int d = clustersWorthTrying[j];
                //test if d is in a hole
                for (int t = 0; t < (int)holesInLabels.size(); t++)
                    if (d == holesInLabels[t])
                        continue;
                
                //rough distance test
                if (distanceBetweenBoundingBoxes(clusterInfo[c-1], clusterInfo[d-1]) > threshold)
                    continue;
                // compute distance between cluster c and d, if distance < D, stop calculating, and merge them
                if (isDisanceBetweenClustersSmallerThanThreshold(clusterLabels, c, d, threshold, clusterInfo[d]))
                {
                    changeClusterLabel(clusterLabels, d, c);
                    
                    mergeClusterInfo(clusterInfo[c-1], clusterInfo[d-1]);
                    holesInLabels.push_back(d);
                    numberOfMerges++;
                    if (!firstMerge)
                    {
                        firstMerge = true;
                    }
                }
            }
            if (!firstMerge)
                clustersNotMerged.push_back(c);
        }
        clustersWorthTrying.clear();
        for (int i = 1; i < numberOfClusters + 1; i++)
        {
            bool isAHole(false),didNotMerge(false);
            for (int t = 0; t < (int)holesInLabels.size(); t++)
            {
                if (i == holesInLabels[t])
                {
                    isAHole = true;
                    break;
                }
            }
            for (int t = 0; t < (int)clustersNotMerged.size(); t++)
            {
                if (i == clustersNotMerged[t])
                {
                    didNotMerge = true;
                    break;
                }
            }
            if((!isAHole)&&(!didNotMerge))
                clustersWorthTrying.push_back(i);
        }
        Logger::log(LoggerMode::rayTracingLog) << "merges this loop : " << numberOfMerges << Logger::endl;
        loop = firstMerge;
    }
    Logger::log(LoggerMode::rayTracingLog) << "numberOfClusters after merge: " << numberOfClusters-(int)holesInLabels.size() << Logger::endl;

    return;
}

int TlScanOverseer::distanceBetweenBoundingBoxes(const ClusterInfo& info1, const ClusterInfo& info2)
{
    int result;
    result = std::max(info1.m_xMin - info2.m_xMax, 0);
    result = std::min(result, std::max(info1.m_yMin - info2.m_yMax, 0));
    result = std::min(result, std::max(info1.m_zMin - info2.m_zMax, 0));
    result = std::min(result, std::max(info2.m_xMin - info1.m_xMax, 0));
    result = std::min(result, std::max(info2.m_yMin - info1.m_yMax, 0));
    result = std::min(result, std::max(info2.m_zMin - info1.m_zMax, 0));
    return result;
}

int TlScanOverseer::distanceBetweenPointAndBoundingBox(const ClusterInfo& info, const int& x, const int& y, const int& z)
{
    int result;
    result = std::max(info.m_xMin - x, 0);
    result = std::min(result, std::max(info.m_yMin - y, 0));
    result = std::min(result, std::max(info.m_zMin - z, 0));
    result = std::min(result, std::max(x - info.m_xMax, 0));
    result = std::min(result, std::max(y - info.m_yMax, 0));
    result = std::min(result, std::max(z - info.m_zMax, 0));
    return result;
}

void TlScanOverseer::mergeClusterInfo(ClusterInfo& targetInfo, const ClusterInfo& info2)
{
    targetInfo.m_volume += info2.m_volume;

    if (info2.m_xMin < targetInfo.m_xMin)
        targetInfo.m_xMin = info2.m_xMin;

    if (info2.m_yMin < targetInfo.m_yMin)
        targetInfo.m_yMin = info2.m_yMin;

    if (info2.m_zMin < targetInfo.m_zMin)
        targetInfo.m_zMin = info2.m_zMin;

    if (info2.m_xMax > targetInfo.m_xMax)
        targetInfo.m_xMax = info2.m_xMax;

    if (info2.m_yMax > targetInfo.m_yMax)
        targetInfo.m_yMax = info2.m_yMax;

    if (info2.m_zMax > targetInfo.m_zMax)
        targetInfo.m_zMax = info2.m_zMax;

    return;
}

bool TlScanOverseer::isDistanceBetweenVoxelAndClusterSmallerThanThreshold(const std::vector<std::vector<std::vector<int>>>& clusterLabels, const int& x, const int& y, const int& z, const int& clusterLabel, const int& threshold, const ClusterInfo& info2)
{
    int sizeX((int)clusterLabels.size()), sizeY((int)clusterLabels[0].size()), sizeZ((int)clusterLabels[0][0].size());
    if (distanceBetweenPointAndBoundingBox(info2, x, y, z) > threshold)
        return true;

    for (int i = x-threshold; i < x+threshold; i++) {
        for (int j = y-threshold; j < y+threshold; j++) {
            for (int k = z-threshold; k < z+threshold; k++) {
                if (i < 0 || i >= sizeX || j < 0 || j >= sizeY || k < 0 || k >= sizeZ)
                    continue;
                if (clusterLabels[i][j][k]==clusterLabel)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool TlScanOverseer::isDisanceBetweenClustersSmallerThanThreshold(const std::vector<std::vector<std::vector<int>>>& clusterLabels, const int& label1, const int& label2, const int& threshold, const ClusterInfo& info2)
{
    int sizeX((int)clusterLabels.size()), sizeY((int)clusterLabels[0].size()), sizeZ((int)clusterLabels[0][0].size());

    for (int i = 0; i < sizeX; i++) {
        for (int j = 0; j < sizeY; j++) {
            for (int k = 0; k < sizeZ; k++) {
                if (clusterLabels[i][j][k] == label1)
                {
                    if (isDistanceBetweenVoxelAndClusterSmallerThanThreshold(clusterLabels, i, j, k, label2, threshold, info2))
                        return true;
                }
            }
        }
    }
    return false;
}

void TlScanOverseer::changeClusterLabel(std::vector<std::vector<std::vector<int>>>& clusterLabels, const int& oldLabel, const int& newLabel)
{
    if (oldLabel == newLabel)
        return;
    int sizeX((int)clusterLabels.size()), sizeY((int)clusterLabels[0].size()), sizeZ((int)clusterLabels[0][0].size());

    for (int i = 0; i < sizeX; i++) {
        for (int j = 0; j < sizeY; j++) {
            for (int k = 0; k < sizeZ; k++) {
                if (clusterLabels[i][j][k] == oldLabel)
                {
                    clusterLabels[i][j][k] = newLabel;
                }
            }
        }
    }
    return;
}

void TlScanOverseer::getBoundingBoxOfCluster(const std::vector<std::vector<std::vector<int>>>& clusterLabels, const int& label, std::vector<int>& boundingBox)
{
    // boundingBox = (xMin,xMax,yMin,yMax,zMin,zMax) , min inclusive, max exclusive
    int sizeX((int)clusterLabels.size()), sizeY((int)clusterLabels[0].size()), sizeZ((int)clusterLabels[0][0].size());
    boundingBox.clear();
    boundingBox.push_back(sizeX);
    boundingBox.push_back(0);
    boundingBox.push_back(sizeY);
    boundingBox.push_back(0);
    boundingBox.push_back(sizeZ);
    boundingBox.push_back(0);
    for (int i = 0; i < sizeX; i++) {
        for (int j = 0; j < sizeY; j++) {
            for (int k = 0; k < sizeZ; k++) {
                if (clusterLabels[i][j][k] == label)
                {
                    if (i < boundingBox[0])
                        boundingBox[0] = i;
                    if (i >= boundingBox[1])
                        boundingBox[1] = i+1;
                    if (j < boundingBox[2])
                        boundingBox[2] = j;
                    if (j >= boundingBox[3])
                        boundingBox[3] = j+1;
                    if (k < boundingBox[4])
                        boundingBox[4] = k;
                    if (k >= boundingBox[5])
                        boundingBox[5] = k+1;
                }
            }
        }
    }
    
    return;
}

std::vector<std::vector<int>> TlScanOverseer::coverClusterWithBoxes(const std::vector<std::vector<std::vector<int>>>& clusterLabels, const int& label, const std::vector<std::vector<std::vector<bool>>>& dynamicVoxels, const VoxelGrid& voxelGrid, const ClusterInfo& clusterInfo)
{

    std::vector<std::vector<int>> resultBoxes;
    std::vector<int> boundingBox(6);
    //getBoundingBoxOfCluster(clusterLabels, label, boundingBox);
    boundingBox[0] = clusterInfo.m_xMin;
    boundingBox[1] = clusterInfo.m_xMax;
    boundingBox[2] = clusterInfo.m_yMin;
    boundingBox[3] = clusterInfo.m_yMax;
    boundingBox[4] = clusterInfo.m_zMin;
    boundingBox[5] = clusterInfo.m_zMax;
    std::queue<std::vector<int>> q;
    q.push(boundingBox);
    if ((clusterInfo.m_volume >= 2000) && (!clusterInfo.isTrueDynamic()))
        return resultBoxes;
    while (!q.empty())
    {
        std::vector<int> currBox = q.front();
        q.pop();
        if (!isBoxOfPositiveSize(currBox))
            continue;
        int x, y, z;
        std::vector<std::vector<int>> boxesToAdd;
        if (shouldSplitBox(clusterLabels, label, dynamicVoxels, voxelGrid, currBox, x, y, z))
        {
            splitBoxAtPoint2(currBox, boxesToAdd, x, y, z);
            for (int i = 0; i < (int)boxesToAdd.size(); i++)
                q.push(boxesToAdd[i]);
        }
        else
        {
            smallifyBoxToMatchCluster(clusterLabels, label, currBox);
            if(isBoxOfPositiveSize(currBox))
                resultBoxes.push_back(currBox);
        }
    }
    if(clusterInfo.m_volume<2000)
        mergeBoxes(resultBoxes);
    
    //Logger::log(LoggerMode::rayTracingLog) << "cluster number : " << label << " has been covered by " << resultBoxes.size() << " boxes" << Logger::endl;

    return resultBoxes;
}

std::vector<std::vector<int>> TlScanOverseer::coverClusterWithBoxesOctree(const std::vector<std::vector<std::vector<int>>>& clusterLabels, const int& label, const std::vector<std::vector<std::vector<bool>>>& dynamicVoxels, const OctreeVoxelGrid& octreeVoxelGrid, const ClusterInfo& clusterInfo)
{

    std::vector<std::vector<int>> resultBoxes;
    std::vector<int> boundingBox(6);
    //getBoundingBoxOfCluster(clusterLabels, label, boundingBox);
    boundingBox[0] = clusterInfo.m_xMin;
    boundingBox[1] = clusterInfo.m_xMax;
    boundingBox[2] = clusterInfo.m_yMin;
    boundingBox[3] = clusterInfo.m_yMax;
    boundingBox[4] = clusterInfo.m_zMin;
    boundingBox[5] = clusterInfo.m_zMax;
    std::queue<std::vector<int>> q;
    q.push(boundingBox);
    while (!q.empty())
    {
        std::vector<int> currBox = q.front();
        q.pop();
        if (!isBoxOfPositiveSize(currBox))
            continue;
        int x, y, z;
        std::vector<std::vector<int>> boxesToAdd;
        if (shouldSplitBoxOctree(clusterLabels, label, dynamicVoxels, octreeVoxelGrid, currBox, x, y, z))
        {
            splitBoxAtPoint(currBox, boxesToAdd, x, y, z);
            for (int i = 0; i < (int)boxesToAdd.size(); i++)
                q.push(boxesToAdd[i]);
        }
        else
        {
            //smallifyBoxToMatchCluster(clusterLabels, label, currBox);
            if(isBoxOfPositiveSize(currBox))
                resultBoxes.push_back(currBox);
        }
    }
    mergeBoxes(resultBoxes);
    smallifyBoxesToMatchCluster(clusterLabels, label, resultBoxes);
    return resultBoxes;
}

int TlScanOverseer::getVolumeOfCluster(const std::vector<std::vector<std::vector<int>>>& clusterLabels, const int& label)
{
    int sizeX((int)clusterLabels.size()), sizeY((int)clusterLabels[0].size()), sizeZ((int)clusterLabels[0][0].size());
    int result(0);
    for (int i = 0; i < sizeX; i++) 
        for (int j = 0; j < sizeY; j++) 
            for (int k = 0; k < sizeZ; k++) 
                if (clusterLabels[i][j][k] == label)
                
                    result++;
                
    return result;
}

void TlScanOverseer::mergeBoxes(std::vector<std::vector<int>>& boxes) {
    //successively merge boxes that share an exact face
    bool merged;
    do {
        merged = false;
        for (int i = 0; i < boxes.size(); i++) {
            for (int j = i + 1; j < boxes.size(); j++) {
                std::set<std::vector<int>> box1Corners = getBoxCorners(boxes[i]);
                std::set<std::vector<int>> box2Corners = getBoxCorners(boxes[j]);
                std::vector<std::vector<int>> commonCorners;
                std::set_intersection(box1Corners.begin(), box1Corners.end(), box2Corners.begin(), box2Corners.end(), std::back_inserter(commonCorners));
                if (commonCorners.size() == 4) {
                    std::vector<int> mergedBox = mergeTwoBoxes(boxes[i], boxes[j]);
                    boxes.erase(boxes.begin() + i);
                    boxes.erase(boxes.begin() + j - 1);
                    boxes.push_back(mergedBox);
                    merged = true;
                    break;
                }
            }
            if (merged) break;
        }
    } while (merged);
    return;
}

std::vector<int> TlScanOverseer::mergeTwoBoxes(const std::vector<int>& box1, const std::vector<int>& box2) {
    //return a box that contains both boxes
    std::vector<int> mergedBox;
    mergedBox.push_back(std::min(box1[0], box2[0]));
    mergedBox.push_back(std::max(box1[1], box2[1]));
    mergedBox.push_back(std::min(box1[2], box2[2]));
    mergedBox.push_back(std::max(box1[3], box2[3]));
    mergedBox.push_back(std::min(box1[4], box2[4]));
    mergedBox.push_back(std::max(box1[5], box2[5]));
    return mergedBox;
}

void TlScanOverseer::smallifyBoxToMatchCluster(const std::vector<std::vector<std::vector<int>>>& clusterLabels, const int& label, std::vector<int>& box)
{
    std::vector<int> resultbox;
    resultbox.push_back(box[1]);
    resultbox.push_back(box[0]);
    resultbox.push_back(box[3]);
    resultbox.push_back(box[2]);
    resultbox.push_back(box[5]);
    resultbox.push_back(box[4]);
    for (int i = box[0]; i < box[1]; i++)
        for (int j = box[2]; j < box[3]; j++)
            for (int k = box[4]; k < box[5]; k++)
            {
                if (clusterLabels[i][j][k] == label)
                {
                    if (i < resultbox[0])
                        resultbox[0] = i;
                    if (i >= resultbox[1])
                        resultbox[1] = i+1;
                    if (j < resultbox[2])
                        resultbox[2] = j;
                    if (j >= resultbox[3])
                        resultbox[3] = j+1;
                    if (k < resultbox[4])
                        resultbox[4] = k;
                    if (k >= resultbox[5])
                        resultbox[5] = k+1;
                }
            }
    box = resultbox;
    return;
}

void TlScanOverseer::smallifyBoxesToMatchCluster(const std::vector<std::vector<std::vector<int>>>& clusterLabels, const int& label, std::vector<std::vector<int>>& boxes)
{
    std::vector<int> tempBox;
    for (int i = 0; i < (int)boxes.size(); i++)
    {
        tempBox = boxes[i];
        smallifyBoxToMatchCluster(clusterLabels, label, tempBox);
        boxes[i] = tempBox;
    }
    return;
}

std::set<std::vector<int>> TlScanOverseer::getBoxCorners(const std::vector<int>& box) {
    std::set<std::vector<int>> corners;
    corners.insert({ box[0], box[2], box[4] });
    corners.insert({ box[0], box[2], box[5] });
    corners.insert({ box[0], box[3], box[4] });
    corners.insert({ box[0], box[3], box[5] });
    corners.insert({ box[1], box[2], box[4] });
    corners.insert({ box[1], box[2], box[5] });
    corners.insert({ box[1], box[3], box[4] });
    corners.insert({ box[1], box[3], box[5] });
    return corners;
}


bool TlScanOverseer::shouldSplitBox(const std::vector<std::vector<std::vector<int>>>& clusterLabels, const int& label, const std::vector<std::vector<std::vector<bool>>>& dynamicVoxels, const VoxelGrid& voxelGrid, const std::vector<int>& box, int& x, int& y, int& z)
{
    // (x;y;z) is a spot to split, only relevant if it returns true
    for (int i=box[0];i<box[1];i++)
        for (int j=box[2];j<box[3];j++)
            for (int k = box[4]; k < box[5]; k++)
            {
                if(voxelGrid.m_grid[i][j][k]!=0)
                    if (clusterLabels[i][j][k] != label)
                    {
                        x = i;
                        y = j;
                        z = k;
                        return true;
                    }
            }
    return false;
}

bool TlScanOverseer::shouldSplitBoxOctree(const std::vector<std::vector<std::vector<int>>>& clusterLabels, const int& label, const std::vector<std::vector<std::vector<bool>>>& dynamicVoxels, const OctreeVoxelGrid& octreeVoxelGrid, const std::vector<int>& box, int& x, int& y, int& z)
{
    // (x;y;z) is a spot to split, only relevant if it returns true
    for (int i = box[0]; i < box[1]; i++)
        for (int j = box[2]; j < box[3]; j++)
            for (int k = box[4]; k < box[5]; k++)
            {
                if (!octreeVoxelGrid.isEmpty(i,j,k))
                    if (clusterLabels[i][j][k] != label)
                    {
                        x = i;
                        y = j;
                        z = k;
                        return true;
                    }
            }
    return false;
}

bool TlScanOverseer::doesBoxContainALabeledVoxel(const std::vector<int>& box, const std::vector<std::vector<std::vector<int>>>& clusterLabels, const int& label)
{
    for (int i = box[0]; i < box[1]; i++)
        for (int j = box[2]; j < box[3]; j++)
            for (int k = box[4]; k < box[5]; k++)
            {
                if (clusterLabels[i][j][k] == label)
                    return true;
            }
    return false;
}

bool TlScanOverseer::isBoxOfPositiveSize(const std::vector<int>& box)
{
    return(box[0] < box[1] && box[2] < box[3] && box[4] < box[5]);
}

void TlScanOverseer::splitBoxAtPoint(const std::vector<int>& box, std::vector<std::vector<int>>& resultBoxes, const int& x, const int& y, const int& z)
{
    resultBoxes.clear();
    //check if point inside box, nothing to do if thats the case
    if (x < box[0] || x >= box[1] || y < box[2] || y >= box[3] || z < box[4] || z >= box[5])
    {
        return;
    }
    //26 box to create, which correspond to directions (a;b;c) in {-1;0;1}^3 \ (0;0;0)
    for (int i = 0; i < 27; i++)
    {

        int a = (i % 3);
        int b = (((int)(i / 3)) % 3);
        int c = (((int)(i / 9)) % 3);
        
        if (a == 1 && b == 1 && c == 1)
            continue;
        std::vector<int> tempBox(6);
        switch (a)
        {
        case 0:
        {
            tempBox[0] = box[0];
            tempBox[1] = x;
            break;
        }
        case 1:
        {
            tempBox[0] = x;
            tempBox[1] = x+1;
            break;
        }
        case 2:
        {
            tempBox[0] = x+1;
            tempBox[1] = box[1];
            break;
        }
        }
        switch (b)
        {
        case 0:
        {
            tempBox[2] = box[2];
            tempBox[3] = y;
            break;
        }
        case 1:
        {
            tempBox[2] = y;
            tempBox[3] = y + 1;
            break;
        }
        case 2:
        {
            tempBox[2] = y + 1;
            tempBox[3] = box[3];
            break;
        }
        }
        switch (c)
        {
        case 0:
        {
            tempBox[4] = box[4];
            tempBox[5] = z;
            break;
        }
        case 1:
        {
            tempBox[4] = z;
            tempBox[5] = z + 1;
            break;
        }
        case 2:
        {
            tempBox[4] = z + 1;
            tempBox[5] = box[5];
            break;
        }
        }
        resultBoxes.push_back(tempBox);
    }
    return;
}

void TlScanOverseer::splitBoxAtPoint2(const std::vector<int>& box, std::vector<std::vector<int>>& resultBoxes, const int& x, const int& y, const int& z)
{
    resultBoxes.clear();
    //check if point inside box, nothing to do if thats the case
    if (x < box[0] || x >= box[1] || y < box[2] || y >= box[3] || z < box[4] || z >= box[5])
    {
        return;
    }
    //create 6 boxes, we split first in the biggest dimension, then in the next biggest
    int sizeX(box[1] - box[0]), sizeY(box[3] - box[2]), sizeZ(box[5] - box[4]);
    int caseDimension;
    // (x,y,z) (x,z,y) (y,x,z) (y,z,x) (z,x,y) (z,y,x)
    if (sizeX >= sizeY)
        if (sizeY >= sizeZ)
            //case (x,y,z)
            caseDimension = 0;
        else if (sizeX >= sizeZ)
            //case (x,z,y)
            caseDimension = 1;
        else
            //case (z,x,y)
            caseDimension = 4;
    else if (sizeX >= sizeZ)
        //case (y,x,z)
        caseDimension = 2;
    else if (sizeY >= sizeZ)
        // case (y,z,x)
        caseDimension = 3;
    else // case (z,y,x)
        caseDimension = 5;

    switch (caseDimension)
    {
    case 0:
    {
        //(x,y,z)
        std::vector<int> tempBox = box;
        tempBox[1] = x - 1;
        resultBoxes.push_back(tempBox);
        tempBox[0] = x + 1;
        tempBox[1] = box[1];
        resultBoxes.push_back(tempBox);
        tempBox[0] = x;
        tempBox[1] = x + 1;
        tempBox[3] = y - 1;
        resultBoxes.push_back(tempBox);
        tempBox[2] = y + 1;
        tempBox[3] = box[3];
        resultBoxes.push_back(tempBox);
        tempBox[2] = y;
        tempBox[3] = y + 1;
        tempBox[5] = z - 1;
        resultBoxes.push_back(tempBox);
        tempBox[4] = z + 1;
        tempBox[5] = box[5];
        resultBoxes.push_back(tempBox);
        break;
    }
    case 1:
    {
        //(x,z,y)
        std::vector<int> tempBox = box;
        tempBox[1] = x - 1;
        resultBoxes.push_back(tempBox);
        tempBox[0] = x + 1;
        tempBox[1] = box[1];
        resultBoxes.push_back(tempBox);
        tempBox[0] = x;
        tempBox[1] = x + 1;
        tempBox[5] = z - 1;
        resultBoxes.push_back(tempBox);
        tempBox[4] = z + 1;
        tempBox[5] = box[5];
        resultBoxes.push_back(tempBox);
        tempBox[4] = z;
        tempBox[5] = z + 1;
        tempBox[3] = y - 1;
        resultBoxes.push_back(tempBox);
        tempBox[2] = y + 1;
        tempBox[3] = box[3];
        resultBoxes.push_back(tempBox);
        break;
    }
    case 2:
    {
        //(y,x,z)
        std::vector<int> tempBox = box;
        tempBox[3] = y - 1;
        resultBoxes.push_back(tempBox);
        tempBox[2] = y + 1;
        tempBox[3] = box[3];
        resultBoxes.push_back(tempBox);
        tempBox[2] = y;
        tempBox[3] = y + 1;
        tempBox[1] = x - 1;
        resultBoxes.push_back(tempBox);
        tempBox[0] = x + 1;
        tempBox[1] = box[1];
        resultBoxes.push_back(tempBox);
        tempBox[0] = x;
        tempBox[1] = x + 1;
        tempBox[5] = z - 1;
        resultBoxes.push_back(tempBox);
        tempBox[4] = z + 1;
        tempBox[5] = box[5];
        resultBoxes.push_back(tempBox);
        break;
    }
    case 3:
    {
        //(y,z,x)
        std::vector<int> tempBox = box;
        tempBox[3] = y - 1;
        resultBoxes.push_back(tempBox);
        tempBox[2] = y + 1;
        tempBox[3] = box[3];
        resultBoxes.push_back(tempBox);
        tempBox[2] = y;
        tempBox[3] = y + 1;
        tempBox[5] = z - 1;
        resultBoxes.push_back(tempBox);
        tempBox[4] = z + 1;
        tempBox[5] = box[5];
        resultBoxes.push_back(tempBox);
        tempBox[4] = z;
        tempBox[5] = z + 1;
        tempBox[1] = x - 1;
        resultBoxes.push_back(tempBox);
        tempBox[0] = x + 1;
        tempBox[1] = box[1];
        resultBoxes.push_back(tempBox);
        break;
    }
    case 4:
    {
        //(z,x,y)
        std::vector<int> tempBox = box;
        tempBox[5] = z - 1;
        resultBoxes.push_back(tempBox);
        tempBox[4] = z + 1;
        tempBox[5] = box[5];
        resultBoxes.push_back(tempBox);
        tempBox[4] = z;
        tempBox[5] = z + 1;
        tempBox[1] = x - 1;
        resultBoxes.push_back(tempBox);
        tempBox[0] = x + 1;
        tempBox[1] = box[1];
        resultBoxes.push_back(tempBox);
        tempBox[0] = x;
        tempBox[1] = x + 1;
        tempBox[3] = y - 1;
        resultBoxes.push_back(tempBox);
        tempBox[2] = y + 1;
        tempBox[3] = box[3];
        resultBoxes.push_back(tempBox);
        break;
    }
    case 5:
    {
        //(z,y,x)
        std::vector<int> tempBox = box;
        tempBox[5] = z - 1;
        resultBoxes.push_back(tempBox);
        tempBox[4] = z + 1;
        tempBox[5] = box[5];
        resultBoxes.push_back(tempBox);
        tempBox[4] = z;
        tempBox[5] = z + 1;
        tempBox[3] = y - 1;
        resultBoxes.push_back(tempBox);
        tempBox[2] = y + 1;
        tempBox[3] = box[3];
        resultBoxes.push_back(tempBox);
        tempBox[2] = y;
        tempBox[3] = y + 1;
        tempBox[1] = x - 1;
        resultBoxes.push_back(tempBox);
        tempBox[0] = x + 1;
        tempBox[1] = box[1];
        resultBoxes.push_back(tempBox);
        break;
    }
    }
    

}

void TlScanOverseer::breadthFirstSearch(const int& x,const int& y, const int& z, const std::vector<std::vector<std::vector<bool>>>& dynamicVoxels, std::vector<std::vector<std::vector<int>>>& clusterLabels, int& currentLabel, ClusterInfo& clusterInfo, const int& threshold, const VoxelGrid& voxelGrid)
{
    //assigns currentLabel to dynamic cells in the same cluster as (x,y,z)

    int sizeX((int)dynamicVoxels.size()), sizeY((int)dynamicVoxels[0].size()), sizeZ((int)dynamicVoxels[0][0].size());
    int dx[6] = { -1, 0, 0, 1, 0, 0 };
    int dy[6] = { 0, -1, 0, 0, 1, 0 };
    int dz[6] = { 0, 0, -1, 0, 0, 1 };
    std::queue<std::vector<int>> q;
    std::vector<int> coord;
    coord.push_back(x);
    coord.push_back(y);
    coord.push_back(z);
    q.push(coord);
    clusterLabels[x][y][z] = currentLabel;
    clusterInfo.updateWithPoint(x, y, z);
    while (!q.empty()) {
        coord = q.front();
        q.pop();
        for (int i = -threshold + 1; i < threshold; i++)
        {
            for (int j = -threshold + 1; j < threshold; j++)
            {
                for (int k = -threshold + 1; k < threshold; k++)
                {
                    int nx(coord[0] + i), ny(coord[1] + j), nz(coord[2] + k);
                    if (nx >= 0 && nx < sizeX && ny >= 0 && ny < sizeY && nz >= 0 && nz < sizeZ)
                    {
                        //if this is an unlabeled dynamic voxel, add it to the cluster
                        if (dynamicVoxels[nx][ny][nz] && (clusterLabels[nx][ny][nz] < 1))
                        {
                            clusterLabels[nx][ny][nz] = currentLabel;
                            clusterInfo.updateWithPoint(nx, ny, nz);
                            std::vector<int> tempCoord;
                            tempCoord.push_back(nx);
                            tempCoord.push_back(ny);
                            tempCoord.push_back(nz);
                            q.push(tempCoord);
                        }

                        //if this is a neighbor, check if empty or static, and update clusterInfo accordingly
                        if ((abs(nx - coord[0]) + abs(ny - coord[1]) + abs(nz - coord[2])) < 2)
                        {
                            if ((voxelGrid.m_grid[nx][ny][nz] == 0) && (clusterLabels[nx][ny][nz] != -currentLabel))
                            {
                                clusterInfo.m_emptyNeighbors++;
                                clusterLabels[nx][ny][nz]=-currentLabel;
                            }
                            else if ((!dynamicVoxels[nx][ny][nz]) && (clusterLabels[nx][ny][nz] != -currentLabel))
                            {
                                clusterInfo.m_staticNeighbors++;
                                clusterLabels[nx][ny][nz] = -currentLabel;
                            }
                        }

                    }
                        
                }
            }
        }
        /*for (int i = 0; i < 6; i++)
        {
            int nx = coord[0] + dx[i], ny = coord[1] + dy[i], nz= coord[2]+dz[i];
            if (nx >= 0 && nx < sizeX && ny >= 0 && ny < sizeY && nz >= 0 && nz < sizeZ)
                if(dynamicVoxels[nx][ny][nz] && clusterLabels[nx][ny][nz] == 0) 
                {
                    clusterLabels[nx][ny][nz] = currentLabel;
                    clusterInfo.updateWithPoint(nx, ny, nz);
                    std::vector<int> tempCoord;
                    tempCoord.push_back(nx);
                    tempCoord.push_back(ny);
                    tempCoord.push_back(nz);
                    q.push(tempCoord);
                }
        }*/
    }
    currentLabel++;
    return;
}

void TlScanOverseer::breadthFirstSearchOctree(const int& x, const int& y, const int& z, const std::vector<std::vector<std::vector<bool>>>& dynamicVoxels, std::vector<std::vector<std::vector<int>>>& clusterLabels, int& currentLabel, ClusterInfo& clusterInfo, const int& threshold, const OctreeVoxelGrid& octreeVoxelGrid)
{
    //assigns currentLabel to dynamic cells in the same cluster as (x,y,z)

    int sizeX((int)dynamicVoxels.size()), sizeY((int)dynamicVoxels[0].size()), sizeZ((int)dynamicVoxels[0][0].size());
    int dx[6] = { -1, 0, 0, 1, 0, 0 };
    int dy[6] = { 0, -1, 0, 0, 1, 0 };
    int dz[6] = { 0, 0, -1, 0, 0, 1 };
    std::queue<std::vector<int>> q;
    std::vector<int> coord;
    coord.push_back(x);
    coord.push_back(y);
    coord.push_back(z);
    q.push(coord);
    clusterLabels[x][y][z] = currentLabel;
    clusterInfo.updateWithPoint(x, y, z);
    while (!q.empty()) {
        coord = q.front();
        q.pop();
        for (int i = -threshold + 1; i < threshold; i++)
        {
            for (int j = -threshold + 1; j < threshold; j++)
            {
                for (int k = -threshold + 1; k < threshold; k++)
                {
                    int nx(coord[0] + i), ny(coord[1] + j), nz(coord[2] + k);
                    if (nx >= 0 && nx < sizeX && ny >= 0 && ny < sizeY && nz >= 0 && nz < sizeZ)
                    {
                        //if this is an unlabeled dynamic voxel, add it to the cluster
                        if (dynamicVoxels[nx][ny][nz] && (clusterLabels[nx][ny][nz] < 1))
                        {
                            clusterLabels[nx][ny][nz] = currentLabel;
                            clusterInfo.updateWithPoint(nx, ny, nz);
                            std::vector<int> tempCoord;
                            tempCoord.push_back(nx);
                            tempCoord.push_back(ny);
                            tempCoord.push_back(nz);
                            q.push(tempCoord);
                        }

                        //if this is a neighbor, check if empty or static, and update clusterInfo accordingly
                        if ((abs(nx - coord[0]) + abs(ny - coord[1]) + abs(nz - coord[2])) < 2)
                        {
                            if ((octreeVoxelGrid.isEmpty(nx,ny,nz)) && (clusterLabels[nx][ny][nz] != -currentLabel))
                            {
                                clusterInfo.m_emptyNeighbors++;
                                clusterLabels[nx][ny][nz] = -currentLabel;
                            }
                            else if ((!dynamicVoxels[nx][ny][nz]) && (clusterLabels[nx][ny][nz] != -currentLabel))
                            {
                                clusterInfo.m_staticNeighbors++;
                                clusterLabels[nx][ny][nz] = -currentLabel;
                            }
                        }

                    }

                }
            }
        }
        /*for (int i = 0; i < 6; i++)
        {
            int nx = coord[0] + dx[i], ny = coord[1] + dy[i], nz= coord[2]+dz[i];
            if (nx >= 0 && nx < sizeX && ny >= 0 && ny < sizeY && nz >= 0 && nz < sizeZ)
                if(dynamicVoxels[nx][ny][nz] && clusterLabels[nx][ny][nz] == 0)
                {
                    clusterLabels[nx][ny][nz] = currentLabel;
                    clusterInfo.updateWithPoint(nx, ny, nz);
                    std::vector<int> tempCoord;
                    tempCoord.push_back(nx);
                    tempCoord.push_back(ny);
                    tempCoord.push_back(nz);
                    q.push(tempCoord);
                }
        }*/
    }
    currentLabel++;
    return;
}


bool TlScanOverseer::fitLocalPlane(const ClippingAssembly& clippingAssembly, const glm::dvec3& seedPoint, RectangularPlane& result)
{
    glm::dvec3 planeDirectionX, planeDirectionY;
    
    std::vector<double> plane;
    fitPlaneRegionGrowing(seedPoint, plane, clippingAssembly);
    if (plane.size() != 4)
        return false;
    glm::dvec3 normal = glm::dvec3(plane[0], plane[1], plane[2]);
    normal = normal / glm::length(normal);

    if (abs(normal[0]) > std::max(abs(normal[1]), abs(normal[2])))
    {
        glm::dvec3 temp(0.0, 1.0, 0.0);
        planeDirectionX = glm::cross(normal, temp);
        planeDirectionY = glm::cross(normal, planeDirectionX);
    }
    else 
    {
        glm::dvec3 temp(1.0, 0.0, 0.0);
        planeDirectionX = glm::cross(normal, temp);
        planeDirectionY = glm::cross(normal, planeDirectionX);
    }
    std::vector<glm::dvec3> corners;
    glm::dvec3 tempCorner = seedPoint + sqrt(2) / 2 * (planeDirectionX + planeDirectionY);
    corners.push_back(tempCorner);
    tempCorner = seedPoint + sqrt(2) / 2 * (planeDirectionX - planeDirectionY);
    corners.push_back(tempCorner);
    tempCorner = seedPoint + sqrt(2) / 2 * (-planeDirectionX + planeDirectionY);
    corners.push_back(tempCorner);
    tempCorner = seedPoint + sqrt(2) / 2 * (-planeDirectionX - planeDirectionY);
    corners.push_back(tempCorner);
    result=RectangularPlane(normal, corners, seedPoint,PlaneType::tilted);
    return true;
}

bool TlScanOverseer::fitPlaneAutoExtend(const ClippingAssembly& clippingAssembly, const glm::dvec3& seedPoint, RectangularPlane& rectPlane)
{
    //starter plane

    glm::dvec3 planeDirectionX, planeDirectionY;

    std::vector<double> plane;
    fitPlaneRegionGrowing(seedPoint, plane, clippingAssembly);
    if (plane.size() != 4)
        return false;
    glm::dvec3 normal = glm::dvec3(plane[0], plane[1], plane[2]);
    normal = normal / glm::length(normal);

    if (abs(normal[0]) > std::max(abs(normal[1]), abs(normal[2])))
    {
        glm::dvec3 temp(0.0, 1.0, 0.0);
        planeDirectionX = glm::cross(normal, temp);
        planeDirectionY = glm::cross(normal, planeDirectionX);
    }
    else
    {
        glm::dvec3 temp(1.0, 0.0, 0.0);
        planeDirectionX = glm::cross(normal, temp);
        planeDirectionY = glm::cross(normal, planeDirectionX);
    }
    std::vector<glm::dvec3> corners;
    double startingScale = 0.01;
    glm::dvec3 planeCenter = MeasureClass::projectPointToPlane(seedPoint, plane);
    glm::dvec3 tempCorner = seedPoint + startingScale*sqrt(2) / 2 * (planeDirectionX + planeDirectionY);
    corners.push_back(tempCorner);
    tempCorner = seedPoint + startingScale * sqrt(2) / 2 * (planeDirectionX - planeDirectionY);
    corners.push_back(tempCorner);
    tempCorner = seedPoint + startingScale * sqrt(2) / 2 * (-planeDirectionX + planeDirectionY);
    corners.push_back(tempCorner);
    tempCorner = seedPoint + startingScale * sqrt(2) / 2 * (-planeDirectionX - planeDirectionY);
    corners.push_back(tempCorner);
    rectPlane = RectangularPlane(normal, corners, seedPoint, PlaneType::tilted);


    double step = 0.5;
    double planeAngleThreshold(0.98);
    int loopCounter(0),maxLoop(200), consecutiveFails(0);
    int cornerIndex(0);
    while((consecutiveFails<4)&&(loopCounter<maxLoop))
    {
        loopCounter++;
        if (extendRectangleFromCorner(rectPlane, step, planeAngleThreshold, cornerIndex, clippingAssembly))
        {
            consecutiveFails=0;
        }
        else
        {
            consecutiveFails++;
        }
        //go to the next cornerIndex
        if (cornerIndex < 3)
            cornerIndex++;
        else cornerIndex = 0;
    }
    Logger::log(LoggerMode::rayTracingLog) << "loopCounter" << loopCounter << Logger::endl;

    consecutiveFails = 0;
    loopCounter = 0;
    step = 0.01;
    maxLoop = 400;
    planeAngleThreshold = 0.99;
    while ((consecutiveFails < 4) && (loopCounter < maxLoop))
    {
        loopCounter++;
        if (extendRectangleFromCorner(rectPlane, step, planeAngleThreshold, cornerIndex, clippingAssembly))
        {
            consecutiveFails = 0;
        }
        else
        {
            consecutiveFails++;
        }
        //go to the next cornerIndex
        if (cornerIndex < 3)
            cornerIndex++;
        else cornerIndex = 0;
    }

    Logger::log(LoggerMode::rayTracingLog) << "loopCounter" << loopCounter << Logger::endl;

    return true;
    //make a rectangular plane, define directions to look for extension

    //extend patch by patch

    //check if extension is similar enough, update plane accordingly

    //maybe patch local planes together, and check similarity only with neighbouring planes ?

    //each patch should store its local value
}

bool TlScanOverseer::extendRectangleFromCorner(RectangularPlane& rectPlane, const double& step, const double& planeAngleThreshold, const int& cornerIndex, const ClippingAssembly& clippingAssembly)
{
    //returns true if it extended something
    std::vector<glm::dvec3> extensionSeeds;
    glm::dvec3 dir1, dir2;
    switch (cornerIndex)
    {
    case 0:
    {
        dir1 = rectPlane.m_corners[0] - rectPlane.m_corners[1];
        dir2 = rectPlane.m_corners[0] - rectPlane.m_corners[2];
        break;
    }
    case 1:
    {
        dir1 = rectPlane.m_corners[1] - rectPlane.m_corners[0];
        dir2 = rectPlane.m_corners[1] - rectPlane.m_corners[3];
        break;
    }
    case 2:
    {
        dir1 = rectPlane.m_corners[2] - rectPlane.m_corners[0];
        dir2 = rectPlane.m_corners[2] - rectPlane.m_corners[3];
        break;
    }
    case 3:
    {
        dir1 = rectPlane.m_corners[3] - rectPlane.m_corners[1];
        dir2 = rectPlane.m_corners[3] - rectPlane.m_corners[2];
        break;
    }
    }
    dir1 /= glm::length(dir1);
    dir2 /= glm::length(dir2);
    extensionSeeds.push_back(rectPlane.m_corners[cornerIndex] + step * dir1 + step * dir2);
    extensionSeeds.push_back(rectPlane.m_corners[cornerIndex] + step * dir1);
    extensionSeeds.push_back(rectPlane.m_corners[cornerIndex] + step * dir2);

    // try to extend for each seed
    for (int i = 0; i < 3; i++)
    {
        if (extendPlaneFromSeed(rectPlane, extensionSeeds[i], planeAngleThreshold, clippingAssembly))
        {
            std::vector<glm::dvec3> newCorners = rectPlane.m_corners;
            // update rectPlane to do that extension
            switch (cornerIndex)
            {
            case 0:
            {
                switch (i)
                {
                case 0:
                {
                    newCorners[0] += step * dir1 + step * dir2;
                    newCorners[1] += step * dir2;
                    newCorners[2] += step * dir1;
                    rectPlane.updateCorners(newCorners);
                    break;
                }
                
                case 1:
                {
                    newCorners[0] += step * dir1;
                    newCorners[2] += step * dir1;
                    rectPlane.updateCorners(newCorners);
                    break;
                }
                
                case 2:
                {
                    newCorners[0] += step * dir2;
                    newCorners[1] += step * dir2;
                    rectPlane.updateCorners(newCorners);
                    break;
                }
                }
                break;
            }
            case 1:
            {
                switch (i)
                {
                case 0:
                {
                    newCorners[1] += step * dir1 + step * dir2;
                    newCorners[0] += step * dir2;
                    newCorners[3] += step * dir1;
                    rectPlane.updateCorners(newCorners);
                    break;
                }

                case 1:
                {
                    newCorners[1] += step * dir1;
                    newCorners[3] += step * dir1;
                    rectPlane.updateCorners(newCorners);
                    break;
                }

                case 2:
                {
                    newCorners[1] += step * dir2;
                    newCorners[0] += step * dir2;
                    rectPlane.updateCorners(newCorners);
                    break;
                }
                }
                break;
            }
            case 2:
            {
                switch (i)
                {
                case 0:
                {
                    newCorners[2] += step * dir1 + step * dir2;
                    newCorners[0] += step * dir2;
                    newCorners[3] += step * dir1;
                    rectPlane.updateCorners(newCorners);
                    break;
                }

                case 1:
                {
                    newCorners[2] += step * dir1;
                    newCorners[3] += step * dir1;
                    rectPlane.updateCorners(newCorners);
                    break;
                }

                case 2:
                {
                    newCorners[2] += step * dir2;
                    newCorners[0] += step * dir2;
                    rectPlane.updateCorners(newCorners);
                    break;
                }
                }
                break;
            }
            case 3:
            {
                switch (i)
                {
                case 0:
                {
                    newCorners[3] += step * dir1 + step * dir2;
                    newCorners[1] += step * dir2;
                    newCorners[2] += step * dir1;
                    rectPlane.updateCorners(newCorners);
                    break;
                }

                case 1:
                {
                    newCorners[3] += step * dir1;
                    newCorners[2] += step * dir1;
                    rectPlane.updateCorners(newCorners);
                    break;
                }

                case 2:
                {
                    newCorners[3] += step * dir2;
                    newCorners[1] += step * dir2;
                    rectPlane.updateCorners(newCorners);
                    break;
                }
                }
                break;
            }
            }
            //then return true
            return true;
        }
    }
    return false;
}

bool TlScanOverseer::extendPlaneFromSeed(const RectangularPlane& rectPlane, const glm::dvec3& seed, const double& planeAngleThreshold, const ClippingAssembly& clippingAssembly)
{
    //detect a local plane from seed

    glm::dvec3 planeDirectionX, planeDirectionY;
    std::vector<double> plane;
    fitPlaneRegionGrowing(seed, plane, clippingAssembly);
    if (plane.size() != 4)
        return false;
    glm::dvec3 normal = glm::dvec3(plane[0], plane[1], plane[2]);
    normal = normal / glm::length(normal);

    if (abs(normal[0]) > std::max(abs(normal[1]), abs(normal[2])))
    {
        glm::dvec3 temp(0.0, 1.0, 0.0);
        planeDirectionX = glm::cross(normal, temp);
        planeDirectionY = glm::cross(normal, planeDirectionX);
    }
    else
    {
        glm::dvec3 temp(1.0, 0.0, 0.0);
        planeDirectionX = glm::cross(normal, temp);
        planeDirectionY = glm::cross(normal, planeDirectionX);
    }
    std::vector<glm::dvec3> corners;
    glm::dvec3 tempCorner = seed + sqrt(2) / 2 * (planeDirectionX + planeDirectionY);
    corners.push_back(tempCorner);
    tempCorner = seed + sqrt(2) / 2 * (planeDirectionX - planeDirectionY);
    corners.push_back(tempCorner);
    tempCorner = seed + sqrt(2) / 2 * (-planeDirectionX + planeDirectionY);
    corners.push_back(tempCorner);
    tempCorner = seed + sqrt(2) / 2 * (-planeDirectionX - planeDirectionY);
    corners.push_back(tempCorner);
    RectangularPlane newPlane = RectangularPlane(normal, corners, seed, PlaneType::tilted);

    //test if plane is close enough to accept extension
    double planeAngle = abs(glm::dot(newPlane.m_normal, rectPlane.m_normal));
    Logger::log(LoggerMode::rayTracingLog) << "planeAngle : " << planeAngle << Logger::endl;

    if (planeAngle < planeAngleThreshold)
        return false;
    else return true;

}

void TlScanOverseer::fitPlaneMultipleSeeds(const std::vector<glm::dvec3>& seedPoints, TransformationModule& result)
{
    glm::dvec3 planeDirectionX, planeDirectionY;
    std::vector<double> plane;
    OctreeRayTracing::fitPlane(seedPoints, plane);
    if (plane.size() != 4)
        return;	
    glm::dvec3 normal = glm::dvec3(plane[0], plane[1], plane[2]);
    normal = normal / glm::length(normal);
    glm::dvec3 temp;

    if (abs(normal[0]) > std::max(abs(normal[1]), abs(normal[2])))
    {
        temp=glm::dvec3(0.0, 1.0, 0.0);
    }
    else if((abs(normal[1]) > std::max(abs(normal[0]), abs(normal[2]))))
    {		
        temp = glm::dvec3(1.0, 0.0, 0.0);
    }

    else
    {
        temp = glm::dvec3(1.0, 0.0, 0.0);		
    }
    planeDirectionX = glm::cross(normal, temp);
    planeDirectionX = planeDirectionX / glm::length(planeDirectionX);
    planeDirectionY = glm::cross(normal, planeDirectionX);
    planeDirectionY = planeDirectionY / glm::length(planeDirectionY);
    double xMin(DBL_MAX), xMax(-DBL_MAX), yMin(DBL_MAX), yMax(-DBL_MAX), zMin(DBL_MAX), zMax(-DBL_MAX);
    for (int i = 0; i < (int)seedPoints.size(); i++)
    {
        if (glm::dot(seedPoints[i],planeDirectionX) < xMin)
            xMin = glm::dot(seedPoints[i], planeDirectionX);
        if (glm::dot(seedPoints[i], planeDirectionX) > xMax)
            xMax = glm::dot(seedPoints[i], planeDirectionX);
        if (glm::dot(seedPoints[i], planeDirectionY) < yMin)
            yMin = glm::dot(seedPoints[i], planeDirectionY);
        if (glm::dot(seedPoints[i], planeDirectionY) > yMax)
            yMax = glm::dot(seedPoints[i], planeDirectionY);
        if (glm::dot(seedPoints[i], normal) < zMin)
            zMin = glm::dot(seedPoints[i], normal);
        if (glm::dot(seedPoints[i], normal) > zMax)
            zMax = glm::dot(seedPoints[i], normal);
    }
    
    glm::dvec3 planeCenter = 0.5 * ((xMax + xMin) * planeDirectionX + (yMax + yMin) * planeDirectionY + (zMax + zMin) * normal);
    planeCenter = MeasureClass::projectPointToPlane(planeCenter, plane);

    std::vector<glm::dvec3> corners(0);
    glm::dvec3 tempCorner;
    tempCorner = 0.5 * (zMin + zMax) * normal + xMin * planeDirectionX + yMin * planeDirectionY;
    corners.push_back(tempCorner);
    tempCorner = 0.5 * (zMin + zMax) * normal + xMax * planeDirectionX + yMin * planeDirectionY;
    corners.push_back(tempCorner);
    tempCorner = 0.5 * (zMin + zMax) * normal + xMin * planeDirectionX + yMax * planeDirectionY;
    corners.push_back(tempCorner);
    tempCorner = 0.5 * (zMin + zMax) * normal + xMax * planeDirectionX + yMax * planeDirectionY;
    corners.push_back(tempCorner);
    
    RectangularPlane rectPlane(normal, corners, planeCenter,PlaneType::tilted);
    result = rectPlane.createTransfo();
    return;
}

void TlScanOverseer::fitVerticalPlane(const glm::dvec3& seedPoint1, const glm::dvec3& seedPoint2, TransformationModule& result)
{

    glm::dvec3 normal = glm::cross(seedPoint1 - seedPoint2, glm::dvec3(0.0, 0.0, 1.0));
    normal = normal / glm::length(normal);
    std::vector<glm::dvec3> corners;
    glm::dvec3 tempCorner;
    tempCorner = seedPoint1;
    corners.push_back(tempCorner);
    tempCorner[2] = seedPoint2[2];
    corners.push_back(tempCorner);
    tempCorner = seedPoint2;
    tempCorner[2] = seedPoint1[2];
    corners.push_back(tempCorner);
    tempCorner[2] = seedPoint2[2];
    corners.push_back(tempCorner);
    glm::dvec3 center = 0.5 * (seedPoint1 + seedPoint2);
    
    RectangularPlane plane = RectangularPlane(normal, corners, center,PlaneType::vertical);
    result = plane.createTransfo();
    
    return;
}

void TlScanOverseer::setOfPoints(const glm::dvec3& projDirection, const glm::dvec3& startingPoint, const glm::dvec3& endPoint, const double& step, const double& threshold, std::vector<glm::dvec3>& createdPoints, std::vector<bool>& pointCreatedIsReal, const double& cosAngleThreshold, const ClippingAssembly& clipAssembly)
{
    createdPoints = std::vector<glm::dvec3>(0);
    pointCreatedIsReal = std::vector<bool>(0);
    glm::dvec3 setsOfPointsDirection = endPoint - startingPoint;
    setsOfPointsDirection /= glm::length(setsOfPointsDirection);
    int numberOfPoints = (int)(glm::length(endPoint - startingPoint) / step)+2;
    for (int i = 0; i < numberOfPoints; i++)
    {	
        //trace ray
        glm::dvec3 result;
        std::string scanName;
        if (rayTracing(projDirection, startingPoint + i * step * setsOfPointsDirection-1.1*threshold*projDirection, result, cosAngleThreshold, clipAssembly, true, scanName))
        {
            //check if the result is above threshold
            glm::dvec3 temp = glm::cross(setsOfPointsDirection, projDirection);
            temp /= glm::length(temp);
            glm::dvec3 normalPlane = glm::cross(setsOfPointsDirection, temp);
            normalPlane /= glm::length(normalPlane);
            std::vector<double> plane(0);
            plane.push_back(normalPlane[0]);
            plane.push_back(normalPlane[1]);
            plane.push_back(normalPlane[2]);
            plane.push_back(-glm::dot(normalPlane, startingPoint));

            glm::dvec3 projPoint = MeasureClass::projectPointToPlane(result, plane);
            if (glm::length(result - projPoint) < threshold)
            {
                createdPoints.push_back(result);
                pointCreatedIsReal.push_back(true);
            }
            else
            {
                createdPoints.push_back(glm::dvec3(0.0, 0.0, 0.0));
                pointCreatedIsReal.push_back(false);
            }
        }
        else
        {
            createdPoints.push_back(glm::dvec3(0.0, 0.0, 0.0));
            pointCreatedIsReal.push_back(false);
        }		
    }
    return;
}

void TlScanOverseer::setOfPointsWith4thPoint(const glm::dvec3& projDirection, const std::vector<glm::dvec3>& userPoints, const double& step, const double& threshold, std::vector<glm::dvec3>& createdPointsStart, std::vector<glm::dvec3>& createdPointsEnd, std::vector<bool>& pointCreatedIsReal, const double& cosAngleThreshold, const ClippingAssembly& clipAssembly)
{
    if ((int)userPoints.size() != 4)
        return;
    createdPointsStart = std::vector<glm::dvec3>(0);
    createdPointsEnd = std::vector<glm::dvec3>(0);
    pointCreatedIsReal = std::vector<bool>(0);
    glm::dvec3 setsOfPointsDirectionStart = userPoints[1] - userPoints[0];
    glm::dvec3 setsOfPointsDirectionEnd = userPoints[3] - userPoints[2];
    setsOfPointsDirectionStart /= glm::length(setsOfPointsDirectionStart);
    setsOfPointsDirectionEnd /= glm::length(setsOfPointsDirectionEnd);
    glm::dvec3 targetPoint1 = userPoints[2];
    glm::dvec3 targetPoint2 = userPoints[3];
    
    if (glm::dot(setsOfPointsDirectionEnd, setsOfPointsDirectionStart) < 0)
    {
        setsOfPointsDirectionEnd = -setsOfPointsDirectionEnd;
        targetPoint1 = targetPoint2;
        targetPoint2 = userPoints[2];
    }

    std::vector<double> plane;
    std::vector<glm::dvec3> planePoints(0);
    planePoints.push_back(userPoints[0]);
    planePoints.push_back(userPoints[1]);
    planePoints.push_back(targetPoint1);
    OctreeRayTracing::fitPlane(planePoints, plane);
    targetPoint2 = MeasureClass::projectPointToPlane(targetPoint2, plane);
    setsOfPointsDirectionEnd = targetPoint2 - targetPoint1;
    setsOfPointsDirectionEnd /= glm::length(setsOfPointsDirectionEnd);
    glm::dvec3 tempNormal = glm::cross(targetPoint2 - targetPoint1, projDirection);
    glm::dvec3 targetPlaneNormal = glm::cross(tempNormal, targetPoint2 - targetPoint1);
    targetPlaneNormal /= glm::length(targetPlaneNormal);
    std::vector<double> targetPlane(0);
    targetPlane.push_back(targetPlaneNormal[0]);
    targetPlane.push_back(targetPlaneNormal[1]);
    targetPlane.push_back(targetPlaneNormal[2]);
    targetPlane.push_back(-glm::dot(targetPlaneNormal,targetPoint1));


    int numberOfPoints = (int)(glm::length(userPoints[1] - userPoints[0]) / step)+2;
    for (int i = 0; i < numberOfPoints; i++)
    {
        //trace ray
        glm::dvec3 result;
        std::string scanName;
        //starter points
        if (rayTracing(-projDirection, userPoints[0] + i * step * setsOfPointsDirectionStart + 1.01 * threshold * projDirection, result, cosAngleThreshold, clipAssembly, true, scanName))
        {
            //check if the result is above threshold
            glm::dvec3 temp= glm::cross(setsOfPointsDirectionStart, projDirection);
            temp /= glm::length(temp);
            glm::dvec3 normalPlane = glm::cross(setsOfPointsDirectionStart, temp);
            normalPlane /= glm::length(normalPlane);
            std::vector<double> plane(0);
            plane.push_back(normalPlane[0]);
            plane.push_back(normalPlane[1]);
            plane.push_back(normalPlane[2]);
            plane.push_back(-glm::dot(normalPlane, userPoints[0]));

            glm::dvec3 projPoint = MeasureClass::projectPointToPlane(result, plane);
            if (glm::length(result - projPoint) < threshold)
            {
                createdPointsStart.push_back(result);
                pointCreatedIsReal.push_back(true);
            }
            else
            {
                createdPointsStart.push_back(glm::dvec3(0.0, 0.0, 0.0));
                pointCreatedIsReal.push_back(false);
            }
        }
        else
        {
            createdPointsStart.push_back(glm::dvec3(0.0, 0.0, 0.0));
            pointCreatedIsReal.push_back(false);
        }
        glm::dvec3 tempTarget = MeasureClass::projectPointToPlaneAlongVector(userPoints[0] + i * step * setsOfPointsDirectionStart, projDirection, targetPlane);
        if (rayTracing(projDirection, tempTarget - 1.01 * threshold * projDirection, result, cosAngleThreshold, clipAssembly, true, scanName))
        {
            //check if the result is above threshold
            glm::dvec3 temp = glm::cross(setsOfPointsDirectionEnd, projDirection);
            temp /= glm::length(temp);
            glm::dvec3 normalPlane = glm::cross(setsOfPointsDirectionEnd, temp);
            normalPlane /= glm::length(normalPlane);
            std::vector<double> plane(0);
            plane.push_back(normalPlane[0]);
            plane.push_back(normalPlane[1]);
            plane.push_back(normalPlane[2]);
            plane.push_back(-glm::dot(normalPlane, targetPoint1));

            glm::dvec3 projPoint = MeasureClass::projectPointToPlane(result, plane);
            if (glm::length(result - projPoint) < threshold)
            {
                createdPointsEnd.push_back(result);
            }
            else
            {
                createdPointsEnd.push_back(glm::dvec3(0.0, 0.0, 0.0));
                pointCreatedIsReal[i] = false;
            }
        }
        else
        {
            createdPointsEnd.push_back(glm::dvec3(0.0, 0.0, 0.0));
            pointCreatedIsReal[i] = false;
        }
    }
    return;
}

bool TlScanOverseer::testIndices(const int& x, const int& y, const int& z, const int& xMax, const int& yMax, const int& zMax)
{
    if ((x < 0) || (x >= xMax) || (y < 0) || (y >= yMax) || (z < 0) || (z >= zMax))
        return false;
    else return true;
}

void TlScanOverseer::naiveTorus(const GeometricBox& box, const ClippingAssembly& clippingAssembly)
{
    //get points
    
    //std::vector<glm::dvec3> dataPoints=pointsInBox(box, clippingAssembly);
    std::vector<glm::dvec3> dataPoints(0);
    findNeighbors(box.getCenter(), 3*box.getRadius(), dataPoints, clippingAssembly);
    //sample planes
    double step(0.01);
    glm::dvec3 dirX(box.getDirX()), dirY(box.getDirY()), dirZ(box.getDirZ());
    int sizeX = (int)(glm::length(box.m_corners[1] - box.m_corners[0]) / step);
    int sizeY = (int)(glm::length(box.m_corners[2] - box.m_corners[0]) / step);
    int sizeZ = (int)(glm::length(box.m_corners[3] - box.m_corners[0]) / step);
    std::vector<std::vector<std::vector<bool>>> populated(sizeX,std::vector<std::vector<bool>>(sizeY,std::vector<bool>(sizeZ,false)));
    std::vector<std::vector<std::vector<bool>>> resetPopulated = populated;
    std::vector<std::vector<std::vector<AbstractPlane>>> planes(sizeX, std::vector<std::vector<AbstractPlane>>(sizeY, std::vector<AbstractPlane>(sizeZ, AbstractPlane(glm::dvec3(1.0,0.0,0.0),glm::dvec3(0.0,0.0,0.0)))));
    std::vector<int> neighborCount(0);
    int testPopulation(0);
    for (int i = 0; i < (int)dataPoints.size(); i++)
    {
        //find voxel coordinate
        int dataX = (int)(abs(glm::dot(dirX, dataPoints[i] - box.m_corners[0])) / step);
        int dataY = (int)(abs(glm::dot(dirY, dataPoints[i] - box.m_corners[0])) / step);
        int dataZ = (int)(abs(glm::dot(dirZ, dataPoints[i] - box.m_corners[0])) / step);

        //check if wrong coordinates
        if (!testIndices(dataX, dataY, dataZ, sizeX, sizeY, sizeZ))
            continue;
        //check if voxel has already been visited
        if (populated[dataX][dataY][dataZ])
            continue;
        //otherwise, find a local plane
        std::vector<glm::dvec3> neighborPoints(0);
        findNeighbors(dataPoints[i], step, neighborPoints, clippingAssembly);
        neighborCount.push_back((int)neighborPoints.size());
        std::vector<double> plane(0);
        if (OctreeRayTracing::fitPlane(neighborPoints, plane))
        {
            AbstractPlane absPlane(glm::dvec3(plane[0], plane[1], plane[2]), MeasureClass::projectPointToPlane(box.m_corners[0] + dataX * step * dirX + dataY * step * dirY + dataZ * step * dirZ, plane));
            populated[dataX][dataY][dataZ] = true;
            planes[dataX][dataY][dataZ] = absPlane;
            testPopulation++;
        }
    }

    //test curvature
    populated = resetPopulated;
    std::vector<std::vector<std::vector<double>>> curvatures(sizeX, std::vector<std::vector<double>>(sizeY, std::vector<double>(sizeZ, 0)));
    std::vector<double> relevantCurvatures(0);
    for(int i=0;i<sizeX;i++)
        for(int j=0;j<sizeY;j++)
            for (int k = 0; k < sizeZ; k++)
            {
                if(populated[i][j][k])
                    continue;
                //get neighborcells indices
                std::vector<int> temp(3, 0);
                std::vector<std::vector<int>> move(0);
                temp[0] = -1;
                move.push_back(temp);
                temp[0] = 1;
                move.push_back(temp);
                temp[0] = 0;
                temp[1] = -1;
                move.push_back(temp);
                temp[1] = 1;
                move.push_back(temp);
                temp[1] = 0;
                temp[2] = -1;
                move.push_back(temp);
                temp[2] = 1;
                move.push_back(temp);

                int relevantNeighbors(0);
                double curv(0);
                for (int t = 0; t < (int)move.size(); t++)
                {
                    if (testIndices(i + move[t][0], j + move[t][1], k + move[t][2], sizeX, sizeY, sizeZ))
                    {
                        if (populated[i + move[t][0]][j + move[t][1]][k + move[t][2]])
                        {
                            relevantNeighbors++;
                            curv += abs(glm::dot(planes[i][j][k].m_normal, (planes[i + move[t][0]][j + move[t][1]][k + move[t][2]].m_normal)));
                        }
                    }
                }
                if (relevantNeighbors > 0)
                {
                    curv /= (double)relevantNeighbors;
                    curvatures[i][j][k] = curv;
                    relevantCurvatures.push_back(curv);
                    populated[i][j][k] = true;
                }
                    


            }
    //display curvatures
    int temp = 0;
    temp++;
}

void TlScanOverseer::fitCircle(const std::vector<glm::dvec2>& points, glm::dvec2& center, double& radius, double& error)
{
    //center the points

    glm::dvec2 centerOfMass(glm::dvec2(0.0, 0.0));
    std::vector<glm::dvec2> centeredPoints(0);
    for (int i = 0; i < (int)points.size(); i++)
    {
        centerOfMass += points[i];
    }
    centerOfMass /= (double)points.size();
    for (int i = 0; i < (int)points.size(); i++)
    {
        centeredPoints.push_back(points[i] - centerOfMass);
    }

    center = glm::dvec3(0.0, 0.0, 0.0);
    radius = 0;
    error = 0;
    //code uses this paper : https://fr.scribd.com/document/14819165/Regressions-coniques-quadriques-circulaire-spherique
    if ((int)points.size() < 3)
    {
        Logger::log(LoggerMode::rayTracingLog) << "called fitCircle with fewer than 3 points" << Logger::endl;
        return;
    }

    double d11, d20, d30, d21, d02, d03, d12;
    d11 = computeD11(centeredPoints);
    d20 = computeD20(centeredPoints);
    d30 = computeD30(centeredPoints);
    d21 = computeD21(centeredPoints);
    d02 = computeD02(centeredPoints);
    d03 = computeD03(centeredPoints);
    d12 = computeD12(centeredPoints);

    if (abs(d20 * d02 - d11 * d11) <= std::numeric_limits<double>::epsilon())
    {
        Logger::log(LoggerMode::rayTracingLog) << "trying to divide by 0 in fitCircle" << Logger::endl;
        return;
    }

    double xCenter, yCenter;
    xCenter = ((d30 + d12) * d02 - (d03 + d21) * d11) / (2 * (d20 * d02 - d11 * d11));
    yCenter = ((d03 + d21) * d20 - (d30 + d12) * d11) / (2 * (d02 * d20 - d11 * d11));
    center = glm::dvec2(xCenter, yCenter)+centerOfMass;
    double c;
    c = computeC(centeredPoints, xCenter, yCenter);
    if ((c + xCenter * xCenter + yCenter * yCenter) < 0)
    {
        Logger::log(LoggerMode::rayTracingLog) << "trying to sqrt a negative in fitCircle" << Logger::endl;
        return;
    }
    radius = sqrt(c + xCenter * xCenter + yCenter * yCenter);
    error = 0;
    for (int i = 0; i < (int)points.size(); i++)
    {
        //error += abs(radius * radius - glm::length(points[i] - center) * glm::length(points[i] - center));
        error += abs(radius - glm::length(points[i] - center));
    }
    error /= (double)points.size();
    return;
}

double TlScanOverseer::computeD11(const std::vector<glm::dvec2>& points)
{
    double first(0), second(0), third(0);
    for (int i = 0; i < (int)points.size(); i++)
    {
        first += points[i][0] * points[i][1];
        second += points[i][0];
        third += points[i][1];
    }
    return (double)points.size() * first + second * third;
}

double TlScanOverseer::computeD20(const std::vector<glm::dvec2>& points)
{
    double first(0), second(0);
    for (int i = 0; i < (int)points.size(); i++)
    {
        first += points[i][0] * points[i][0];
        second += points[i][0];
    }
    return (double)points.size() * first - second * second;
}

double TlScanOverseer::computeD30(const std::vector<glm::dvec2>& points)
{
    double first(0), second(0), third(0);
    for (int i = 0; i < (int)points.size(); i++)
    {
        first += points[i][0] * points[i][0] * points[i][0];
        second += points[i][0] * points[i][0];
        third += points[i][0];
    }
    return (double)points.size() * first - second * third;
}

double TlScanOverseer::computeD21(const std::vector<glm::dvec2>& points)
{
    double first(0), second(0), third(0);
    for (int i = 0; i < (int)points.size(); i++)
    {
        first += points[i][0] * points[i][0] * points[i][1];
        second += points[i][0] * points[i][0];
        third += points[i][1];
    }
    return (double)points.size() * first - second * third;
}

double TlScanOverseer::computeD02(const std::vector<glm::dvec2>& points)
{
    double first(0), second(0);
    for (int i = 0; i < (int)points.size(); i++)
    {
        first += points[i][1] * points[i][1];
        second += points[i][1];
    }
    return (double)points.size() * first - second * second;
}

double TlScanOverseer::computeD03(const std::vector<glm::dvec2>& points)
{
    double first(0), second(0), third(0);
    for (int i = 0; i < (int)points.size(); i++)
    {
        first += points[i][1] * points[i][1] * points[i][1];
        second += points[i][1] * points[i][1];
        third += points[i][1];
    }
    return (double)points.size() * first - second * third;
}

double TlScanOverseer::computeD12(const std::vector<glm::dvec2>& points)
{
    double first(0), second(0), third(0);
    for (int i = 0; i < (int)points.size(); i++)
    {
        first += points[i][1] * points[i][1] * points[i][0];
        second += points[i][1] * points[i][1];
        third += points[i][0];
    }
    return (double)points.size() * first - second * third;
}

double TlScanOverseer::computeC(const std::vector<glm::dvec2>& points, const double& a, const double& b)
{
    double first(0), second(0), third(0),fourth(0);
    for (int i = 0; i < (int)points.size(); i++)
    {
        first += points[i][0] * points[i][0];
        second += points[i][1] * points[i][1];
        third += points[i][0];
        fourth += points[i][1];
    }
    return double((first + second - 2 * a * third - 2 * b * fourth) / (double)points.size());
}

std::vector<glm::dvec3> TlScanOverseer::getPointsNearPlane(const std::vector<glm::dvec3>& points, const std::vector<double>& plane, const double& threshold)
{
    std::vector<glm::dvec3> result(0);
    for (int i = 0; i < (int)points.size(); i++)
    {
        if (OctreeRayTracing::pointToPlaneDistance(points[i], plane) < threshold)
            result.push_back(points[i]);
    }
    return result;
}

std::vector<glm::dvec2> TlScanOverseer::convertToPlanePoints(const std::vector<glm::dvec3>& points, const glm::dvec3& planeCenter, const glm::dvec3& dirX, const glm::dvec3& dirY)
{
    glm::dvec3 planeNormal = glm::cross(dirX, dirY);
    planeNormal /= glm::length(planeNormal);
    std::vector<double> plane(0);
    plane.push_back(planeNormal[0]);
    plane.push_back(planeNormal[1]);
    plane.push_back(planeNormal[2]);
    plane.push_back(-glm::dot(planeNormal, planeCenter));
    std::vector<glm::dvec2> result(0);
    for (int i = 0; i < (int)points.size(); i++)
    {
        glm::dvec2 temp;
        glm::dvec3 projectedPoint = MeasureClass::projectPointToPlane(points[i], plane);
        temp[0] = glm::dot(projectedPoint-planeCenter, dirX);
        temp[1] = glm::dot(projectedPoint-planeCenter, dirY);
        result.push_back(temp);
    }
    return result;
}

void TlScanOverseer::makeListOfTestPlanes(const glm::dvec3& searchCenter, const double& searchRadius, std::vector<std::vector<std::vector<double>>>& planes, std::vector<std::vector<glm::dvec3>>& planeCenters)
{
    planes = std::vector<std::vector<std::vector<double>>>(0);
    planeCenters = std::vector<std::vector<glm::dvec3>>(0);

    //sample normal directions
    std::vector<glm::dvec3> normals = OctreeRayTracing::makeDirectionList2(6);
    std::vector<glm::dvec3> normalsTest = OctreeRayTracing::makeDirectionList(6);

    //for each normal direction, samples planes to cover the box
    
    double step(0.03);
    for (int i = 0; i < (int)normals.size(); i++)
    {
        planes.push_back(std::vector<std::vector<double>>(0));
        planeCenters.push_back(std::vector<glm::dvec3>(0));
        double tempDistance(0);
        bool first(false);
        while (abs(tempDistance) < searchRadius)
        {
            planeCenters[i].push_back(searchCenter + tempDistance * normals[i]);
            std::vector<double> tempPlane(0);
            tempPlane.push_back(normals[i][0]);
            tempPlane.push_back(normals[i][1]);
            tempPlane.push_back(normals[i][2]);
            tempPlane.push_back(-glm::dot(normals[i], searchCenter + tempDistance * normals[i]));
            planes[i].push_back(tempPlane);
            if (first)
            {
                planeCenters[i].push_back(searchCenter - tempDistance * normals[i]);
                tempPlane = std::vector<double>(0);
                tempPlane.push_back(normals[i][0]);
                tempPlane.push_back(normals[i][1]);
                tempPlane.push_back(normals[i][2]);
                tempPlane.push_back(-glm::dot(normals[i], searchCenter - tempDistance * normals[i]));
                planes[i].push_back(tempPlane);
            }
            first = true;
            tempDistance += step;
        }
    }
    

    return;
}

glm::dvec3 TlScanOverseer::convertTo3DPoint(const glm::dvec2& point2D, const glm::dvec3& origin, const glm::dvec3& dirX, const glm::dvec3& dirY)
{
    return origin + point2D[0] * dirX + point2D[1] * dirY;
}

template <typename T>
std::vector<T> selectByIndices(const std::vector<T>& V, const std::vector<int>& Indices) {
    std::vector<T> result;

    for (int index : Indices) {
        if (index >= 0 && index < V.size()) {
            result.push_back(V[index]);
        }
    }
    return result;
}

bool TlScanOverseer::torusFitFromCircles(const glm::dvec3& searchOrigin, const double& searchSize, const std::vector<glm::dvec3>& dataPoints, glm::dvec3& torusCenter, double& principalRadius, double& pipeRadius, glm::dvec3& axis)
{
    //sample planes
    std::vector<std::vector<std::vector<double>>> planes(0);
    std::vector<std::vector<glm::dvec3>> planeCenters(0);
    makeListOfTestPlanes(searchOrigin, searchSize, planes, planeCenters);

    //for each plane, fit a circle	

    std::vector<std::vector<glm::dvec3>> circleCenters(0);
    std::vector<std::vector<double>> circleRadii(0);
    std::vector<std::vector<double>> circleErrors(0);
    std::vector<std::vector<int>> planePoints(0);
    std::vector<std::vector<bool>> enoughPoints(0);
    std::vector<std::vector<glm::dvec3>> circleDirX(0);
    std::vector<std::vector<glm::dvec3>> circleDirY(0);

    int dirNumber = (int)planes.size();
    int minPlanePoints(50);
    double threshold = 0.002;
    for (int i = 0; i < dirNumber; i++)
    {
        circleCenters.push_back(std::vector<glm::dvec3>(0));
        circleDirX.push_back(std::vector<glm::dvec3>(0));
        circleDirY.push_back(std::vector<glm::dvec3>(0));
        circleRadii.push_back(std::vector<double>(0));
        circleErrors.push_back(std::vector<double>(0));
        planePoints.push_back(std::vector<int>(0));
        enoughPoints.push_back(std::vector<bool>(0));
        for (int j = 0; j < (int)planes[i].size(); j++)
        {
            std::vector<glm::dvec3> planePointsCurr = getPointsNearPlane(dataPoints, planes[i][j], threshold);
            planePoints[i].push_back((int)planePointsCurr.size());
            if ((int)planePointsCurr.size() < minPlanePoints)
            {
                enoughPoints[i].push_back(false);
                circleCenters[i].push_back(glm::dvec3(0.0, 0.0, 0.0));
                circleRadii[i].push_back(0.0);
                circleErrors[i].push_back(0.0);
                circleDirX[i].push_back(glm::dvec3(0.0, 0.0, 0.0));
                circleDirY[i].push_back(glm::dvec3(0.0, 0.0, 0.0));
            }
            else
            {
                glm::dvec3 normal, dirX, dirY;
                normal[0] = planes[i][j][0];
                normal[1] = planes[i][j][1];
                normal[2] = planes[i][j][2];
                OctreeRayTracing::completeVectorToOrthonormalBasis(normal, dirX, dirY);
                std::vector<glm::dvec2> planePoints2D = convertToPlanePoints(planePointsCurr, planeCenters[i][j], dirX, dirY);
                glm::dvec2 circleCenter2D;
                double circleRadius, error;
                fitCircle(planePoints2D, circleCenter2D, circleRadius, error);
                circleErrors[i].push_back(error);
                circleRadii[i].push_back(circleRadius);
                circleCenters[i].push_back(planeCenters[i][j] + circleCenter2D[0] * dirX + circleCenter2D[1] * dirY);
                circleDirX[i].push_back(dirX);
                circleDirY[i].push_back(dirY);
                enoughPoints[i].push_back(true);
            }
        }
    }

    //keep best circles
    std::vector<glm::dvec3> bestCircleCenters(std::vector<glm::dvec3>(0)), bestCircleDirX(std::vector<glm::dvec3>(0)), bestCircleDirY(std::vector<glm::dvec3>(0));
    std::vector<double> bestRadii(std::vector<double>(0)),bestErrors(std::vector<double>(0));
    double bestErrorThreshold(0.005);
    std::vector<int> bestPlanePoints(0);
    for (int i = 0; i < dirNumber; i++)
    {
        double bestError = DBL_MAX;
        glm::dvec3 currBestCenter, currBestDirX, currBestDirY;
        double currBestRadius;
        int currBestX(0), currBestY(0),currBestPlanePoints(0);
        for (int j = 0; j < (int)planes[i].size(); j++)
        {
            if (!enoughPoints[i][j])
                continue;
            if (circleErrors[i][j] < bestError)
            {
                bestError = circleErrors[i][j];
                currBestCenter = circleCenters[i][j];
                currBestDirX = circleDirX[i][j];
                currBestDirY = circleDirY[i][j];
                currBestRadius = circleRadii[i][j];
                currBestX = i;
                currBestY = j;
                currBestPlanePoints = planePoints[i][j];
            }
        }
        if (bestError < bestErrorThreshold)
        {
            bestCircleCenters.push_back(currBestCenter);
            bestCircleDirX.push_back(currBestDirX);
            bestCircleDirY.push_back(currBestDirY);
            bestRadii.push_back(currBestRadius);
            bestErrors.push_back(bestError);
            bestPlanePoints.push_back(currBestPlanePoints);
        }
    }

    //fit circle with the circle centers
    if ((int)bestCircleCenters.size() < 3)
        return false;

    std::vector<glm::dvec3> finalCenters(0), finalDirX(0), finalDirY(0);
    std::vector<double> finalRadii(0);
    int keepNumber(std::min(40,(int)bestCircleCenters.size()));
    std::vector<int> keepIndices = getTopIndices(bestErrors, keepNumber);
    for (int i = 0; i < keepNumber; i++)
    {
        finalCenters.push_back(bestCircleCenters[keepIndices[i]]);
        finalDirX.push_back(bestCircleDirX[keepIndices[i]]);
        finalDirY.push_back(bestCircleDirY[keepIndices[i]]);
        finalRadii.push_back(bestRadii[keepIndices[i]]);
    }

    std::vector<int> removedOutliers = filterOutlierValues(finalRadii);
    finalCenters = selectByIndices(finalCenters, removedOutliers);
    finalDirX = selectByIndices(finalDirX, removedOutliers);
    finalDirY = selectByIndices(finalDirY, removedOutliers);
    finalRadii = selectByIndices<double>(finalRadii, removedOutliers);

    std::vector<double> principalPlane(4, 0);
    glm::dvec3 dirX, dirY;
    if ((int)finalCenters.size() < 3)
        return false;
    OctreeRayTracing::fitPlane(finalCenters, principalPlane);
    glm::dvec3 principalNormal;
    principalNormal[0] = principalPlane[0];
    principalNormal[1] = principalPlane[1];
    principalNormal[2] = principalPlane[2];
    principalNormal /= glm::length(principalNormal);
    OctreeRayTracing::completeVectorToOrthonormalBasis(principalNormal, dirX, dirY);
    glm::dvec3 principalPlaneCenter = MeasureClass::projectPointToPlane(finalCenters[0], principalPlane);
    std::vector<glm::dvec2> planePoints2D = convertToPlanePoints(finalCenters, principalPlaneCenter, dirX, dirY);
    glm::dvec2 circleCenter2D;
    double circleRadius, error;
    fitCircle(planePoints2D, circleCenter2D, circleRadius, error);
    torusCenter = principalPlaneCenter + circleCenter2D[0] * dirX + circleCenter2D[1] * dirY;
    principalRadius = circleRadius;
    pipeRadius = 0;
    for (int i = 0; i < (int)finalRadii.size(); i++)
    {
        pipeRadius += finalRadii[i];
    }
    pipeRadius /= (double)finalRadii.size();
    axis = principalNormal;
    return true;
}

bool TlScanOverseer::torusFitFromCirclesPrep(const glm::dvec3& point, const ClippingAssembly& clipAssembly, glm::dvec3& torusCenter, double& principalRadius, double& pipeRadius, glm::dvec3& axis)
{
    double searchSize = 0.2;
    std::vector<glm::dvec3> dataPoints(0);
    findNeighbors(point, searchSize, dataPoints, clipAssembly);
    return torusFitFromCircles(point, searchSize, dataPoints, torusCenter, principalRadius, pipeRadius,axis);
}

void TlScanOverseer::fitCircleTest()
{
    std::vector<glm::dvec2> circlePoints(0);
    glm::dvec2 center(4.0, 8.0);
    double radius(0.25);
    int numberOfPoints(50);
    double error(0);
    double detectedRadius(0);

    glm::dvec2 detectedCenter(0.0, 0.0);

    for (int i = 0; i < numberOfPoints; i++)
    {
        circlePoints.push_back(glm::dvec2(center[0]+radius * cos((double)i/50.0), center[1]+radius * sin((double)i/50.0)));
    }
    fitCircle(circlePoints, detectedCenter, detectedRadius, error);
    bool wait(false);
    wait = true;
    return;
}

std::vector<int> TlScanOverseer::getTopIndices(const std::vector<double>& values, int k) {
    std::priority_queue<IndexedValue> pq;
    std::vector<int> result;

    for (int i = 0; i < values.size(); i++) {
        pq.push({ values[i], i });
        if (pq.size() > k) {
            pq.pop();
        }
    }

    while (!pq.empty()) {
        result.push_back(pq.top().index);
        pq.pop();
    }

    // The indices are currently in reverse order, so reverse them.
    std::reverse(result.begin(), result.end());

    return result;
}

std::vector<int> TlScanOverseer::filterOutlierValues(const std::vector<double>& values)
{
    double median = findMedian(values);
    std::vector<int> result(0);
    double threshold(0.02);
    for (int i = 0; i < (int)values.size(); i++)
    {
        if (abs(values[i] - median) < threshold)
            result.push_back(i);
    }
    return result;
}

double TlScanOverseer::findMedian(const std::vector<double>& values) {
    // Create a copy of the input list to avoid modifying the original
    std::vector<double> sortedValues = values;

    // Sort the values in ascending order
    std::sort(sortedValues.begin(), sortedValues.end());

    size_t n = sortedValues.size();

    if (n % 2 == 0) {
        // If the number of values is even, average the middle two values
        size_t middle1 = (n - 1) / 2;
        size_t middle2 = n / 2;
        return (sortedValues[middle1] + sortedValues[middle2]) / 2.0;
    }
    else {
        // If the number of values is odd, return the middle value
        size_t middle = n / 2;
        return sortedValues[middle];
    }
}
