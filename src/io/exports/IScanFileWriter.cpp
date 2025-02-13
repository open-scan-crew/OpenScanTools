#include "io/exports/IScanFileWriter.h"
#include "io/exports/TlsFileWriter.h"

#ifndef PORTABLE
#include "io/exports/E57FileWriter.h"
#include "io/exports/RcpFileWriter.h"
#include "io/exports/PTSFileWriter.h"
#endif

IScanFileWriter::IScanFileWriter(const std::filesystem::path& filepath)
    : m_filepath(filepath)
    , m_currentScanCount(0)
    , m_totalPointCount(0)
    , m_scanPointCount(0)
    , post_translation_(0.0, 0.0, 0.0)
{ }

std::filesystem::path IScanFileWriter::getFilePath() const
{
    return m_filepath;
}

uint32_t IScanFileWriter::getScanCount() const
{
    return m_currentScanCount;
}

uint64_t IScanFileWriter::getTotalPoints() const
{
    return m_totalPointCount;
}

uint64_t IScanFileWriter::getScanPointCount() const
{
    return m_scanPointCount;
}

tls::ScanHeader IScanFileWriter::getLastScanHeader() const
{
    if (out_scan_headers.empty())
        return tls::ScanHeader();
    return out_scan_headers.back();
}

void IScanFileWriter::setPostTranslation(const glm::dvec3& translation)
{
    post_translation_ = translation;
}

// TODO - Change with 'dirPath' + 'fileName' without the extension
bool getScanFileWriter(const std::filesystem::path& dirPath, const std::wstring& fileName, FileType type, std::wstring& log, IScanFileWriter** scanFileWriter)
{
    switch (type)
    {
#ifndef PORTABLE
    case FileType::E57:
    {
        return E57FileWriter::getWriter(dirPath, fileName, log, scanFileWriter);
    }
    case FileType::RCP:
    {
        return RcpFileWriter::getWriter(dirPath, fileName, log, scanFileWriter);
    }
    case FileType::PTS:
    {
        return PTSFileWriter::getWriter(dirPath, fileName, log, scanFileWriter);
    }
#endif
    case FileType::TLS:
    {
        return TlsFileWriter::getWriter(dirPath, fileName, log, scanFileWriter);
    }
    case FileType::FARO_LS:
    case FileType::FARO_PROJ:
    {
        log += L"Error: This is not possible to export to any Faro format.";
        return false;
    }
    default:
        return false;
    }
}