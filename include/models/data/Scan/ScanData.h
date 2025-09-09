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

    // New attribute to determine if a scan is an object
    void setIsObject(bool is_object);
    bool getIsObject() const;

    void setClippable(bool clippable);
    bool getClippable() const;

    void freeScanFile() const;
    void eraseScanFile() const;

    const tls::ScanGuid getScanGuid() const;
    bool getTlsPresent() const;

    tls::PointFormat getPointFormat() const;
    bool getRGBAvailable() const;
    bool getIntensityAvailable() const;
    uint64_t getNbPoint() const;

    std::wstring getSensorModel() const;
    std::wstring getSensorSerialNumber() const;
    time_t getAcquisitionTime() const;
    const std::wstring getStringAcquisitionTime() const;

    std::filesystem::path getBackupFilePath() const;

protected:
    // We store the file path even if the TlScanOverseer also store it.
    // It is used to look for the file when it is missing.
    std::filesystem::path backup_file_path_;
    tls::ScanGuid m_scanGuid = xg::Guid();
    bool m_clippable = true;
    bool is_object_ = false;
};


#endif // !SETTERSCANDATA_H_
