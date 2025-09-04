#ifndef PTS_WRITER_H
#define PTS_WRITER_H

#include "io/exports/IScanFileWriter.h"
#include "tls_def.h"

#include <filesystem>
#include <fstream>

struct PointXYZIRGB;

class PTSFileWriter : public IScanFileWriter
{
public:
    ~PTSFileWriter();

    static bool getWriter(const std::filesystem::path& dirPath, const std::wstring& fileName, std::wstring& log, IScanFileWriter** writer);

    uint64_t getTotalPointCount() const override;
    uint64_t getScanPointCount() const override;
    tls::ScanHeader getLastScanHeader() const override;

    FileType getType() const override;
    bool appendPointCloud(const tls::ScanHeader& header, const TransformationModule& transfo) override;
    bool addPoints(PointXYZIRGB const* srcBuf, uint64_t srcSize) override;
    bool mergePoints(PointXYZIRGB const* srcBuf, uint64_t srcSize, const TransformationModule& src_transfo, tls::PointFormat srcFormat) override;
    bool finalizePointCloud() override;

private:
    PTSFileWriter(const std::filesystem::path& filepath);
    void insertPoint(const PointXYZIRGB& point);

private:

    // current write
    tls::ScanHeader m_scanHeader;
    TransformationModule scan_transfo;
    std::ofstream m_streamWriteScan;

    uint64_t total_point_count_;
    bool has_intensity;
    bool has_color;
};

#endif