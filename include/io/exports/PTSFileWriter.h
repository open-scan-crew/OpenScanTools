#ifndef PTS_WRITER_H
#define PTS_WRITER_H

#include "io/exports/IScanFileWriter.h"
#include "models/pointCloud/TLS.h"
#include "io/StagingBuffers.h"

#include <filesystem>
#include <fstream>

struct PointXYZIRGB;

class PTSFileWriter : public IScanFileWriter
{
public:
    ~PTSFileWriter();

    static bool getWriter(const std::filesystem::path& dirPath, const std::wstring& fileName, std::wstring& log, IScanFileWriter** writer);

    FileType getType() const override;
    bool appendPointCloud(const tls::ScanHeader& info) override;
    bool addPoints(PointXYZIRGB const* srcBuf, uint64_t srcSize) override;
    bool mergePoints(PointXYZIRGB const* srcBuf, uint64_t srcSize, const glm::dmat4& srcTransfo, tls::PointFormat srcFormat) override;
    bool flushWrite() override;

private:
    PTSFileWriter(const std::filesystem::path& filepath);
    void insertPoint(const PointXYZIRGB& point);

private:

    // current write
    tls::ScanHeader m_scanHeader;
    std::ofstream m_streamWriteScan;

    bool m_hasIntensity;
    bool m_hasColor;
};

#endif