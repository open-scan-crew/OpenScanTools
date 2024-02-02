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

bool TlsFileWriter::mergePoints(PointXYZIRGB const* srcBuf, uint64_t srcSize, const glm::dmat4& srcTransfo, tls::PointFormat srcFormat)
{
#ifdef _DEBUG
    if (m_octree == nullptr)
        assert(0);
#endif

    glm::dmat4 finalMatrix = tls::math::getInverseTransformDMatrix(m_header.transfo.translation, m_header.transfo.quaternion);
    {
        //glm::mat4 srcMatrix = tls::math::getTransformMatrix(srcTransfo.translation, srcTransfo.quaternion);
        finalMatrix *= srcTransfo;
    }

    if (srcFormat == m_header.format)
    {
        for (uint64_t n = 0; n < srcSize; ++n)
        {
            PointXYZIRGB point = convert_keepIRGB(srcBuf[n], finalMatrix);
            m_octree->insertPoint(point);
        }
    }
    else if (srcFormat == tls::PointFormat::TL_POINT_XYZ_RGB)
    {
        for (uint64_t n = 0; n < srcSize; ++n)
        {
            PointXYZIRGB point = convert_overwriteI(srcBuf[n], finalMatrix);
            m_octree->insertPoint(point);
        }
    }
    else if (srcFormat == tls::PointFormat::TL_POINT_XYZ_I)
    {
        for (uint64_t n = 0; n < srcSize; ++n)
        {
            PointXYZIRGB point = convert_overwriteRGB(srcBuf[n], finalMatrix);
            m_octree->insertPoint(point);
        }
    }
    else
    {
        // ERROR
        return false;
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
