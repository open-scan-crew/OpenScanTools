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

FileType TlsFileWriter::getType() const
{
    return FileType::TLS;
}

bool TlsFileWriter::appendPointCloud(const tls::ScanHeader& info, const TransformationModule& transfo)
{
    glm::dquat q = transfo.getOrientation();
    glm::dvec3 t = transfo.getCenter();
    tls::Transformation tls_transfo = { { q.x, q.y, q.z, q.w }, { t.x, t.y, t.z } };

    return img_file_.appendPointCloud(info, tls_transfo);
}

bool TlsFileWriter::addPoints(PointXYZIRGB const* srcBuf, uint64_t srcSize)
{
    return img_file_.addPoints(srcBuf, srcSize);
}

bool TlsFileWriter::mergePoints(PointXYZIRGB const* srcBuf, uint64_t srcSize, const TransformationModule& srcTransfo, tls::PointFormat srcFormat)
{
    glm::dquat q = srcTransfo.getOrientation();
    glm::dvec3 t = srcTransfo.getCenter();
    tls::Transformation tls_transfo = { { q.x, q.y, q.z, q.w }, { t.x, t.y, t.z } };

    return img_file_.mergePoints(srcBuf, srcSize, tls_transfo, srcFormat);
}

bool TlsFileWriter::finalizePointCloud()
{
    return img_file_.finalizePointCloud(0, post_translation_.x, post_translation_.y, post_translation_.z);
}
