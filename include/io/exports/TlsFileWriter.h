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
    //void setPostTranslation(const glm::dvec3& translation) override;

    bool finalizePointCloud() override;

private:
    TlsFileWriter(const std::filesystem::path& filepath);

private:
    std::ofstream m_ostream;
    // current write
    //tls::ScanHeader scan_header_;
    std::wstring pc_name_;           // mandatory
    // std::wstring sensor_;         // optional
    tls::PrecisionType precision_;   // mandatory
    tls::PointFormat format_;        // mandatory
    uint64_t acquisition_date_;      // optional
    //enum class DataOrigin { sensor, aggregated }; // mandatory
    TransformationModule scan_transfo_;
    OctreeCtor* m_octree;
};

#endif