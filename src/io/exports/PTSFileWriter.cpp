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
    if (has_color)
    {
        pointStr += std::to_string(point.r) + separator;
        pointStr += std::to_string(point.g) + separator;
        pointStr += std::to_string(point.b) + separator;
    }
    if (has_intensity)
    {
        pointStr += std::to_string(point.i) + separator;
    }

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

bool PTSFileWriter::appendPointCloud(const tls::ScanHeader& info, const TransformationModule& transfo)
{
    m_scanHeader = info;
    scan_transfo = transfo;
    m_scanPointCount = 0;

    // FIXME - Where is the header ??
    // X Y Z (optional) r g b (optional) i

    has_intensity = (info.format == tls::PointFormat::TL_POINT_XYZ_I || info.format == tls::PointFormat::TL_POINT_XYZ_I_RGB) ? true : false;
    has_color = (info.format == tls::PointFormat::TL_POINT_XYZ_RGB || info.format == tls::PointFormat::TL_POINT_XYZ_I_RGB) ? true : false;
    return true;
}

bool PTSFileWriter::addPoints(PointXYZIRGB const* srcBuf, uint64_t srcSize)
{
    // In a PTS, all points are in global system coordinates
    return mergePoints(srcBuf, srcSize, scan_transfo, m_scanHeader.format);
}

bool PTSFileWriter::mergePoints(PointXYZIRGB const* srcBuf, uint64_t srcSize, const TransformationModule& src_transfo, tls::PointFormat srcFormat)
{
    glm::dmat4 post_translation_mat = glm::dmat4(1.0);
    post_translation_mat[3] = glm::dvec4(post_translation_, 1.0);
    glm::dmat4 total_transfo = post_translation_mat * src_transfo.getTransformation();

    // Select the correct conversion function
    typedef PointXYZIRGB(*convert_fn_t)(const PointXYZIRGB&, const glm::dmat4&);
    convert_fn_t convert_fn = convert_transfo;
    if (srcFormat == m_scanHeader.format)
    {
        convert_fn = convert_transfo;
    }
    else if (srcFormat == tls::PointFormat::TL_POINT_XYZ_RGB)
    {
        convert_fn = convert_RGB_to_I_transfo;
    }
    else if (srcFormat == tls::PointFormat::TL_POINT_XYZ_I)
    {
        convert_fn = convert_I_to_RGB_transfo;
    }

    for (uint64_t n = 0; n < srcSize; ++n)
    {
        PointXYZIRGB point = convert_fn(srcBuf[n], total_transfo);
        insertPoint(point);
    }

    m_scanPointCount += srcSize;
    return true;
}

bool PTSFileWriter::finalizePointCloud()
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