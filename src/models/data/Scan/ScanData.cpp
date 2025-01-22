#include "models/data/Scan/ScanData.h"
#include "utils/time.h"
#include "pointCloudEngine/PCE_core.h"

#include "qglobal.h"

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
	m_scanPath = uiScanData.getScanPath();
	m_scanGuid = uiScanData.getScanGuid();
}

void ScanData::freeScanFile()
{
	tlFreeScan(m_scanGuid);
}

void ScanData::eraseScanFile()
{
	tlHardDeleteScanFile(m_scanGuid);
}

void ScanData::setScanPath(const std::filesystem::path& scanPath)
{
	m_scanPath = scanPath;
}

const std::wstring& ScanData::getSensorModel() const
{
	return (m_sensorModel);
}

const std::wstring& ScanData::getSensorSerialNumber() const
{
	return (m_sensorSerialNumber);
}

uint32_t ScanData::getAcquisitionTime() const
{
	return (m_acquisitionTime);
}

const std::wstring ScanData::getStringAcquisitionTime() const
{
	if (m_acquisitionTime == 0)
		return L"Not available";
	else
	{
		wchar_t strDate[128];
		std::time_t acquisitionTime(m_acquisitionTime);
		std::wcsftime(strDate, sizeof(strDate), DISPLAY_WIDE_TIME_FORMAT, std::localtime(&acquisitionTime));

		return (std::wstring(strDate));
	}
}

const std::filesystem::path& ScanData::getScanPath() const
{
	return (m_scanPath);
}

std::filesystem::path ScanData::getCurrentScanPath() const
{
	std::filesystem::path currentUsedPath;
	tlGetCurrentScanPath(m_scanGuid, currentUsedPath);
	return currentUsedPath;
}

bool ScanData::getTlsPresent() const
{
	xg::Guid nullGuid;

	return (m_scanGuid != nullGuid);
}

tls::PointFormat ScanData::getPointFormat() const
{
	return m_pointFormat;
}

bool ScanData::getRGBAvailable() const
{
	return (m_pointFormat == tls::TL_POINT_XYZ_I_RGB ||
		m_pointFormat == tls::TL_POINT_XYZ_RGB);
}

bool ScanData::getIntensityAvailable() const
{
	return (m_pointFormat == tls::TL_POINT_XYZ_I ||
		m_pointFormat == tls::TL_POINT_XYZ_I_RGB);
}

uint64_t ScanData::getNbPoint() const
{
	return (m_NbPoint);
}

const tls::ScanGuid ScanData::getScanGuid() const
{
	return (m_scanGuid);
}

bool ScanData::getClippable() const
{
	return (m_clippable);
}

void ScanData::setClippable(bool clippable)
{
	m_clippable = clippable;
}