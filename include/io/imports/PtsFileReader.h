#ifndef PTS_FILE_READER_H_
#define PTS_FILE_READER_H_

#include "io/imports/IScanFileReader.h"
#include "glm/glm.hpp"
#include <cstdint>
#include <fstream>

class PtsFileReader : public IScanFileReader
{
public:
    static bool getReader(const std::filesystem::path& filepath, std::wstring& log, IScanFileReader** reader, const Import::AsciiInfo& asciiInfo);
    ~PtsFileReader();
    FileType getType() const override;
    uint32_t getScanCount() const override;
    uint64_t getTotalPoints() const override;

    bool startReadingScan(uint32_t scanNumber) override;
    bool readPoints(PointXYZIRGB* dstBuf, uint64_t bufSize, uint64_t& readCount) override;

    tls::FileHeader getTlsHeader() const override;
    tls::ScanHeader getTlsScanHeader(uint32_t scanNumber) const override;


private:
    struct AsciiPointD
    {
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
        uint8_t i = 255;
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        bool hasX = false;
        bool hasY = false;
        bool hasZ = false;
    };

    PtsFileReader(const std::filesystem::path& filepath, const Import::AsciiInfo& asciiInfo);

    bool getNextPoint(const std::string& line, AsciiPointD& point);
    bool fillPoint(const std::string& value, AsciiPointD& point, const Import::AsciiValueRole& valueRole);

private:
    uint64_t totalPointCount;

    tls::FileHeader m_header;
    tls::ScanHeader m_scanHeader;

    std::ifstream m_streamReadScan;

    Import::AsciiInfo m_asciiInfo;
    glm::dvec3 m_doubleTranslation = glm::dvec3(0.0);
};

#endif
