#ifndef E57_WRITER_H
#define E57_WRITER_H

#include "io/exports/IScanFileWriter.h"
#include "openE57.h"
#include "models/pointCloud/TLS.h"
#include "io/StagingBuffers.h"

#include <filesystem>
#include <vector>

struct PointXYZIRGB;

struct CVWriterWrapper
{
    e57::CompressedVectorWriter cvw;
    tls::PointFormat format;
};

class E57FileWriter : public IScanFileWriter
{
public:
    ~E57FileWriter();

    static bool getWriter(const std::filesystem::path& dirPath, const std::wstring& fileName, std::wstring& log, IScanFileWriter** writer);

    FileType getType() const override;
    bool appendPointCloud(const tls::ScanHeader& info) override;
    bool addPoints(PointXYZIRGB const* srcBuf, uint64_t srcSize) override;
    bool mergePoints(PointXYZIRGB const* srcBuf, uint64_t srcSize, const glm::dmat4& srcTransfo, tls::PointFormat srcFormat) override;
    void updateBoundingBox(const PointXYZIRGB& point);
    bool flushWrite() override;

private:
    E57FileWriter(const std::filesystem::path& filepath, e57::ImageFile imf);
    bool setHeader();

private:
    e57::ImageFile m_imf;

    // current write
    tls::ScanHeader m_scanHeader;
    StagingBuffers m_stagingBufs;
    CVWriterWrapper* m_storedWriter;
    tls::BoundingBox m_bbox;
};

#endif