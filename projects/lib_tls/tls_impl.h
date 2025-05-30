#ifndef TLS_IMPL_H
#define TLS_IMPL_H

#include "tls_def.h"
#include "tls_core.h"

#include "OctreeBase.h"

#include <fstream>

namespace tls
{
    float getPrecisionValue(PrecisionType precisionType);
    void getCompatibleFormat(PointFormat& inFormat, PointFormat addFormat);
    size_t sizeofPointFormat(PointFormat format);
    void seekScanHeaderPos(std::fstream& _fs, uint32_t _scanN, uint32_t _fieldPos);

    class OctreeCtor;

    class ImagePointCloud_p
    {
    public:
        ImagePointCloud_p(uint32_t _num, std::fstream& _fstr);
        ~ImagePointCloud_p();

        bool is_valid() const;

        bool loadOctree(OctreeBase& _octree_base) const;

        // TO REMOVE - temporary function until new functions are ready
        bool getData(uint64_t _file_pos, void* _data_buf, uint64_t _data_size) const;

        bool getPointsRenderData(uint32_t _cell_id, void* _data_buf, uint64_t& _data_size) const;

        /// <summary>
        ///  Return the render data for the cells specified into their respective destination
        ///  buffers.
        ///  All data formating are done by the function.
        ///  Each buffer must be allocated before the call and must have a size sufficient to 
        ///  contain the data.
        ///  For each n < '_count':
        ///  * If '_dst_bufs[n]' is 'nullptr', then the minimum size needed for it is
        ///    returned in '_dst_sizes[n]'.
        ///  * If '_dst_bufs[n]' is not 'nullptr', it must be a valid span of memory of size
        ///    of at least '_dst_sizes[n]'.
        /// </summary>
        /// <param name="_count"></param>
        /// <param name="_cell_ids"></param>
        /// <param name="_dst_bufs"></param>
        /// <param name="_dst_sizes"></param>
        /// <returns>'true' if all requested data have been successfully loaded in their buffer.
        ///          'false' otherwise.
        /// </returns>
        bool getPointsRenderData_multi(size_t _count, uint32_t* _cell_ids, void** _dst_bufs, uint64_t* _dst_sizes) const;
        bool getCellRenderData(void* _data_buf, uint64_t& _data_size) const;

        uint32_t getCellCount() const;
        uint32_t getCellPointCount(uint32_t _cell_id) const;

        bool isLeaf(uint32_t _cell_id) const;
        bool getCellPoints(uint32_t _cell_id, Point* _dst_buf, uint64_t _dst_size) const;
        bool getNextPoints(Point* _dst_buf, uint64_t _dst_size, uint64_t& _point_count);

        bool copyCellPoints(uint32_t _cell_id, Point* _dst_buf, uint64_t _dst_size, uint64_t& _dst_offset);

        void sortCellsByAddress();
        bool decodeCell(uint32_t _cell_id, Point* _dst_buf, uint64_t _dst_size) const;

        void printStats() const;

    private:
        const uint32_t num_ = 0; // number in file
        std::fstream& fstr_;
        OctreeBase octree_;
        uint64_t octree_data_addr_ = 0;
        uint64_t point_data_addr_ = 0;
        uint64_t cell_data_addr_ = 0;

        // From OctreeDecoder
        uint32_t decoded_cell_ = NO_CHILD;
        std::vector<Point> decoded_points_;
        std::vector<uint32_t> sorted_cells_;

        // Read params
        uint32_t current_cell_ = 0;
        uint32_t current_point_ = 0;

        // Stats
        mutable float alloc_time_ms = 0.f;
        mutable float read_time_ms = 0.f;
        mutable float decode_time_ms = 0.f;
        mutable float copy_time_ms = 0.f;
        mutable size_t read_size = 0;
    };


    class ImageFile_p
    {
    public:
        ImageFile_p(const std::filesystem::path& filepath, usage usage);
        ~ImageFile_p();

        bool is_valid_file();

        ImagePointCloud_p* getImagePointCloud(uint32_t _pc_num);
        bool writeOctreeBase(uint32_t _pc_number, OctreeBase& _octree_ctor, ScanHeader _header);

    private:
        void open_file();
        void create_file();

        void read_headers();
        void write_headers();

        uint32_t getPointCloudCount() const;
        uint64_t getPointCount() const;

        bool appendPointCloud(const tls::ScanHeader& info);
        bool finalizePointCloud(double add_x = 0.0, double add_y = 0.0, double add_z = 0.0);

        bool addPoints(Point const* src_buf, uint64_t src_size);
        bool mergePoints(Point const* src_buf, uint64_t src_size, const Transformation& src_transfo, tls::PointFormat src_format);

    public:
        // File manipulation functions
        void overwriteTransformation(uint32_t _pc_num, const Transformation& new_transfo);

        friend ImageFile;

    protected:
        // Creation Attributes
        const std::filesystem::path filepath_;
        const usage usg_;

        std::fstream fstr_;

        struct ResultMessage {
            tls::result res;
            std::string msg;
        };

        std::vector<ResultMessage> results_;

        struct PCState // Status, pack, infos, ?
        {
            tls::ScanHeader infos_;
            OctreeCtor* octree_ctor_ = nullptr;
        };

        tls::FileHeader file_header_;
        std::vector<PCState> pcs_;
        std::vector<std::shared_ptr<ImagePointCloud>> point_clouds_;
        size_t file_size_ = 0;

        uint32_t current_pc_ = 0;
    };

}

#endif