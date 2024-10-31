#ifndef TLS_WRITER_H
#define TLS_WRITER_H

#include "io/exports/IScanFileWriter.h"
#include "models/pointCloud/TLS.h"

#include <filesystem>
#include <fstream>

struct PointXYZIRGB;
class OctreeCtor;

class TlsFileWriter : public IScanFileWriter
{
public:
    ~TlsFileWriter();

    static bool getWriter(const std::filesystem::path& dirPath, const std::wstring& fileName, std::wstring& log, IScanFileWriter** writer);

    FileType getType() const override;
    bool appendPointCloud(const tls::ScanHeader& info, const TransformationModule& transfo) override;
    bool addPoints(PointXYZIRGB const* srcBuf, uint64_t srcSize) override;
    bool mergePoints(PointXYZIRGB const* srcBuf, uint64_t srcSize, const TransformationModule& srcTransfo, tls::PointFormat srcFormat) override;
    void addTranslation(const glm::dvec3& translation) override;

    bool flushWrite() override;

    // Temporary for large coordinates
    void translateOrigin(double dx, double dy, double dz);

private:
    TlsFileWriter(const std::filesystem::path& filepath);

private:
    std::ofstream m_ostream;
    // current write
    tls::ScanHeader m_header;
    TransformationModule scan_transfo;
    OctreeCtor* m_octree;
};

#endif