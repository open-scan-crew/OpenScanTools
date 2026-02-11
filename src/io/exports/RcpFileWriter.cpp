#include "io/exports/RcpFileWriter.h"
#include "models/pointCloud/PointXYZIRGB.h"
#include "utils/Logger.h"

#include <foundation/RCQuaternion.h>

using namespace Autodesk::RealityComputing::Foundation;
using namespace Autodesk::RealityComputing::Data;
using namespace Autodesk::RealityComputing::ImportExport;

RcpFileWriter::RcpFileWriter(const std::filesystem::path& file_path, RCSharedPtr<RCProjectImportSession> projectImportSession) noexcept
    : IScanFileWriter(file_path)
    , m_projectImportSession(projectImportSession)
    , total_point_count_(0)
    , m_hasColor(false)
    , m_hasIntensity(false)
    , m_exportDensity(-1.0)
{ }

RcpFileWriter::~RcpFileWriter()
{
    finalizeProject();
}

void projectProgressCallback(const RCIOStatus& status)
{
    Logger::log(LoggerMode::IOLog) << "ReCap project processed at " << (int)status.getTotalProgress() << "%" << Logger::endl;
}

void projectCompletionCallback(RCCode errorCode, const RCBuffer<RCScanImportStatus>& importStatus)
{
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
}

void scanProgressCallback(const RCIOStatus& status)
{
    Logger::log(LoggerMode::IOLog) << "ReCap scan processed at " << (int)status.getTotalProgress() << "%" << Logger::endl;
}

void scanCompletionCallback(const RCScanImportStatus& importStatus)
{
   auto scanName = importStatus.scanName;

    if (importStatus.code == RCCode::OK)
        Logger::log(LoggerMode::IOLog) << (std::string)scanName << " import is completed." << Logger::endl;
    else
        Logger::log(LoggerMode::IOLog) << (std::string)scanName << " import failed with return code " << (int)importStatus.code << Logger::endl;
}

bool RcpFileWriter::getWriter(const std::filesystem::path& outPath, const std::wstring& projectName, std::wstring& log, IScanFileWriter** writer)
{
    RCCode errorCode;
    std::filesystem::path project_path = outPath / projectName;
    RCString rc_project_path(project_path.wstring());
    RCImportProgressCallbackPtr projectProgressCallbackPtr = projectProgressCallback;
    RCImportCompletionCallbackPtr projectCompletionCallbackPtr = projectCompletionCallback;
    RCSharedPtr<RCProjectImportSession> projectImportSession = RCProjectImportSession::init(rc_project_path, RCFileMode::New, errorCode, projectProgressCallbackPtr,
        projectCompletionCallbackPtr);

    if (errorCode != RCCode::OK || projectImportSession == nullptr)
    {
        log += L"Failed to init the ReCap project, error code ";
        log += (int)errorCode;
        return false;
    }

    *writer = new RcpFileWriter(project_path, projectImportSession);

    return true;
}

uint64_t RcpFileWriter::getTotalPointCount() const
{
    return total_point_count_;
}

uint64_t RcpFileWriter::getScanPointCount() const
{
    return m_scanHeader.pointCount;
}

tls::ScanHeader RcpFileWriter::getLastScanHeader() const
{
    return m_scanHeader;
}

FileType RcpFileWriter::getType() const
{
    return FileType::RCP;
}

bool RcpFileWriter::appendPointCloud(const tls::ScanHeader& info, const TransformationModule& transfo)
{
    m_scanHeader = info;
    m_scanHeader.pointCount = 0;
    scan_transfo = transfo;

    const glm::dquat q = scan_transfo.getOrientation();
    const glm::dvec3 t = scan_transfo.getCenter();
    m_scanHeader.transfo.quaternion[0] = q.x;
    m_scanHeader.transfo.quaternion[1] = q.y;
    m_scanHeader.transfo.quaternion[2] = q.z;
    m_scanHeader.transfo.quaternion[3] = q.w;
    m_scanHeader.transfo.translation[0] = t.x;
    m_scanHeader.transfo.translation[1] = t.y;
    m_scanHeader.transfo.translation[2] = t.z;

    // NOTE(robin) - The scan is really appended when we call finalizePointCloud() because we want to know if there is more than 0 points in the scan.
    // - The Recap API does not permit to cancel a scan with 0 points.

    m_hasIntensity = (info.format == tls::PointFormat::TL_POINT_XYZ_I || info.format == tls::PointFormat::TL_POINT_XYZ_I_RGB) ? true : false;
    m_hasColor = (info.format == tls::PointFormat::TL_POINT_XYZ_RGB || info.format == tls::PointFormat::TL_POINT_XYZ_I_RGB) ? true : false;
    return true;
}

bool RcpFileWriter::addPoints(PointXYZIRGB const* src_buf, uint64_t src_size)
{
    // NOTE - When the points are inserted in local space, we choose to apply the transform here.
    //      - The Recap API allow to apply the transform when we process the scan, but because 
    //        the writer can also be used with global coordinates (via mergePoints()) we have 
    //        to choose one space or an other.
    return mergePoints(src_buf, src_size, scan_transfo, m_scanHeader.format);
}

bool RcpFileWriter::mergePoints(PointXYZIRGB const* src_buf, uint64_t src_size, const TransformationModule& src_transfo, tls::PointFormat src_format)
{
    if (src_format == tls::PointFormat::TL_POINT_FORMAT_UNDEFINED)
        return false;

    m_scanHeader.pointCount += src_size;

    // The destination scan transform is stored in metadata. Points should therefore be encoded
    // in destination local coordinates.
    glm::dmat4 transfo_mat = scan_transfo.getInverseTransformation() * src_transfo.getTransformation();

    auto rcBuffer = std::make_shared<RCPointBuffer>();
    rcBuffer->setCoordinateType(RCCoordinateType::Cartesian);
    //rcBuffer->setCoordinateSystem() is deprecated

    if (m_hasIntensity)
        rcBuffer->addAttribute(RCAttributeType::Intensity);
    if (m_hasColor)
        rcBuffer->addAttribute(RCAttributeType::Color);

    rcBuffer->resize((unsigned int)src_size);

    for (unsigned int i = 0; i < src_size; ++i)
    {
        glm::dvec4 gPt = transfo_mat * glm::dvec4(src_buf[i].x, src_buf[i].y, src_buf[i].z, 1.0);
        rcBuffer->setPositionAt(i, RCVector3d(gPt.x, gPt.y, gPt.z));
    }

    if (m_hasIntensity)
    {
        for (unsigned int i = 0; i < src_size; ++i)
            rcBuffer->setIntensityAt(i, src_buf[i].i);
    }

    if (m_hasColor)
    {
        for (unsigned int i = 0; i < src_size; ++i)
            rcBuffer->setColorAt(i, RCVector4ub(src_buf[i].r, src_buf[i].g, src_buf[i].b, 255));
    }

    m_pointBuffers.push_back(*rcBuffer);

    return true;
}

bool RcpFileWriter::finalizePointCloud()
{
    // NOTE(robin) - This is not the exact scan point count, the processScan() can decimate the scan
    //               but we do not have this information with the scanCompletionCallback
    if (m_scanHeader.pointCount > 0)
    {
        RCScanMetadata scanMetadata;

        // Estimate for the progress bar
        scanMetadata.estimatedPointCount = m_scanHeader.pointCount;
        total_point_count_ += m_scanHeader.pointCount;

        TransformationModule scan_transfo_with_offset(scan_transfo);
        scan_transfo_with_offset.addGlobalTranslation(post_translation_);
        const glm::dquat q = scan_transfo_with_offset.getOrientation();
        const glm::dvec3 t = scan_transfo_with_offset.getCenter();

        m_scanHeader.transfo.quaternion[0] = q.x;
        m_scanHeader.transfo.quaternion[1] = q.y;
        m_scanHeader.transfo.quaternion[2] = q.z;
        m_scanHeader.transfo.quaternion[3] = q.w;
        m_scanHeader.transfo.translation[0] = t.x;
        m_scanHeader.transfo.translation[1] = t.y;
        m_scanHeader.transfo.translation[2] = t.z;

        RCTransform transform;
        transform.setTranslation(RCVector3d(t.x, t.y, t.z));
        const RCRotationMatrix rotation = RCQuaternion(q.w, RCVector3d(q.x, q.y, q.z)).toMatrix();
        transform.setRotation(rotation);
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

        m_projectImportSession->processScan(scanProgressCallback, scanCompletionCallback);
    }
    return true;
}

void RcpFileWriter::finalizeProject()
{
    if (m_sessionFinalized || !m_projectImportSession)
        return;

    // No more scans to be imported
    m_projectImportSession->endSession();

    // synchronize the importing process
    m_projectImportSession->waitTillFinished();
    m_sessionFinalized = true;
}

void RcpFileWriter::setExportDensity(double density)
{
    m_exportDensity = density;
}
