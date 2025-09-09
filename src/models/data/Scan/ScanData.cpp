#include "models/data/Scan/ScanData.h"
#include "utils/time.h"
#include "pointCloudEngine/PCE_core.h"


ScanData::ScanData()
{ }

ScanData::ScanData(const ScanData & data)
{
    copyScanData(data);
}

ScanData::~ScanData()
{ }

void ScanData::copyScanData(const ScanData& uiScanData)
{
    m_scanGuid = uiScanData.getScanGuid();
    m_clippable = uiScanData.m_clippable;
    is_object_ = uiScanData.is_object_;
}

void ScanData::setClippable(bool clippable)
{
    m_clippable = clippable;
}

bool ScanData::getClippable() const
{
    return (m_clippable);
}

void ScanData::setIsObject(bool is_object)
{
    is_object_ = is_object;
    // TODO - cannot change the marker icon here - not defined
    //Data::marker_icon_ = is_object ? scs::MarkerIcon::PCO : scs::MarkerIcon::Scan_Base;
}

bool ScanData::getIsObject() const
{
    return is_object_;
}

void ScanData::freeScanFile() const
{
    tlFreeScan(m_scanGuid);
}

void ScanData::eraseScanFile() const
{
    tlHardDeleteScanFile(m_scanGuid);
}

const tls::ScanGuid ScanData::getScanGuid() const
{
    return (m_scanGuid);
}

bool ScanData::getTlsPresent() const
{
    xg::Guid nullGuid;

    return (m_scanGuid != nullGuid);
}

tls::PointFormat ScanData::getPointFormat() const
{
    tls::ScanHeader header;
    tlGetScanHeader(m_scanGuid, header);
    return header.format;
}

bool ScanData::getRGBAvailable() const
{
    tls::PointFormat format = getPointFormat();
    return (format == tls::TL_POINT_XYZ_I_RGB ||
        format == tls::TL_POINT_XYZ_RGB);
}

bool ScanData::getIntensityAvailable() const
{
    tls::PointFormat format = getPointFormat();
    return (format == tls::TL_POINT_XYZ_I ||
        format == tls::TL_POINT_XYZ_I_RGB);
}

uint64_t ScanData::getNbPoint() const
{
    tls::ScanHeader header;
    tlGetScanHeader(m_scanGuid, header);
    return header.pointCount;
}

std::wstring ScanData::getSensorModel() const
{
    tls::ScanHeader header;
    tlGetScanHeader(m_scanGuid, header);
    return header.sensorModel;
}

std::wstring ScanData::getSensorSerialNumber() const
{
    tls::ScanHeader header;
    tlGetScanHeader(m_scanGuid, header);
    return header.sensorSerialNumber;
}

time_t ScanData::getAcquisitionTime() const
{
    tls::ScanHeader header;
    tlGetScanHeader(m_scanGuid, header);
    return header.acquisitionDate;
}

const std::wstring ScanData::getStringAcquisitionTime() const
{
    time_t t = getAcquisitionTime();
    if (t == 0)
        return L"Not available";
    else
    {
        wchar_t strDate[128];
        std::wcsftime(strDate, sizeof(strDate), DISPLAY_WIDE_TIME_FORMAT, std::localtime(&t));
        return (std::wstring(strDate));
    }
}

std::filesystem::path ScanData::getBackupFilePath() const
{
    return backup_file_path_;
}