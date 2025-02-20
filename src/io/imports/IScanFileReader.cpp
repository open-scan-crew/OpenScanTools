#include "io/imports/IScanFileReader.h"
#include "io/imports/TlsFileReader.h"
#ifndef PORTABLE
#include "io/imports/E57FileReader.h"
#include "io/imports/FlsFileReader.h"
#include "io/imports/RcpFileReader.h"
#include "io/imports/RcsFileReader.h"
#include "io/imports/PtsFileReader.h"
#endif

IScanFileReader::IScanFileReader(const std::filesystem::path& filepath, const bool forceColor)
    : m_filepath(filepath)
{ }

std::filesystem::path IScanFileReader::getFilePath() const
{
    return m_filepath;
}

bool getScanFileReader(const std::filesystem::path& filepath, std::wstring& log, IScanFileReader** scanFileReader, const Import::AsciiInfo& asciiInfo, const bool forceColor)
{
    switch (FileUtils::getType(filepath))
    {
#ifndef PORTABLE
    case FileType::E57:
    {
        return E57FileReader::getReader(filepath, log, (E57FileReader**)scanFileReader);
    }
    case FileType::FARO_LS:
    {
        return FlsFileReader::getReader(filepath, log, (FlsFileReader**)scanFileReader, forceColor);
    }
    case FileType::FARO_PROJ:
    {
        return FlsFileReader::getReader(filepath, log, (FlsFileReader**)scanFileReader, forceColor);
    }
    case FileType::RCP:
    {
        return RcpFileReader::getReader(filepath, log, scanFileReader);
    }
    case FileType::RCS:
    {
        return RcsFileReader::getReader(filepath, log, scanFileReader);
    }
    case FileType::PTS:
    {
        return PtsFileReader::getReader(filepath, log, scanFileReader, asciiInfo);
    }
#endif
    case FileType::TLS:
    {
        return TlsFileReader::getReader(filepath, log, (TlsFileReader**)scanFileReader);
    }
    default:
        return false;
    }
}