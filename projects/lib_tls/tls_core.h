#ifndef TLS_CORE_H
#define TLS_CORE_H

#include "tls_def.h"
#include "PointXYZIRGB_k.h"

#include <filesystem>
#include <fstream>
#include <memory>

//class OctreeBase;
//class EmbeddedScan;
//class OctreeDecoder;


namespace tls
{
    enum class result
    {
        OK = 0x01,
        NOT_A_TLS = 0x02,
        ERROR = 0x03,
        INVALID_FILE = 0x04,
        INVALID_CONTENT,
        BAD_USAGE = 0x05,
    };

    enum class usage_options
    {
        read,
        write,
        shred,
        render,
    };

    // TODO - a "scan" should be refered a a "point cloud"
    // The tls format is not format destined to store only "scan" coming directly from
    //   a scanner (terrestrial, drone, handheld, etc)
    // The tls can store any point cloud from any source. For example, the point cloud
    //   can be merged frome "real scan".
    // The tls can even store generated point cloud that do not reflect a real physical
    //   object.

    class ImageFile_p;
    class ImageFile
    {
        ImageFile(const std::filesystem::path& filepath, usage_options usage);
        ~ImageFile();

        ImageFile() = delete;
        ImageFile(ImageFile const&) = delete;
        void operator=(ImageFile const&) = delete;

        result open();
        result close();

        result integrity_check();

        uint32_t getScanCount() const;
        uint64_t getTotalPoints() const;

        tls::FileHeader getFileHeader() const;
        tls::ScanHeader getScanHeader(uint32_t n) const;

        bool setCurrentPointCloud(uint32_t n); // for reading or adding points
        bool appendPointCloud(const tls::ScanHeader& info, const Transformation& transfo);
        bool finalizePointCloud(uint32_t n);

        bool readPoints(PointXYZIRGB* dstBuf, uint64_t bufSize, uint64_t& readCount);
        bool addPoints(PointXYZIRGB const* srcBuf, uint64_t srcSize);
        bool mergePoints(PointXYZIRGB const* srcBuf, uint64_t srcSize, const Transformation& srcTransfo, tls::PointFormat srcFormat);

    protected:
        std::shared_ptr<ImageFile_p> p_;
    };
}


#endif