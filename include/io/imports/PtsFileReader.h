#ifndef PTS_FILE_READER_H_
#define PTS_FILE_READER_H_

#include "io/imports/IScanFileReader.h"
#include <fstream>

class PtsFileReader : public IScanFileReader
{
public:
    static bool getReader(const std::filesystem::path& filepath, std::wstring& log, IScanFileReader** reader, const AsciiImport::Info& asciiInfo);
    ~PtsFileReader();
    FileType getType() const override;
    uint32_t getScanCount() const override;
    uint64_t getTotalPoints() const override;

    bool startReadingScan(uint32_t scanNumber) override;
    bool readPoints(PointXYZIRGB* dstBuf, uint64_t bufSize, uint64_t& readCount) override;

    const tls::FileHeader& getTlsHeader() const override;
    tls::ScanHeader getTlsScanHeader(uint32_t scanNumber) const override;


private:
    PtsFileReader(const std::filesystem::path& filepath, const AsciiImport::Info& asciiInfo);

    bool getNextPoint(const std::string& line, PointXYZIRGB& point);
    bool fillPoint(const std::string& value, PointXYZIRGB& point, const AsciiImport::ValueRole& valueRole);

private:
    uint64_t totalPointCount;

    tls::FileHeader m_header;
    tls::ScanHeader m_scanHeader;

    std::ifstream m_streamReadScan;

    AsciiImport::Info m_asciiInfo;
};

#endif