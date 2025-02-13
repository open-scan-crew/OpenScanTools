#ifndef I_SCAN_FILE_WRITER_H
#define I_SCAN_FILE_WRITER_H

#include "io/FileUtils.h"
#include "tls_def.h"
#include "models/graph/TransformationModule.h"

#include <string>
#include <filesystem>


struct PointXYZIRGB;

class IScanFileWriter
{
public:
    IScanFileWriter(const std::filesystem::path& filepath);
    virtual ~IScanFileWriter() {};
    std::filesystem::path getFilePath() const;
    uint32_t getScanCount() const;
    uint64_t getTotalPoints() const;
    virtual uint64_t getScanPointCount() const;
    virtual tls::ScanHeader getLastScanHeader() const;

    virtual FileType getType() const = 0;

    virtual bool appendPointCloud(const tls::ScanHeader& header, const TransformationModule& transfo) = 0;
    // Add points with no transformation attached
    // The coordinates are in the local referential of the new scan
    virtual bool addPoints(PointXYZIRGB const* srcBuf, uint64_t srcSize) = 0;
    virtual bool mergePoints(PointXYZIRGB const* srcBuf, uint64_t srcSize, const TransformationModule& src_transfo, tls::PointFormat srcFormat) = 0;
    // FIXME - Need to be a "setPostTranslation" that can affect the point encoding depending of the File format:
    //  * tls, e57 have their own referential system
    //  * rcs is in global coordinates (but can be in local)
    //  * pts is in global coordinates
    virtual void setPostTranslation(const glm::dvec3& translation);

    /// <summary>
    /// Call this function when all the points have been added.
    /// Depending on the format, it will encode, write a file.
    /// No more points can be added after this function.
    /// This function must be called before appending a new point cloud.
    /// </summary>
    /// <returns>'true' after a successfull finalisation.</returns>
    virtual bool finalizePointCloud() = 0;

protected:
    std::filesystem::path m_filepath;
    uint32_t m_currentScanCount;
    uint64_t m_totalPointCount;
    uint64_t m_scanPointCount;
    std::vector<tls::ScanHeader> out_scan_headers;

    glm::dvec3 post_translation_;
};

bool getScanFileWriter(const std::filesystem::path& dirPath, const std::wstring& fileName, FileType type, std::wstring& log, IScanFileWriter**);

#endif