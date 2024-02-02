#ifndef RCP_FILE_WRITER_H
#define RCP_FILE_WRITER_H

#include "io/exports/IScanFileWriter.h"
#include "models/pointCloud/TLS.h"

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
    bool appendPointCloud(const tls::ScanHeader& info) override;
    bool addPoints(PointXYZIRGB const* srcBuf, uint64_t srcSize) override;
    bool mergePoints(PointXYZIRGB const* srcBuf, uint64_t srcSize, const glm::dmat4& srcTransfo, tls::PointFormat srcFormat) override;
    bool flushWrite() override;

    void setExportDensity(double density);

private:
    RcpFileWriter(RCSharedPtr<RCProjectImportSession> projectImporter) noexcept;

private:
    RCSharedPtr<RCProjectImportSession> m_projectImportSession;
    bool m_hasColor;
    bool m_hasIntensity;
    double m_exportDensity;

    // current write
    tls::ScanHeader m_scanHeader;
    std::list<Autodesk::RealityComputing::Data::RCPointBuffer> m_pointBuffers;
};

#endif
