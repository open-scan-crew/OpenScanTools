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

using namespace Autodesk::RealityComputing::Foundation;
using namespace Autodesk::RealityComputing::Data;

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
    initHeaders();
}

RcsFileReader::RcsFileReader(const std::filesystem::path& filepath, bool visiblePointsOnly, bool userEdits)
    : IScanFileReader(filepath)
    , m_pointCount(0)
    , m_visiblePointsOnly(visiblePointsOnly)
    , m_userEdits(userEdits)
    , m_currentPointIndex(0)
{
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
    m_scanHeader.version = tls::ScanVersion::SCAN_V_0_4;
    m_scanHeader.acquisitionDate = 0; // no date
    m_scanHeader.name = m_rcScan->getName();
    m_scanHeader.sensorModel = m_rcScan->getScanProvider();
    m_scanHeader.sensorSerialNumber = L"Not provided";

    RCTransform rcTransfo;
    bool result = m_rcScan->getFullTransform(rcTransfo);
    if (result)
    {
        auto rcQuat = rcTransfo.getRotation().toQuaternion();
        transfo_.setRotation(glm::dquat(rcQuat.values.w, rcQuat.values.x, rcQuat.values.y, rcQuat.values.z));
        const auto& tr = rcTransfo.getTranslation();
        transfo_.setPosition(glm::dvec3(tr.x, tr.y, tr.z));
    }

    auto rcbbox = m_rcScan->getBoundingBox();
    const auto& bboxMin = rcbbox.getMin();
    const auto& bboxMax = rcbbox.getMax();
    m_scanHeader.bbox = { (float)bboxMin.x, (float)bboxMax.x,
        (float)bboxMin.y, (float)bboxMax.y,
        (float)bboxMin.z, (float)bboxMax.z };
    m_scanHeader.pointCount = m_rcScan->getNumberOfPoints();
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

const tls::FileHeader& RcsFileReader::getTlsHeader() const
{
    return m_fileHeader;
}

tls::ScanHeader RcsFileReader::getTlsScanHeader(uint32_t scanNumber) const
{
    return m_scanHeader;
}

bool RcsFileReader::startReadingScan(uint32_t _scanNumber)
{
    m_currentPointIndex = 0;
    RCTransform rcTransform;
    bool result = m_rcScan->setFullTransform(rcTransform);
    return result;
}

bool RcsFileReader::readPoints(PointXYZIRGB* dstBuf, uint64_t bufSize, uint64_t& readCount)
{
    Autodesk::RealityComputing::Data::RCPointIteratorSettings settings;
    settings.setDensity(-1.0); // highest density
    settings.setIsVisiblePointsOnly(true);

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

            glm::dvec4 glPos = inv_transfo * glm::dvec4(pos.x, pos.y, pos.z, 1.0);

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
