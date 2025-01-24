#ifndef RCS_FILE_READER_H
#define RCS_FILE_READER_H

#include "io/imports/IScanFileReader.h"

#include "models/graph/TransformationModule.h"

#include <data/RCScan.h>


using Autodesk::RealityComputing::Foundation::RCSharedPtr;
using Autodesk::RealityComputing::Data::RCScan;

class RcsFileReader : public IScanFileReader
{

public:
    static bool getReader(const std::filesystem::path& filepath, std::wstring log, IScanFileReader** reader);
    RcsFileReader(RCSharedPtr<RCScan> rcScan);
    ~RcsFileReader();
    FileType getType() const override;
    uint32_t getScanCount() const override;
    uint64_t getTotalPoints() const override;

    bool startReadingScan(uint32_t scanNumber) override;
    bool readPoints(PointXYZIRGB* dstBuf, uint64_t bufSize, uint64_t& readCount) override;

    tls::FileHeader getTlsHeader() const override;
    tls::ScanHeader getTlsScanHeader(uint32_t scanNumber) const override;

private:
    RcsFileReader(const std::filesystem::path& filepath, bool visiblePointsOnly, bool userEdits);

    void initHeaders();

private:
    RCSharedPtr<RCScan> m_rcScan;
    uint64_t m_pointCount;
    bool m_visiblePointsOnly;
    bool m_userEdits;

    uint64_t m_currentPointIndex;

    tls::FileHeader m_fileHeader;
    tls::ScanHeader m_scanHeader;
    TransformationModule transfo_;
};

#endif