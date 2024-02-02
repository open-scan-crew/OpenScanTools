#ifndef I_SCAN_FILE_READER_H
#define I_SCAN_FILE_READER_H

#include <string>
#include <filesystem>
#include <map>

#include "io/FileUtils.h"
#include "io/imports/ImportTypes.h"
#include "models/pointCloud/TLS.h"

struct PointXYZIRGB;
class IOctreeReader;

class IScanFileReader
{
public:
    IScanFileReader(const std::filesystem::path& filepath, const bool forceColor = true);
    virtual ~IScanFileReader() {};
    std::filesystem::path getFilePath() const;

    virtual FileType getType() const = 0;
    virtual uint32_t getScanCount() const = 0;
    virtual uint64_t getTotalPoints() const = 0;

    virtual bool startReadingScan(uint32_t scanNumber) = 0;
    virtual bool readPoints(PointXYZIRGB* dstBuf, uint64_t bufSize, uint64_t& readCount) = 0;

    virtual const tls::FileHeader& getTlsHeader() const = 0;
    virtual tls::ScanHeader getTlsScanHeader(uint32_t scanNumber) const = 0;

protected:
    std::filesystem::path m_filepath;

};

bool getScanFileReader(const std::filesystem::path& filepath, std::wstring& log, IScanFileReader**, const AsciiImport::Info& asciiInfo, const bool forceColor = true);

#endif