#include "io/imports/RcpFileReader.h"
#include "io/imports/RcsFileReader.h"
#include "models/pointCloud/PointXYZIRGB.h"
#include "utils/time.h"
#include "utils/logger.h"

// SDK Autodest ReCap
#include <data/RCProject.h>
#include <data/RCScan.h>

using namespace Autodesk::RealityComputing::Foundation;
using namespace Autodesk::RealityComputing::Data;

bool RcpFileReader::getReader(const std::filesystem::path& filepath, std::wstring& log, IScanFileReader** reader)
{
    try
    {
        *reader = new RcpFileReader(filepath, true, false);
    }
    catch (std::exception& e)
    {
        Logger::log(LoggerMode::IOLog) << e.what() << Logger::endl;
        //log += std::wstring(e.what());
        return false;
    }

    return true;
}

RcpFileReader::RcpFileReader(const std::filesystem::path& filepath, bool visiblePointsOnly, bool userEdits)
    : IScanFileReader(filepath)
    , m_currentScanReader(nullptr)
    , m_totalPointCount(0)
    , m_visiblePointsOnly(visiblePointsOnly)
    , m_userEdits(userEdits)
{
    //RCString path(filepath.wstring().c_str());
    RCString path(filepath);
    auto rcUserEdits = m_userEdits ? RCProjectUserEdits::All : RCProjectUserEdits::None;
    RCCode errorCode;
    m_project = RCProject::loadFromFile(path, RCFileAccess::ReadOnly, rcUserEdits, errorCode);
    if (m_project == nullptr)
    {
        char msg[1024];
        sprintf(msg, "Failed to load ReCap project %ls (error code %d)\n", filepath.c_str(), errorCode);
        throw (std::exception::exception(msg));
    }

    m_header.creationDate = 0;
    Logger::log(LoggerMode::IOLog) << "ReCap project ID: " << m_project->getProjectIdentifier().getString() << Logger::endl;
    m_header.guid = xg::Guid(m_project->getProjectIdentifier().getString());
    m_header.scanCount = m_project->getNumberOfScans();
    m_header.version = tls::FileVersion::V_UNKNOWN;

    m_totalPointCount = m_project->getNumberOfPoints();

    for (uint32_t s = 0; s < m_header.scanCount; ++s)
    {
        auto rcScan = m_project->getScanAt(s);
        if (rcScan == nullptr)
            break;

        RcsFileReader* scanReader = new RcsFileReader(rcScan);

        m_scanHeaders.push_back(scanReader->getTlsScanHeader(0));
        delete scanReader;
    }
}

RcpFileReader::~RcpFileReader()
{
    if (m_currentScanReader != nullptr)
        delete m_currentScanReader;
}

FileType RcpFileReader::getType() const
{
    return FileType::RCP;
}

uint32_t RcpFileReader::getScanCount() const
{
    return m_header.scanCount;
}

uint64_t RcpFileReader::getTotalPoints() const
{
    return m_totalPointCount;
}

const tls::FileHeader& RcpFileReader::getTlsHeader() const
{
    return m_header;
}

tls::ScanHeader RcpFileReader::getTlsScanHeader(uint32_t scanNumber) const
{
    if (scanNumber < m_scanHeaders.size())
        return m_scanHeaders[scanNumber];
    else
        return (tls::ScanHeader{});
}

bool RcpFileReader::startReadingScan(uint32_t _scanNumber)
{
    if (_scanNumber >= m_header.scanCount)
        return false;

    if (m_currentScanReader != nullptr)
        delete m_currentScanReader;

    m_currentScanReader = new RcsFileReader(m_project->getScanAt(_scanNumber));
    m_currentScanReader->startReadingScan(0);
    return true;
}

bool RcpFileReader::readPoints(PointXYZIRGB* dstBuf, uint64_t bufSize, uint64_t& readCount)
{
    if (m_currentScanReader == nullptr)
    {
        readCount = 0;
        return false;
    }

    return (m_currentScanReader->readPoints(dstBuf, bufSize, readCount));
}
