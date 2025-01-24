#ifndef TLS_FILE_WRITER_H
#define TLS_FILE_WRITER_H

#include "io/exports/IScanFileWriter.h"
#include "tls_def.h"

#include <filesystem>
#include <fstream>

struct PointXYZIRGB;

class TlsFileWriter : public IScanFileWriter
{
public:
    ~TlsFileWriter();

    static bool getWriter(const std::filesystem::path& dirPath, const std::wstring& fileName, std::wstring& log, IScanFileWriter** writer);

    FileType getType() const override;
    bool appendPointCloud(const tls::ScanHeader& info, const TransformationModule& transfo) override;
    bool addPoints(PointXYZIRGB const* srcBuf, uint64_t srcSize) override;
    bool mergePoints(PointXYZIRGB const* srcBuf, uint64_t srcSize, const TransformationModule& srcTransfo, tls::PointFormat srcFormat) override;

    bool finalizePointCloud() override;

private:
    TlsFileWriter(const std::filesystem::path& filepath);

private:
    std::ofstream m_ostream;
    std::wstring pc_name_;           // mandatory
    tls::PrecisionType precision_;   // mandatory
    tls::PointFormat format_;        // mandatory
    uint64_t acquisition_date_;      // optional
    TransformationModule scan_transfo_;
    OctreeCtor* m_octree;
};

#endif