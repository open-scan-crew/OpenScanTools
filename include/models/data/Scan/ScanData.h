#ifndef SCANDATA_H_
#define SCANDATA_H_

#include "tls_def.h"

#include <filesystem>

class ScanData
{
public:
	ScanData();
	ScanData(const ScanData& data);
	~ScanData();

	void copyScanData(const ScanData& uiScan);

	//Functions
	void freeScanFile();
	void eraseScanFile();

	//Getters
	bool getClippable() const;

	const std::filesystem::path& getScanPath() const;
	std::filesystem::path getCurrentScanPath() const;
	bool getTlsPresent() const;
	tls::PointFormat getPointFormat() const;
	bool getRGBAvailable() const;
	bool getIntensityAvailable() const;
	uint64_t getNbPoint() const;
	const tls::ScanGuid getScanGuid() const;

	const std::wstring& getSensorModel() const;
	const std::wstring& getSensorSerialNumber() const;
	uint32_t getAcquisitionTime() const;
	const std::wstring getStringAcquisitionTime() const;

	//Setters
	void setClippable(bool clippable);
	void setScanPath(const std::filesystem::path& scanPath);


protected:
	std::filesystem::path m_scanPath = "";
	tls::PointFormat m_pointFormat = tls::TL_POINT_FORMAT_UNDEFINED;
	uint64_t m_NbPoint = 0;
	tls::ScanGuid m_scanGuid = xg::Guid();
	bool m_clippable = true;

	std::wstring m_sensorModel = L"";
	std::wstring m_sensorSerialNumber = L"";
	time_t m_acquisitionTime = 0;
};


#endif // !SETTERSCANDATA_H_
