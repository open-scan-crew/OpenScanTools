#ifndef TLS_FILE_READER_H
#define TLS_FILE_READER_H

#include "io/imports/IScanFileReader.h"

#include "tls_core.h"

class TlsFileReader : public IScanFileReader
{
public:
    static bool getReader(const std::filesystem::path& filepath, std::wstring& log, TlsFileReader** reader);

    ~TlsFileReader();
    FileType getType() const override;
    uint32_t getScanCount() const override;
    uint64_t getTotalPoints() const override;

    bool startReadingScan(uint32_t scanNumber) override;
    bool readPoints(PointXYZIRGB* dstBuf, uint64_t bufSize, uint64_t& readCount) override;

    tls::FileHeader getTlsHeader() const override;
    tls::ScanHeader getTlsScanHeader(uint32_t scanNumber) const override;

private:
    TlsFileReader(const std::filesystem::path& filepath);

private:
    tls::ImageFile img_file_;
    tls::ImagePointCloud point_cloud_;
};

#endif