#ifndef E57_FILE_READER_H_
#define E57_FILE_READER_H_

#include "io/imports/IScanFileReader.h"
#include "openE57.h"
#include "io/E57Utils.h"
#include "io/StagingBuffers.h"

struct CVReaderWrapper
{
    e57::CompressedVectorReader cvr;
    E57AttribFormat format;
    Limits limits;

    ~CVReaderWrapper() {
        if (cvr.isOpen())
            cvr.close();
    };
};


class E57FileReader : public IScanFileReader
{
public:
    static bool getReader(const std::filesystem::path& filepath, std::wstring& log, E57FileReader** reader);
    ~E57FileReader();
    FileType getType() const override;
    uint32_t getScanCount() const override;
    uint64_t getTotalPoints() const override;

    bool startReadingScan(uint32_t scanNumber) override;
    bool readPoints(PointXYZIRGB* dstBuf, uint64_t bufSize, uint64_t& readCount) override;

    tls::FileHeader getTlsHeader() const override;
    tls::ScanHeader getTlsScanHeader(uint32_t scanNumber) const override;
    bool hasPoseTranslation(uint32_t scanNumber) const;

private:
    E57FileReader(const std::filesystem::path& filepath, e57::ImageFile imf);
    bool getData3dInfo(uint32_t _id, tls::ScanHeader& _info, std::string& errorMsg);

private:
    e57::ImageFile m_imf;

    uint64_t totalPointCount;

    tls::FileHeader m_header;
    std::vector<tls::ScanHeader> m_scanHeaders;
    std::vector<bool> m_hasPoseTranslation;

    StagingBuffers m_stagingBuffers;
    CVReaderWrapper* m_cvReaderWrapper;
};

#endif
