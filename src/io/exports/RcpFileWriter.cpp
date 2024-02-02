#include "io/exports/RcpFileWriter.h"
#include "models/pointCloud/PointXYZIRGB.h"
#include "utils/time.h"
#include "utils/math/trigo.h"
#include "utils/Logger.h"
#include "utils/Utils.h"

#include <foundation/RCQuaternion.h>

using namespace Autodesk::RealityComputing::Foundation;
using namespace Autodesk::RealityComputing::Data;
using namespace Autodesk::RealityComputing::ImportExport;

RcpFileWriter::RcpFileWriter(RCSharedPtr<RCProjectImportSession> projectImportSession) noexcept
    : IScanFileWriter("")
    , m_projectImportSession(projectImportSession)
    , m_hasColor(false)
    , m_hasIntensity(false)
    , m_exportDensity(-1.0)
{ }

RcpFileWriter::~RcpFileWriter()
{
    // No more scans to be imported
    m_projectImportSession->endSession();

    // synchronize the importing process
    m_projectImportSession->waitTillFinished();
}

void projectProgressCallback(const RCIOStatus& status)
{
#ifdef LOGGER_H
    Logger::log(LoggerMode::IOLog) << "ReCap project processed at " << (int)status.getTotalProgress() << "%" << Logger::endl;
#endif // LOGGER_H
}

void projectCompletionCallback(RCCode errorCode, const RCBuffer<RCScanImportStatus>& importStatus)
{
#ifdef LOGGER_H
   if (errorCode == RCCode::OK)
        Logger::log(LoggerMode::IOLog) << "RCProject export is completed." << Logger::endl;
    else
    {
        Logger::log(LoggerMode::IOLog) << "RCProject export failed with return code " << (int)errorCode << Logger::endl;
        Logger::log(LoggerMode::IOLog) << "The scans who failed are :" << Logger::endl;
        for (auto status : importStatus)
        {
            if (status.code != RCCode::OK)
                Logger::log(LoggerMode::IOLog) << (std::string)status.scanName << " : code " << (int)status.code << Logger::endl;
        }
    }
#endif // LOGGER_H
}

void scanProgressCallback(const RCIOStatus& status)
{
#ifdef LOGGER_H
    Logger::log(LoggerMode::IOLog) << "ReCap scan processed at " << (int)status.getTotalProgress() << "%" << Logger::endl;
#endif // LOGGER_H
}

void scanCompletionCallback(const RCScanImportStatus& importStatus)
{
#ifdef LOGGER_H
   auto scanName = importStatus.scanName;

    if (importStatus.code == RCCode::OK)
        Logger::log(LoggerMode::IOLog) << (std::string)scanName << " import is completed." << Logger::endl;
    else
        Logger::log(LoggerMode::IOLog) << (std::string)scanName << " import failed with return code " << (int)importStatus.code << Logger::endl;
#endif // LOGGER_H
}

bool RcpFileWriter::getWriter(const std::filesystem::path& outPath, const std::wstring& projectName, std::wstring& log, IScanFileWriter** writer)
{
    RCCode errorCode;
    RCString projectPath(outPath.wstring());
    projectPath += "/";
    projectPath += projectName;
    projectPath += ".rcp";
    RCImportProgressCallbackPtr projectProgressCallbackPtr = projectProgressCallback;
    RCImportCompletionCallbackPtr projectCompletionCallbackPtr = projectCompletionCallback;
    RCSharedPtr<RCProjectImportSession> projectImportSession = RCProjectImportSession::init(projectPath, RCFileMode::New, errorCode, projectProgressCallbackPtr,
        projectCompletionCallbackPtr);

    if (errorCode != RCCode::OK || projectImportSession == nullptr)
    {
        log += L"Failed to init the ReCap project, error code ";
        log += (int)errorCode;
        return false;
    }

    *writer = new RcpFileWriter(projectImportSession);

    return true;
}

FileType RcpFileWriter::getType() const
{
    return FileType::RCP;
}

bool RcpFileWriter::appendPointCloud(const tls::ScanHeader& info)
{
    m_scanHeader = info;
    m_scanPointCount = 0;
    // NOTE(robin) - The scan is really appended when we call flushWrite() because we want to know if there is more than 0 points in the scan.
    // - The Recap API does not permit to cancel a scan with 0 points.

    m_hasIntensity = (info.format == tls::PointFormat::TL_POINT_XYZ_I || info.format == tls::PointFormat::TL_POINT_XYZ_I_RGB) ? true : false;
    m_hasColor = (info.format == tls::PointFormat::TL_POINT_XYZ_RGB || info.format == tls::PointFormat::TL_POINT_XYZ_I_RGB) ? true : false;
    return true;
}

bool RcpFileWriter::addPoints(PointXYZIRGB const* srcBuf, uint64_t srcSize)
{
    // NOTE - When the points are inserted in local space, we choose to apply the transform here.
    //      - The Recap API allow to apply the transform when we process the scan, but because 
    //        the writer can also be used with global coordinates (via mergePoints()) we have 
    //        to choose one space or an other.
    glm::dmat4 transfo = tls::math::getTransformDMatrix(m_scanHeader.transfo.translation, m_scanHeader.transfo.quaternion);

    //return mergePoints(srcBuf, srcSize, glm::dmat4(1.0), m_scanHeader.format);
    return mergePoints(srcBuf, srcSize, transfo, m_scanHeader.format);
}

// NOTE - equivalent of IPointCloudWriter::addPoints_localSrc()
bool RcpFileWriter::mergePoints(PointXYZIRGB const* srcBuf, uint64_t srcSize, const glm::dmat4& srcTransfo, tls::PointFormat srcFormat)
{
    m_scanPointCount += srcSize;

    glm::dmat4 matrix = srcTransfo;

    auto rcBuffer = std::make_shared<RCPointBuffer>();
    rcBuffer->setCoordinateType(RCCoordinateType::Cartesian);
    //rcBuffer->setCoordinateSystem() is deprecated

    if (m_hasIntensity)
        rcBuffer->addAttribute(RCAttributeType::Intensity);
    if (m_hasColor)
        rcBuffer->addAttribute(RCAttributeType::Color);

    rcBuffer->resize((unsigned int)srcSize);

    for (unsigned int i = 0; i < srcSize; ++i)
    {
        glm::dvec4 gPt = matrix * glm::dvec4(srcBuf[i].x, srcBuf[i].y, srcBuf[i].z, 1.0);
        rcBuffer->setPositionAt(i, RCVector3d(gPt.x, gPt.y, gPt.z));
    }

    if (m_hasIntensity)
    {
        for (unsigned int i = 0; i < srcSize; ++i)
            rcBuffer->setIntensityAt(i, srcBuf[i].i);
    }

    if (m_hasColor)
    {
        for (unsigned int i = 0; i < srcSize; ++i)
            rcBuffer->setColorAt(i, RCVector4ub(srcBuf[i].r, srcBuf[i].g, srcBuf[i].b, 255));
    }

    m_pointBuffers.push_back(*rcBuffer);

    return true;
}

bool RcpFileWriter::flushWrite()
{
    // NOTE(robin) - This is not the exact scan point count, the processScan() can decimate the scan
    //               but we do not have this information with the scanCompletionCallback
    if (m_scanPointCount > 0)
    {
        RCScanMetadata scanMetadata;

        // Estimate for the progress bar
        m_totalPointCount += m_scanPointCount;
        scanMetadata.estimatedPointCount = m_scanPointCount;

        RCTransform transform;
        //RCVector3d translation(m_scanHeader.transfo.translation[0], m_scanHeader.transfo.translation[1], m_scanHeader.transfo.translation[2]);
        //transform.setTranslation(translation);
        //RCRotationMatrix rotation = RCQuaternion(m_scanHeader.transfo.quaternion[3], RCVector3d(m_scanHeader.transfo.quaternion[0], m_scanHeader.transfo.quaternion[1], m_scanHeader.transfo.quaternion[2])).toMatrix();

        //transform.setRotation(rotation);
        transform.toRowMajor(scanMetadata.globalTransformation);

        std::wstring wname = m_scanHeader.name;
        scanMetadata.name = wname;
        scanMetadata.provider = RCScanProvider::ExternalPlugin;
        scanMetadata.coordinateSystem = L"";
        scanMetadata.isStructured = false;

        RCScanImportOptions options;
        options.pointDistanceInMM = (int)m_exportDensity;
        RCCode err = m_projectImportSession->addScan(scanMetadata, options);
        if (err != RCCode::OK)
        {
#ifdef LOGGER_H
            Logger::log(LoggerMode::IOLog) << "Error: failed to append a new scan (" << m_scanHeader.name << ") to the RCP project." << Logger::endl;
#endif // LOGGER_H
            return false;
        }

        // append the point buffers
        for (RCPointBuffer& rcBuffer : m_pointBuffers)
        {
            m_projectImportSession->addPointsToScan(rcBuffer);
        }
        m_pointBuffers.clear();
        m_scanPointCount = 0;

        m_projectImportSession->processScan(scanProgressCallback, scanCompletionCallback);
    }
    return true;
}

void RcpFileWriter::setExportDensity(double density)
{
    m_exportDensity = density;
}