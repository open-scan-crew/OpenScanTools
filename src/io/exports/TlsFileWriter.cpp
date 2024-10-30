#include "io/exports/TlsFileWriter.h"
#include "pointCloudEngine/OctreeCtor.h"
#include "io/exports/TlsWriter.h"
#include "utils/math/trigo.h"

TlsFileWriter::TlsFileWriter(const std::filesystem::path& filepath)
    : IScanFileWriter(filepath)
    , m_octree(nullptr)
{
    std::string msg;
    m_ostream.open(filepath, std::ios::out | std::ios::binary);
    if (m_ostream.fail()) {
        msg += "Error: cannot save at ";
        msg += filepath.string();
        msg += ": no such file or directory\n";
        throw std::exception(msg.c_str());
    }

    if (!tls::writer::writeFileHeader(m_ostream, 1)) {
        msg += "Tls Writer: An error occured while saving the point cloud.\n";
        throw std::exception(msg.c_str());
    }
}

TlsFileWriter::~TlsFileWriter()
{
    if (m_octree != nullptr)
        delete m_octree;

    m_ostream.close();
}

bool TlsFileWriter::getWriter(const std::filesystem::path& dirPath, const std::wstring& fileName, std::wstring& log, IScanFileWriter** writer)
{
    try
    {
        std::filesystem::path filepath = dirPath / fileName;
        filepath += ".tls";
        (*writer) = new TlsFileWriter(filepath);
    }
    catch (std::exception& e)
    {
        log += L"An error occured while creating a new TLS file: ";
        log += (wchar_t*)e.what();
        return false;
    }
    return true;
}

FileType TlsFileWriter::getType() const
{
    return FileType::TLS;
}

bool TlsFileWriter::appendPointCloud(const tls::ScanHeader& scanHeader)
{
    // Flush the octree and destroy it before creating a new one
    if (m_octree != nullptr)
        return false;

    // NOTE - The 'name' of the scan is not used, the tls (v0.4) is not a multiscan format.
    m_header = scanHeader;
    // Generate a UUID for the scan
    m_header.guid = xg::newGuid();
    m_header.version = tls::ScanVersion::SCAN_V_0_4;
    m_octree = new OctreeCtor(scanHeader.precision, scanHeader.format);
    m_scanPointCount = 0;
    return true;
}

bool TlsFileWriter::addPoints(PointXYZIRGB const* srcBuf, uint64_t srcSize)
{
    if (m_octree == nullptr)
        return false;

    for (uint64_t n = 0; n < srcSize; ++n)
    {
        m_octree->insertPoint(srcBuf[n]);
    }
    m_scanPointCount = m_octree->getPointCount();
    return true;
}

bool TlsFileWriter::mergePoints(PointXYZIRGB const* srcBuf, uint64_t srcSize, const glm::dmat4& src_transfo, tls::PointFormat srcFormat)
{
    assert(m_octree != nullptr);
    if (m_octree == nullptr)
        return false;

    glm::dmat4 cmp_transfo = tls::math::getTransformDMatrix(m_header.transfo.translation, m_header.transfo.quaternion);

    // compare destination and source transformation
    if (cmp_transfo != src_transfo)
    {
        glm::dmat4 dst_transfo = tls::math::getInverseTransformDMatrix(m_header.transfo.translation, m_header.transfo.quaternion);
        glm::dmat4 total_transfo = dst_transfo * src_transfo;

        // Select the correct conversion function
        typedef PointXYZIRGB(*convert_fn_t)(const PointXYZIRGB &, const glm::dmat4 &);
        convert_fn_t convert_fn = convert_keepIRGB;
        if (srcFormat == m_header.format)
        {
            convert_fn = convert_keepIRGB;
        }
        else if (srcFormat == tls::PointFormat::TL_POINT_XYZ_RGB)
        {
            convert_fn = convert_overwriteI;
        }
        else if (srcFormat == tls::PointFormat::TL_POINT_XYZ_I)
        {
            convert_fn = convert_overwriteRGB;
        }

        for (uint64_t n = 0; n < srcSize; ++n)
        {
            PointXYZIRGB point = convert_fn(srcBuf[n], total_transfo);
            m_octree->insertPoint(point);
        }
    }
    else
    {
        glm::dmat4 dst_transfo = tls::math::getInverseTransformDMatrix(m_header.transfo.translation, m_header.transfo.quaternion);
        glm::dmat4 result = dst_transfo * src_transfo;
        // For debugging
        assert(result[0][0] == 1.0);
        assert(result[1][1] == 1.0);
        assert(result[2][2] == 1.0);
        assert(result[3][3] == 1.0);

        for (uint64_t n = 0; n < srcSize; ++n)
        {
            m_octree->insertPoint(srcBuf[n]);
        }
    }

    m_scanPointCount = m_octree->getPointCount();
    return true;
}

bool TlsFileWriter::flushWrite()
{
    // No point in octree - Autodestruct the file
    if (m_octree->getPointCount() == 0)
    {
        m_ostream.close();
        std::filesystem::remove(m_filepath);
    }
    else
    {
        // Write the octree and its data in the file
        if (m_ostream.is_open() == false)
            return false;

        std::stringbuf buffer;
        // TODO(robin) - Do something with the log !
        std::ostream logStream(&buffer);

        // Encode the data contained in the octree in their final form
        if (m_octree != nullptr) {
            m_octree->encode(m_header.format, logStream);
        }

        if (!tls::writer::writeOctreeCtor(m_ostream, 0, m_octree, m_header)) {
            std::cerr << "pcc: An error occured while saving the point cloud." << std::endl;
            delete m_octree;
            m_octree = nullptr;
            return false;
        }

        m_ostream.flush();
        m_ostream.close();
    }

    m_scanPointCount = m_octree->getPointCount();
    m_totalPointCount += m_scanPointCount;
    delete m_octree;
    m_octree = nullptr;
    return true;
}

void TlsFileWriter::translateOrigin(double dx, double dy, double dz)
{
    m_ostream.open(m_filepath, std::ios::out | std::ios::in | std::ios::binary);
    if (m_ostream.fail())
        return;

    double newOrigin[] = { m_header.transfo.translation[0] + dx, m_header.transfo.translation[1] + dy, m_header.transfo.translation[2] + dz };
    tls::writer::overwriteTransformation(m_ostream, newOrigin, m_header.transfo.quaternion);

    m_ostream.flush();
    m_ostream.close();
}
