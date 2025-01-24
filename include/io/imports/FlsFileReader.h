#ifndef FLS_READER_H
#define FLS_READER_H

#include "io/imports/IScanFileReader.h"

//#pragma comment(linker, "\"/manifestdependency:type='win32' name='FARO.LS' version='1.1.904.0' processorArchitecture='amd64' publicKeyToken='1d23f5635ba800ab'\"")

#include <string>

class FlsFileReader : public IScanFileReader
{
public:
    static bool getReader(const std::filesystem::path& filepath, std::wstring& log, FlsFileReader** reader, const bool forceColor);

    ~FlsFileReader();
    FileType getType() const override;
    uint32_t getScanCount() const override;
    uint64_t getTotalPoints() const override;

    // IScanFileReader
    bool startReadingScan(uint32_t scanNumber);
    bool readPoints(PointXYZIRGB* dstBuf, uint64_t bufSize, uint64_t& readCount) override;

    tls::FileHeader getTlsHeader() const override;
    tls::ScanHeader getTlsScanHeader(uint32_t scanNumber) const override;

private:
    FlsFileReader(const std::filesystem::path& filepath, std::wstring& log, const bool forceColor);

private:
    tls::FileHeader m_header;
    std::vector<tls::ScanHeader> m_scanHeaders;
    bool m_readColor;
    std::vector<int> m_colorBuffer;

    std::vector<std::pair<uint32_t, uint32_t>> m_scanRowCols;

    // Reading parameters
    uint32_t m_currentScan;
    int m_currentCol;
};

#endif