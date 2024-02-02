#ifndef RCP_FILE_READER_H
#define RCP_FILE_READER_H

#include "io/imports/IScanFileReader.h"
#include "io/imports/RcsFileReader.h"

#include "data/RCProject.h"

using Autodesk::RealityComputing::Foundation::RCSharedPtr;
using Autodesk::RealityComputing::Data::RCProject;

class RcpFileReader : public IScanFileReader
{

public:
    static bool getReader(const std::filesystem::path& filepath, std::wstring& log, IScanFileReader** reader);
    ~RcpFileReader();
    FileType getType() const override;
    uint32_t getScanCount() const override;
    uint64_t getTotalPoints() const override;

    bool startReadingScan(uint32_t scanNumber) override;
    bool readPoints(PointXYZIRGB* dstBuf, uint64_t bufSize, uint64_t& readCount) override;

    const tls::FileHeader& getTlsHeader() const override;
    tls::ScanHeader getTlsScanHeader(uint32_t scanNumber) const override;

private:
    RcpFileReader(const std::filesystem::path& filepath, bool visiblePointsOnly, bool userEdits);

private:
    RCSharedPtr<RCProject> m_project;
    RcsFileReader* m_currentScanReader;
    uint64_t m_totalPointCount;
    bool m_visiblePointsOnly;
    bool m_userEdits;

    tls::FileHeader m_header;
    std::vector<tls::ScanHeader> m_scanHeaders;
};

#endif