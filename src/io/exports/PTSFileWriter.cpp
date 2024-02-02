#include "io/exports/PTSFileWriter.h"
#include "models/pointCloud/PointXYZIRGB.h"
#include "utils/time.h"
#include "utils/math/trigo.h"
#include "utils/Utils.h"

PTSFileWriter::PTSFileWriter(const std::filesystem::path& filepath)
    : IScanFileWriter(filepath)
    , m_streamWriteScan(filepath)
{
}

void PTSFileWriter::insertPoint(const PointXYZIRGB& point)
{
    uint32_t decimalsNumber = 3;
    char separator = ' ';
    std::string pointStr;
    pointStr += Utils::roundFloat(point.x, decimalsNumber) + separator;
    pointStr += Utils::roundFloat(point.y, decimalsNumber) + separator;
    pointStr += Utils::roundFloat(point.z, decimalsNumber) + separator;
    pointStr += std::to_string(point.r) + separator;
    pointStr += std::to_string(point.g) + separator;
    pointStr += std::to_string(point.b) + separator;
    pointStr += std::to_string(point.i) + separator;

    pointStr += '\n';

    m_streamWriteScan << pointStr;
}

PTSFileWriter::~PTSFileWriter()
{
    try {
        if (m_totalPointCount == 0)
            std::filesystem::remove(m_filepath);
    }
    catch (std::exception& ex) {
        std::cerr << "Got an std::excetion, what=" << ex.what() << std::endl;
    }
    catch (...) {
        std::cerr << "Got an unknown exception" << std::endl;
    }
}

bool PTSFileWriter::getWriter(const std::filesystem::path& dirPath, const std::wstring& fileName, std::wstring& log, IScanFileWriter** writer)
{
    try {
        // Read the file from disk
        std::filesystem::path completePath = dirPath / fileName;
        completePath += ".pts";

        // If we are here, we have created a valid e57 file ready for writing
        *writer = new PTSFileWriter(completePath);
    }
    catch (std::exception& ex) {
        std::cerr << "Got an std::excetion, what=" << ex.what() << std::endl;
        return false;
    }
    catch (...) {
        std::cerr << "Got an unknown exception" << std::endl;
        return false;
    }
    return true;
}

FileType PTSFileWriter::getType() const
{
    return FileType::PTS;
}

bool PTSFileWriter::appendPointCloud(const tls::ScanHeader& info)
{
    m_scanHeader = info;
    m_scanPointCount = 0;

    m_hasIntensity = (info.format == tls::PointFormat::TL_POINT_XYZ_I || info.format == tls::PointFormat::TL_POINT_XYZ_I_RGB) ? true : false;
    m_hasColor = (info.format == tls::PointFormat::TL_POINT_XYZ_RGB || info.format == tls::PointFormat::TL_POINT_XYZ_I_RGB) ? true : false;
    return true;
}

bool PTSFileWriter::addPoints(PointXYZIRGB const* srcBuf, uint64_t srcSize)
{
    glm::mat4 finalMatrix = tls::math::getTransformMatrix(m_scanHeader.transfo.translation, m_scanHeader.transfo.quaternion);

    try
    {
        for (uint64_t n = 0; n < srcSize; ++n)
        {
            PointXYZIRGB point = convert_keepIRGB(srcBuf[n], finalMatrix);
            if (!m_hasColor)
                point = convert_overwriteRGB(point);
            if(!m_hasIntensity)
                point = convert_overwriteI(point);
            insertPoint(point);
        }
        m_scanPointCount += srcSize;
    }
    catch (std::exception& ex) {
        std::cerr << "Got an std::excetion, what=" << ex.what() << std::endl;
        return false;
    }
    catch (...) {
        std::cerr << "Got an unknown exception" << std::endl;
        return false;
    }
    return true;
}

// NOTE - equivalent of IPointCloudWriter::addPoints_localSrc()
bool PTSFileWriter::mergePoints(PointXYZIRGB const* srcBuf, uint64_t srcSize, const glm::dmat4& srcTransfo, tls::PointFormat srcFormat)
{

    glm::dmat4 finalMatrix = srcTransfo;

    if (srcFormat == m_scanHeader.format)
    {
        for (uint64_t n = 0; n < srcSize; ++n)
        {
            PointXYZIRGB point = convert_keepIRGB(srcBuf[n], finalMatrix);
            if (!m_hasColor)
                point = convert_overwriteRGB(point);
            if (!m_hasIntensity)
                point = convert_overwriteI(point);
            insertPoint(point);
        }
    }
    else if (srcFormat == tls::PointFormat::TL_POINT_XYZ_RGB)
    {
        for (uint64_t n = 0; n < srcSize; ++n)
        {
            PointXYZIRGB point = convert_overwriteI(srcBuf[n], finalMatrix);
            insertPoint(point);
        }
    }
    else if (srcFormat == tls::PointFormat::TL_POINT_XYZ_I)
    {
        for (uint64_t n = 0; n < srcSize; ++n)
        {
            PointXYZIRGB point = convert_overwriteRGB(srcBuf[n], finalMatrix);
            insertPoint(point);
        }
    }
    else
    {
        assert(false);
        return false;
    }
    m_scanPointCount += srcSize;
    return true;
}

bool PTSFileWriter::flushWrite()
{
    try
    {
        m_streamWriteScan.flush();
        m_streamWriteScan.close();
        m_totalPointCount += m_scanPointCount;
        return true;
    }
    catch (std::exception& ex) {
        std::cerr << "Got an std::excetion, what=" << ex.what() << std::endl;
        return false;
    }
    catch (...) {
        std::cerr << "Got an unknown exception" << std::endl;
        return false;
    }
}