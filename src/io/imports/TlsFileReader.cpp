#include "io/imports/TlsFileReader.h"

bool TlsFileReader::getReader(const std::filesystem::path& filepath, std::wstring& log, TlsFileReader** reader)
{
    *reader = new TlsFileReader(filepath);

    return (*reader)->img_file_.is_valid_file();
}

TlsFileReader::TlsFileReader(const std::filesystem::path& filepath)
    : IScanFileReader(filepath)
{
    img_file_.open(filepath, tls::usage::read);
}

TlsFileReader::~TlsFileReader()
{}

FileType TlsFileReader::getType() const
{
    return FileType::TLS;
}

uint32_t TlsFileReader::getScanCount() const
{
    return img_file_.getScanCount();
}

uint64_t TlsFileReader::getTotalPoints() const
{
    return img_file_.getTotalPoints();
}

bool TlsFileReader::startReadingScan(uint32_t scanNumber)
{
    point_cloud_ = img_file_.getImagePointCloud(scanNumber);

    return point_cloud_.is_valid();
}

bool TlsFileReader::readPoints(PointXYZIRGB* dstBuf, uint64_t bufSize, uint64_t& readCount)
{
    return point_cloud_.readNextPoints(reinterpret_cast<tls::Point*>(dstBuf), bufSize, readCount);
}

tls::FileHeader TlsFileReader::getTlsHeader() const
{
    return img_file_.getFileHeader();
}

tls::ScanHeader TlsFileReader::getTlsScanHeader(uint32_t scanNumber) const
{
    return img_file_.getPointCloudHeader(scanNumber);
}
