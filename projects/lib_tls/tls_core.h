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
        //update,
        //render
    };

    // TODO - a "scan" should be refered a a "point cloud"
    // The tls format is not format destined to store only "scan" coming directly from
    //   a scanner (terrestrial, drone, handheld, etc)
    // The tls can store any point cloud from any source. For example, the point cloud
    //   can be merged frome "real scan".
    // The tls can even store generated point cloud that do not reflect a real physical
    //   object.


    class OctreeBase;
    class ImagePointCloud_p;
    class ImagePointCloud  // AccessPointCloud
    {
    public:
        ImagePointCloud();

        bool is_valid() const;
        void reset();

        uint32_t getCellCount() const;
        uint32_t getCellPointCount(uint32_t _cell_id) const;

        // Rendering mode functions
        bool getData(uint64_t _file_pos, void* _data_buf, uint64_t _data_size);
        bool getPointsRenderData(uint32_t _cell_id, void* _data_buf, uint64_t& _data_size);
        bool getCellRenderData(void* data_buf, uint64_t& data_size);

        // Functions to retreive decoded points
        bool getCellPoints(uint32_t _cell_id, Point* _dst_buf, uint64_t _dst_size) const;
        bool getNextPoints(Point* _dst_buf, uint64_t _dst_size, uint64_t& _point_count);

        bool getOctreeBase(OctreeBase& _octree_base);

    protected:
        friend class ImageFile;

        ImagePointCloud(std::shared_ptr<ImagePointCloud_p> _p);
        std::shared_ptr<ImagePointCloud_p> p_;
    };

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

        uint32_t getPointCloudCount() const;
        uint64_t getPointCount() const;

        FileHeader getFileHeader() const;
        ScanHeader getPointCloudHeader(uint32_t _pc_num) const;

        // NEW Class !
        ImagePointCloud getImagePointCloud(uint32_t _pc_num);

        bool appendPointCloud(const ScanHeader& info);
        bool finalizePointCloud(double add_x = 0.0, double add_y = 0.0, double add_z = 0.0);

        bool addPoints(Point const* srcBuf, uint64_t srcSize);
        bool mergePoints(Point const* srcBuf, uint64_t srcSize, const Transformation& srcTransfo, PointFormat srcFormat);

        void overwriteTransformation(uint32_t _pc_num, const Transformation& new_transfo);

    protected:
        std::shared_ptr<ImageFile_p> p_;
    };
}

#endif