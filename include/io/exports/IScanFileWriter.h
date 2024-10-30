#ifndef I_SCAN_FILE_WRITER_H
#define I_SCAN_FILE_WRITER_H

#include "io/FileUtils.h"
#include "models/pointCloud/TLS.h"

#include <string>
#include <filesystem>


struct PointXYZIRGB;

class IScanFileWriter// : public IPointCloudWriter
{
public:
    IScanFileWriter(const std::filesystem::path& filepath);
    virtual ~IScanFileWriter() {};
    std::filesystem::path getFilePath() const;
    uint32_t getScanCount() const;
    uint64_t getTotalPoints() const;
    uint64_t getScanPointCount() const;

    virtual FileType getType() const = 0;

    virtual bool appendPointCloud(const tls::ScanHeader& header) = 0;
    virtual bool addPoints(PointXYZIRGB const* srcBuf, uint64_t srcSize) = 0;
    virtual bool mergePoints(PointXYZIRGB const* srcBuf, uint64_t srcSize, const glm::dmat4& src_transfo, tls::PointFormat srcFormat) = 0;

    // TODO(robin) - Rename function
    virtual bool flushWrite() = 0;

protected:
    std::filesystem::path m_filepath;
    uint32_t m_currentScanCount;
    uint64_t m_totalPointCount;
    uint64_t m_scanPointCount;
};

bool getScanFileWriter(const std::filesystem::path& dirPath, const std::wstring& fileName, FileType type, std::wstring& log, IScanFileWriter**);

#endif