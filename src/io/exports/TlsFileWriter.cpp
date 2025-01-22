#include "io/exports/TlsFileWriter.h"
#include "pointCloudEngine/OctreeCtor.h"
#include "io/exports/TlsWriter.h"

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

bool TlsFileWriter::appendPointCloud(const tls::ScanHeader& scanHeader, const TransformationModule& transfo)
{
    // Finalize the octree and destroy it before creating a new one
    if (m_octree != nullptr)
        return false;

    // NOTE - The 'name' of the scan is not used, the tls (v0.4) is not a multiscan format.
    pc_name_ = scanHeader.name;
    precision_ = scanHeader.precision;
    format_ = scanHeader.format;

    m_octree = new OctreeCtor(scanHeader.precision, scanHeader.format);
    scan_transfo_ = transfo;
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

bool TlsFileWriter::mergePoints(PointXYZIRGB const* srcBuf, uint64_t srcSize, const TransformationModule& src_transfo, tls::PointFormat srcFormat)
{
    assert(m_octree != nullptr);
    if (m_octree == nullptr)
        return false;

    // compare destination and source transformation
    if (scan_transfo_ == src_transfo)
    {
        for (uint64_t n = 0; n < srcSize; ++n)
        {
            m_octree->insertPoint(srcBuf[n]);
        }
    }
    else
    {
        glm::dmat4 total_transfo = scan_transfo_.getInverseTransformation() * src_transfo.getTransformation();
        // Or use the multiply in the (pos, quat, scale) space
        TransformationModule transfo_bis(src_transfo);
        transfo_bis.compose_inverse_left(scan_transfo_);
        glm::dmat4 total_transfo_bis = transfo_bis.getTransformation();

        // Select the correct conversion function
        typedef PointXYZIRGB(*convert_fn_t)(const PointXYZIRGB &, const glm::dmat4 &);
        convert_fn_t convert_fn = convert_transfo;
        if (srcFormat == format_)
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
            m_octree->insertPoint(point);
        }
    }

    m_scanPointCount = m_octree->getPointCount();
    return true;
}

bool TlsFileWriter::finalizePointCloud()
{
    if (m_octree == nullptr)
        return false;
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
            m_octree->encode(format_, logStream);
        }

        tls::ScanHeader header;
        header.name = pc_name_;
        // Generate a UUID for the scan
        header.guid = xg::newGuid();
        header.version = tls::ScanVersion::SCAN_V_0_4;
        header.precision = precision_;
        header.format = format_;
        glm::dvec3 t = scan_transfo_.getCenter() + post_translation_;
        header.transfo.translation[0] = t.x;
        header.transfo.translation[1] = t.y;
        header.transfo.translation[2] = t.z;
        glm::dquat q = scan_transfo_.getOrientation();
        header.transfo.quaternion[0] = q.x;
        header.transfo.quaternion[1] = q.y;
        header.transfo.quaternion[2] = q.z;
        header.transfo.quaternion[3] = q.w;
        header.acquisitionDate = std::time(nullptr);

        if (!tls::writer::writeOctreeCtor(m_ostream, 0, m_octree, header)) {
            std::cerr << "pcc: An error occured while saving the point cloud." << std::endl;
            delete m_octree;
            m_octree = nullptr;
            return false;
        }

        m_ostream.flush();
        m_ostream.close();

        m_scanPointCount = m_octree->getPointCount();
        header.pointCount = m_octree->getPointCount();
        header.bbox = m_octree->getBoundingBox();
        m_totalPointCount += m_scanPointCount;
        out_scan_headers.push_back(header);
    }

    delete m_octree;
    m_octree = nullptr;
    return true;
}
