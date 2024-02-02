#include "io/imports/TlsFileReader.h"
#include "io/imports/TlsReader.h"
#include "pointCloudEngine/OctreeDecoder.h"

#include <fstream>

bool TlsFileReader::getReader(const std::filesystem::path& filepath, std::wstring& log, TlsFileReader** reader)
{
    // open file stream
    std::ifstream istream;
    istream.open(filepath, std::ios::in | std::ios::binary | std::ios::ate);
    if (istream.fail()) {
        log += L"An error occured while opening the Scanfile '";
        log += filepath.c_str();
        log += L"'\n";
        return false;
    }

    tls::FileHeader header;

    // Test if the file is a valid TLS
    if (tls::reader::checkFile(istream, header) == false) {

        log += L"The file '";
        log += filepath.c_str();
        log += L"' is not recognized as a valid TLS file.";
        return false;
    }

    *reader = new TlsFileReader(filepath, header);

    // Create and initialize all the scans contained in the file
    for (uint32_t i = 0; i < header.scanCount; i++)
    {
        tls::ScanHeader scanHeader;
        if (tls::reader::getScanInfo(istream, header.version, scanHeader, i) == false)
        {
            log += L"The Scannumber '";
            log += i;
            log += L"' is corrupted.";
            return false;
        }
        (*reader)->m_scans.push_back(scanHeader);
    }

    istream.close();
    return true;
}

TlsFileReader::TlsFileReader(const std::filesystem::path& filepath, tls::FileHeader header)
    : IScanFileReader(filepath)
    , m_header(header)
    , m_octreeDecoder(nullptr)
{
}

TlsFileReader::~TlsFileReader()
{
    if (m_istream.is_open())
        m_istream.close();

    if (m_octreeDecoder != nullptr)
        delete m_octreeDecoder;
}

FileType TlsFileReader::getType() const
{
    return FileType::TLS;
}

uint32_t TlsFileReader::getScanCount() const
{
    return (uint32_t)m_scans.size();
}

uint64_t TlsFileReader::getTotalPoints() const
{
    uint64_t totalPoints = 0;
    for (auto scanHeader : m_scans)
    {
        totalPoints += scanHeader.pointCount;
    }
    return totalPoints;
}

const tls::FileHeader& TlsFileReader::getTlsHeader() const
{
    return m_header;
}

tls::ScanHeader TlsFileReader::getTlsScanHeader(uint32_t scanNumber) const
{
    if (scanNumber < m_scans.size())
        return m_scans[scanNumber];
    else
        return (tls::ScanHeader{});
}

bool TlsFileReader::startReadingScan(uint32_t scanNumber)
{
    if (scanNumber >= m_header.scanCount)
        return false;

    if (m_octreeDecoder != nullptr && m_currentScan == scanNumber)
    {
        m_currentCell = 0;
        return true;
    }

    if (!m_istream.is_open())
    {
        m_istream.open(m_filepath, std::ios::in | std::ios::binary | std::ios::ate);
        if (m_istream.fail()) {
            return false;
        }
    }

    // Read the octree structure but not the points (loadPoints = true)
    m_octreeDecoder = tls::reader::getNewOctreeDecoder(m_istream, m_header.version, scanNumber, true);
    if (m_octreeDecoder == nullptr)
        return false;

    m_currentScan = scanNumber;
    m_currentCell = 0;

    return true;
}

bool TlsFileReader::readPoints(PointXYZIRGB* dstBuf, uint64_t bufSize, uint64_t& readCount)
{
    uint64_t bufferOffset = 0;

    if (m_octreeDecoder == nullptr || !m_istream.is_open())
    {
        std::cerr << "ERROR: Tls Reader - Start reader a Scanbefore trying to read the points." << std::endl;
        readCount = 0;
        return false;
    }

    for (; m_currentCell < m_octreeDecoder->getCellCount(); ++m_currentCell)
    {
        // Only leaves contain points
        if (m_octreeDecoder->isLeaf(m_currentCell) == false)
            continue;

        // Stop when there not enough space remaining in the buffer
        if (bufferOffset + m_octreeDecoder->getCellPointCount(m_currentCell) > bufSize)
            break;

        // Copy the points directly in the destination buffer
        if (m_octreeDecoder->copyCellPoints(m_currentCell, dstBuf, bufSize, bufferOffset) == false)
            break;
    }

    if (bufferOffset == 0)
    {
        delete m_octreeDecoder;
        m_octreeDecoder = nullptr;
        m_istream.close();
    }

    readCount = bufferOffset;
    return true;
}