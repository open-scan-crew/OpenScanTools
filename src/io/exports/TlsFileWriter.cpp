#include "io/exports/TlsFileWriter.h"

TlsFileWriter::TlsFileWriter(const std::filesystem::path& filepath)
    : IScanFileWriter(filepath)
{
    img_file_.open(filepath, tls::usage::write);
}

TlsFileWriter::~TlsFileWriter()
{ }

bool TlsFileWriter::getWriter(const std::filesystem::path& dirPath, const std::wstring& fileName, std::wstring& log, IScanFileWriter** writer)
{
    std::filesystem::path filepath = dirPath / fileName;
    filepath += ".tls";
    (*writer) = new TlsFileWriter(filepath);

    return ((TlsFileWriter*)(*writer))->img_file_.is_valid_file();
}

uint64_t TlsFileWriter::getScanPointCount() const
{
    return img_file_.getPointCount();
}

tls::ScanHeader TlsFileWriter::getLastScanHeader() const
{
    return img_file_.getPointCloudHeader(img_file_.getPointCloudCount() - 1);
}

FileType TlsFileWriter::getType() const
{
    return FileType::TLS;
}

bool TlsFileWriter::appendPointCloud(const tls::ScanHeader& info, const TransformationModule& transfo)
{
    glm::dquat q = transfo.getOrientation();
    glm::dvec3 t = transfo.getCenter();

    tls::ScanHeader info_and_transfo = info;
    info_and_transfo.transfo.translation[0] = t.x;
    info_and_transfo.transfo.translation[1] = t.y;
    info_and_transfo.transfo.translation[2] = t.z;
    info_and_transfo.transfo.quaternion[0] = q.x;
    info_and_transfo.transfo.quaternion[1] = q.y;
    info_and_transfo.transfo.quaternion[2] = q.z;
    info_and_transfo.transfo.quaternion[3] = q.w;

    return img_file_.appendPointCloud(info_and_transfo);
}

bool TlsFileWriter::addPoints(PointXYZIRGB const* srcBuf, uint64_t srcSize)
{
    return img_file_.addPoints(reinterpret_cast<tls::Point const*>(srcBuf), srcSize);
}

bool TlsFileWriter::mergePoints(PointXYZIRGB const* srcBuf, uint64_t srcSize, const TransformationModule& srcTransfo, tls::PointFormat srcFormat)
{
    glm::dquat q = srcTransfo.getOrientation();
    glm::dvec3 t = srcTransfo.getCenter();
    tls::Transformation tls_transfo = { { q.x, q.y, q.z, q.w }, { t.x, t.y, t.z } };

    return img_file_.mergePoints(reinterpret_cast<tls::Point const*>(srcBuf), srcSize, tls_transfo, srcFormat);
}

bool TlsFileWriter::finalizePointCloud()
{
    return img_file_.finalizePointCloud(post_translation_.x, post_translation_.y, post_translation_.z);
}
