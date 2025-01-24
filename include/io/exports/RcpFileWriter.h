#ifndef RCP_FILE_WRITER_H
#define RCP_FILE_WRITER_H

#include "io/exports/IScanFileWriter.h"
#include "tls_def.h"

#include <data/RCProjectImportSession.h>

#include <filesystem>
#include <vector>
#include <list>

using Autodesk::RealityComputing::Foundation::RCSharedPtr;
using Autodesk::RealityComputing::Data::RCProjectImportSession;

struct PointXYZIRGB;

class RcpFileWriter : public IScanFileWriter
{
public:
    ~RcpFileWriter();

    static bool getWriter(const std::filesystem::path& filePath, const std::wstring& projectName, std::wstring& log, IScanFileWriter** writer);

    FileType getType() const override;
    bool appendPointCloud(const tls::ScanHeader& header, const TransformationModule& transfo) override;
    bool addPoints(PointXYZIRGB const* src_buf, uint64_t src_size) override;
    bool mergePoints(PointXYZIRGB const* src_buf, uint64_t src_size, const TransformationModule& src_transfo, tls::PointFormat src_format) override;
    bool finalizePointCloud() override;

    void setExportDensity(double density);

private:
    RcpFileWriter(const std::filesystem::path& file_path, RCSharedPtr<RCProjectImportSession> projectImporter) noexcept;

private:
    RCSharedPtr<RCProjectImportSession> m_projectImportSession;
    bool m_hasColor;
    bool m_hasIntensity;
    double m_exportDensity;

    // current write
    tls::ScanHeader m_scanHeader;
    TransformationModule scan_transfo;
    std::list<Autodesk::RealityComputing::Data::RCPointBuffer> m_pointBuffers;
};

#endif
