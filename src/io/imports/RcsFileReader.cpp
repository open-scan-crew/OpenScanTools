#include "io/imports/RcsFileReader.h"
#include "models/pointCloud/PointXYZIRGB.h"
#include "utils/time.h"
#include "utils/logger.h"

// SDK Autodesk ReCap
#include <data/RCScan.h>
#include <data/RCPointIteratorSettings.h>
#include <data/IRCPointIterator.h>
#include <foundation/RCTransform.h>
#include <foundation/RCQuaternion.h>
#include <foundation/RCBuffer.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>

using namespace Autodesk::RealityComputing::Foundation;
using namespace Autodesk::RealityComputing::Data;

namespace
{
RcsFileReader::ImportTransformMode getImportTransformModeFromEnv()
{
    const char* mode = std::getenv("OPENSCANTOOLS_RCS_IMPORT_MODE");
    if (mode == nullptr)
        return RcsFileReader::ImportTransformMode::Auto;

    std::string lower(mode);
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return (char)std::tolower(c); });

    if (lower == "global")
        return RcsFileReader::ImportTransformMode::PointsAreGlobal;
    if (lower == "local")
        return RcsFileReader::ImportTransformMode::PointsAreLocal;

    return RcsFileReader::ImportTransformMode::Auto;
}

const char* toString(RcsFileReader::ImportTransformMode mode)
{
    switch (mode)
    {
    case RcsFileReader::ImportTransformMode::PointsAreGlobal:
        return "global";
    case RcsFileReader::ImportTransformMode::PointsAreLocal:
        return "local";
    case RcsFileReader::ImportTransformMode::Auto:
    default:
        return "auto";
    }
}
}

bool RcsFileReader::getReader(const std::filesystem::path& filepath, std::wstring log, IScanFileReader** reader)
{
    try
    {
        *reader = new RcsFileReader(filepath, true, false);
    }
    catch (std::exception& e)
    {
        Logger::log(LoggerMode::IOLog) << e.what() << Logger::endl;
        //log += std::wstring(e.what());
        return false;
    }

    return true;
}

RcsFileReader::RcsFileReader(RCSharedPtr<RCScan> rcScan)
    : IScanFileReader("")
    , m_rcScan(rcScan)
    , m_pointCount(0)
    , m_visiblePointsOnly(true)
    , m_userEdits(true)
    , m_currentPointIndex(0)
{
    m_importTransformMode = getImportTransformModeFromEnv();
    initHeaders();
}

RcsFileReader::RcsFileReader(const std::filesystem::path& filepath, bool visiblePointsOnly, bool userEdits)
    : IScanFileReader(filepath)
    , m_pointCount(0)
    , m_visiblePointsOnly(visiblePointsOnly)
    , m_userEdits(userEdits)
    , m_currentPointIndex(0)
{
    m_importTransformMode = getImportTransformModeFromEnv();
    RCString path(filepath);
    auto rcUserEdits = m_userEdits ? RCProjectUserEdits::All : RCProjectUserEdits::None;
    RCCode errorCode;
    m_rcScan = RCScan::loadFile(path, RCFileAccess::ReadOnly, errorCode);
    if (m_rcScan == nullptr)
    {
        char msg[1024];
        sprintf(msg, "Failed to load ReCap scan %ls (error code %d)\n", filepath.c_str(), errorCode);
        throw (std::exception::exception(msg));
    }

    initHeaders();
}

void RcsFileReader::initHeaders()
{
    if (m_rcScan == nullptr)
        return;

    // File Header (because we need one, but the main info is that there is only 1 scan)
    m_fileHeader.creationDate = 0;
    Logger::log(LoggerMode::IOLog) << "ReCap scan ID: " << (std::string)m_rcScan->getScanId() << Logger::endl;
    m_fileHeader.guid = xg::Guid(m_rcScan->getScanId().getString());
    m_fileHeader.scanCount = 1;
    m_fileHeader.version = tls::FileVersion::V_UNKNOWN;

    // Scan Header (the real infos)
    m_scanHeader.guid = xg::Guid(m_rcScan->getScanId().getString());
    m_scanHeader.acquisitionDate = 0; // no date
    m_scanHeader.name = m_rcScan->getName();
    m_scanHeader.sensorModel = m_rcScan->getScanProvider();
    m_scanHeader.sensorSerialNumber = L"Not provided";
    m_scanHeader.transfo = tls::Transformation{ { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 0.0 } };

    RCTransform rcTransfo;
    bool result = m_rcScan->getFullTransform(rcTransfo);
    if (result)
    {
        auto rcQuat = rcTransfo.getRotation().toQuaternion();
        m_scanHeader.transfo.quaternion[0] = rcQuat.values.x;
        m_scanHeader.transfo.quaternion[1] = rcQuat.values.y;
        m_scanHeader.transfo.quaternion[2] = rcQuat.values.z;
        m_scanHeader.transfo.quaternion[3] = rcQuat.values.w;
        transfo_.setRotation(glm::dquat(rcQuat.values.w, rcQuat.values.x, rcQuat.values.y, rcQuat.values.z));
        const auto& tr = rcTransfo.getTranslation();
        m_scanHeader.transfo.translation[0] = tr.x;
        m_scanHeader.transfo.translation[1] = tr.y;
        m_scanHeader.transfo.translation[2] = tr.z;
        transfo_.setPosition(glm::dvec3(tr.x, tr.y, tr.z));
    }

    auto rcbbox = m_rcScan->getBoundingBox();
    const auto& bboxMin = rcbbox.getMin();
    const auto& bboxMax = rcbbox.getMax();
    m_scanHeader.limits = { (float)bboxMin.x, (float)bboxMax.x,
        (float)bboxMin.y, (float)bboxMax.y,
        (float)bboxMin.z, (float)bboxMax.z };
    m_scanHeader.pointCount = m_rcScan->getNumberOfPoints();
    m_pointCount = m_scanHeader.pointCount;
    m_scanHeader.precision = tls::PrecisionType::TL_OCTREE_10UM;
    if (m_rcScan->hasIntensities())
    {
        if (m_rcScan->hasColors())
            m_scanHeader.format = tls::PointFormat::TL_POINT_XYZ_I_RGB;
        else
            m_scanHeader.format = tls::PointFormat::TL_POINT_XYZ_I;
    }
    else
    {
        if (m_rcScan->hasColors())
            m_scanHeader.format = tls::PointFormat::TL_POINT_XYZ_RGB;
        else
            m_scanHeader.format = tls::PointFormat::TL_POINT_FORMAT_UNDEFINED;
    }
}

RcsFileReader::~RcsFileReader()
{
}

FileType RcsFileReader::getType() const
{
    return FileType::RCS;
}

uint32_t RcsFileReader::getScanCount() const
{
    return 1;
}

uint64_t RcsFileReader::getTotalPoints() const
{
    return m_pointCount;
}

tls::FileHeader RcsFileReader::getTlsHeader() const
{
    return m_fileHeader;
}

tls::ScanHeader RcsFileReader::getTlsScanHeader(uint32_t scanNumber) const
{
    return m_scanHeader;
}

bool RcsFileReader::startReadingScan(uint32_t _scanNumber)
{
    if (_scanNumber > 0)
        return false;

    m_currentPointIndex = 0;

    m_applyInverseTransform = true;

    if (m_importTransformMode == ImportTransformMode::PointsAreGlobal)
    {
        m_applyInverseTransform = true;
    }
    else if (m_importTransformMode == ImportTransformMode::PointsAreLocal)
    {
        m_applyInverseTransform = false;
    }
    else
    {
        // Compatibility mode: detect if points look already local or global.
        // If the point cloud is already local, applying inverse transform would misplace scans.
        Autodesk::RealityComputing::Data::RCPointIteratorSettings settings;
        settings.setDensity(-1.0);
        settings.setIsVisiblePointsOnly(m_visiblePointsOnly);

        auto itPt = m_rcScan->createPointIterator(settings);
        bool validPt = itPt->moveToPoint(0);
        if (!validPt)
            return false;

        const glm::dvec3 tr(m_scanHeader.transfo.translation[0], m_scanHeader.transfo.translation[1], m_scanHeader.transfo.translation[2]);
        constexpr uint32_t sampleCount = 128;
        uint32_t sampled = 0;
        double distToOrigin = 0.0;
        double distToTranslation = 0.0;

        while (validPt && sampled < sampleCount)
        {
            const auto& pos = itPt->getPoint().getPosition();
            glm::dvec3 p(pos.x, pos.y, pos.z);
            distToOrigin += glm::length(p);
            distToTranslation += glm::length(p - tr);
            ++sampled;
            validPt = itPt->moveToNextPoint();
        }

        if (sampled > 0)
        {
            // If points are closer to the scan translation than to the origin,
            // they are probably already in global coordinates.
            m_applyInverseTransform = distToTranslation <= distToOrigin;
        }
    }

    Logger::log(LoggerMode::IOLog) << "RCS import mode=" << toString(m_importTransformMode)
        << ", apply inverse transform=" << (m_applyInverseTransform ? "true" : "false") << Logger::endl;

    return true;
}

bool RcsFileReader::readPoints(PointXYZIRGB* dstBuf, uint64_t bufSize, uint64_t& readCount)
{
    Autodesk::RealityComputing::Data::RCPointIteratorSettings settings;
    settings.setDensity(-1.0); // highest density
    settings.setIsVisiblePointsOnly(m_visiblePointsOnly);

    glm::dmat4 inv_transfo = transfo_.getInverseTransformation();

    // Method Point by Point
    {
        auto itPt = m_rcScan->createPointIterator(settings);
        bool validPt = itPt->moveToPoint(m_currentPointIndex);
        if (validPt == false)
        {
            readCount = 0;
            return false;
        }
        uint64_t localIndex = 0;
        while (validPt && localIndex < bufSize)
        {
            auto& pt = itPt->getPoint();
            auto& pos = pt.getPosition();

            glm::dvec4 glPos(pos.x, pos.y, pos.z, 1.0);
            if (m_applyInverseTransform)
                glPos = inv_transfo * glPos;

            dstBuf[localIndex].x = (float)(glPos.x);
            dstBuf[localIndex].y = (float)(glPos.y);
            dstBuf[localIndex].z = (float)(glPos.z);

            if (m_rcScan->hasColors())
            {
                auto color = pt.getColor();
                dstBuf[localIndex].r = color.x;
                dstBuf[localIndex].g = color.y;
                dstBuf[localIndex].b = color.z;
            }
            if (m_rcScan->hasIntensities())
            {
                float intensity = pt.getIntensity();
                float scaleI = 255 / m_rcScan->getIntensityMaxValue();
                float minI = m_rcScan->getIntensityMinValue(); // is it always 0 ?
                dstBuf[localIndex].i = (uint8_t)(intensity * scaleI);
            }
            localIndex++;
            validPt = itPt->moveToNextPoint();
        }
        m_currentPointIndex = itPt->getCurrentIndex();
        readCount = localIndex;
        return true;
    }

    // Method Point by Batch
    /*
    {
        auto itBatchPt = m_rcScan->createPointBatchIterator(settings);
        itBatchPt->moveToPoint(m_currentPointIndex);
        while (itBatchPt->moveToNextBatch(bufSize))
        {
            auto buffer = itBatchPt->getPoints();
            for (uint32_t i = 0; i < buffer.getSize(); ++i)
            {
                auto& pos = (*buffer[i])->getPosition();
                dstBuf[i].x = pos.x;
                dstBuf[i].y = pos.y;
                dstBuf[i].z = pos.z;
            }

            if (m_rcScan->hasIntensities())
            {
                float scaleI = 255 / m_rcScan->getIntensityMaxValue();
                float minI = m_rcScan->getIntensityMinValue(); // is it always 0 ?
                for (uint32_t i = 0; i < buffer.getSize(); ++i)
                {
                    dstBuf[i].i = (uint8_t)((*buffer[i])->getIntensity() * scaleI);
                }
            }

            if (m_rcScan->hasColors())
            {
                for (uint32_t i = 0; i < buffer.getSize(); ++i)
                {
                    auto& color = (*buffer[i])->getColor();
                    dstBuf[i].r = color.x;
                    dstBuf[i].g = color.y;
                    dstBuf[i].b = color.z;
                }
            }
        }
        return ...;
    }
    */
}
