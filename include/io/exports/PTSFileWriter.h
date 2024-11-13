#ifndef PTS_WRITER_H
#define PTS_WRITER_H

#include "io/exports/IScanFileWriter.h"
#include "models/pointCloud/TLS.h"

#include <filesystem>
#include <fstream>

struct PointXYZIRGB;

class PTSFileWriter : public IScanFileWriter
{
public:
    ~PTSFileWriter();

    static bool getWriter(const std::filesystem::path& dirPath, const std::wstring& fileName, std::wstring& log, IScanFileWriter** writer);

    FileType getType() const override;
    bool appendPointCloud(const tls::ScanHeader& header, const TransformationModule& transfo) override;
    bool addPoints(PointXYZIRGB const* srcBuf, uint64_t srcSize) override;
    bool mergePoints(PointXYZIRGB const* srcBuf, uint64_t srcSize, const TransformationModule& src_transfo, tls::PointFormat srcFormat) override;
    void setPostTranslation(const glm::dvec3& translation) override;
    bool finalizePointCloud() override;

private:
    PTSFileWriter(const std::filesystem::path& filepath);
    void insertPoint(const PointXYZIRGB& point);

private:

    // current write
    tls::ScanHeader m_scanHeader;
    TransformationModule scan_transfo;
    std::ofstream m_streamWriteScan;

    bool has_intensity;
    bool has_color;
};

#endif