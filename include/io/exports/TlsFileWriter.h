#ifndef TLS_WRITER_H
#define TLS_WRITER_H

#include "io/exports/IScanFileWriter.h"
#include "tls_core.h"

#include <filesystem>

struct PointXYZIRGB;

class TlsFileWriter : public IScanFileWriter
{
public:
    ~TlsFileWriter();

    static bool getWriter(const std::filesystem::path& dirPath, const std::wstring& fileName, std::wstring& log, IScanFileWriter** writer);

    uint64_t getScanPointCount() const override;
    tls::ScanHeader getLastScanHeader() const override;

    FileType getType() const override;
    bool appendPointCloud(const tls::ScanHeader& info, const TransformationModule& transfo) override;
    bool addPoints(PointXYZIRGB const* srcBuf, uint64_t srcSize) override;
    bool mergePoints(PointXYZIRGB const* srcBuf, uint64_t srcSize, const TransformationModule& srcTransfo, tls::PointFormat srcFormat) override;

    bool finalizePointCloud() override;

private:
    TlsFileWriter(const std::filesystem::path& filepath);

private:
    tls::ImageFile img_file_;
};

#endif