#ifndef TLS_CORE_H
#define TLS_CORE_H

#include "tls_def.h"
#include "tls_point.h"

#include <filesystem>
#include <memory>

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

    enum class usage
    {
        read,
        write,
        //render
    };

    // TODO - a "scan" should be refered a a "point cloud"
    // The tls format is not format destined to store only "scan" coming directly from
    //   a scanner (terrestrial, drone, handheld, etc)
    // The tls can store any point cloud from any source. For example, the point cloud
    //   can be merged frome "real scan".
    // The tls can even store generated point cloud that do not reflect a real physical
    //   object.


    class ImagePointCloud_p;
    class ImagePointCloud  // AccessPointCloud
    {
    public:
        bool is_valid() const;

        uint32_t getCellCount() const;
        uint32_t getCellPointCount(uint32_t _cell_id) const;

        bool getPointsRenderData(uint32_t _cell_id, void* _data_buf, uint64_t& _data_size);
        //bool getCellRenderData(void* data_buf, uint64_t& data_size);

        bool readNextPoints(Point* dst_buf, uint64_t dst_size, uint64_t& point_count);

    protected:
        friend class ImageFile;

        ImagePointCloud(std::shared_ptr<ImagePointCloud_p> _p);
        std::shared_ptr<ImagePointCloud_p> p_;
    };

    class OctreeBase;
    class ImageFile_p;
    class ImageFile
    {
    public:
        ImageFile();
        ImageFile(const std::filesystem::path& _filepath, usage _u);
        ~ImageFile();

        ImageFile(ImageFile const&) = delete;
        void operator=(ImageFile const&) = delete;

        bool open(const std::filesystem::path& _filepath, usage _u);
        void close();
        bool is_valid_file() const;
        std::filesystem::path getPath() const;

        uint32_t getScanCount() const;  // RENAME - getPointCloudCount()
        uint64_t getTotalPoints() const;// RENAME - getTotalPointCount()

        FileHeader getFileHeader() const;
        ScanHeader getPointCloudHeader(uint32_t _pc_num) const;

        // NEW Class !
        ImagePointCloud getImagePointCloud(uint32_t _pc_num);

        bool setCurrentPointCloud(uint32_t _pc_num); // for reading or adding points
        bool appendPointCloud(const ScanHeader& info);
        bool finalizePointCloud(double add_x = 0.0, double add_y = 0.0, double add_z = 0.0);

        bool readNextPoints(Point* dstBuf, uint64_t bufSize, uint64_t& readCount);
        bool addPoints(Point const* srcBuf, uint64_t srcSize);
        bool mergePoints(Point const* srcBuf, uint64_t srcSize, const Transformation& srcTransfo, PointFormat srcFormat);

        // Rendering mode functions
        bool getOctreeBase(uint32_t _pc_num, OctreeBase& _octree_base);
        bool getData(uint32_t _pc_num, uint64_t _file_pos, void* _data_buf, uint64_t _data_size);
        bool getCellRenderData(uint32_t _pc_num, void* data_buf, uint64_t& data_size);

        void overwriteTransformation(uint32_t _pc_num, const Transformation& new_transfo);

    protected:
        std::shared_ptr<ImageFile_p> p_;
    };
}

#endif