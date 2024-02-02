#ifndef TLS_FILE_READER_H
#define TLS_FILE_READER_H

#include "io/imports/IScanFileReader.h"
#include <vector>
#include <fstream>

class OctreeDecoder;

class TlsFileReader : public IScanFileReader
{
public:
    static bool getReader(const std::filesystem::path& filepath, std::wstring& log, TlsFileReader** reader);

    ~TlsFileReader();
    FileType getType() const override;
    uint32_t getScanCount() const override;
    uint64_t getTotalPoints() const override;

    bool startReadingScan(uint32_t scanNumber) override;
    bool readPoints(PointXYZIRGB* dstBuf, uint64_t bufSize, uint64_t& readCount) override;

    const tls::FileHeader& getTlsHeader() const;
    tls::ScanHeader getTlsScanHeader(uint32_t scanNumber) const;

private:
    TlsFileReader(const std::filesystem::path& filepath, tls::FileHeader header);

private:
    tls::FileHeader m_header;
    std::vector<tls::ScanHeader> m_scans;

    uint32_t m_currentScan;
    std::ifstream m_istream;
    OctreeDecoder* m_octreeDecoder;
    uint32_t m_currentCell;
};

#endif