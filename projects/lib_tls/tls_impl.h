#ifndef TLS_IMPL_H
#define TLS_IMPL_H

#include "tls_def.h"
#include "tls_core.h"

#include "OctreeBase.h"

#include <fstream>

#define TLS_READER_DECLARE(klass) \
friend bool tls::ImageFile_p::get##klass(uint32_t _pc_number, klass& object);
#define TLS_WRITER_DECLARE(klass) \
friend bool tls::ImageFile_p::write##klass(uint32_t _scanNumber, klass& object, ScanHeader _header);

namespace tls
{
    float getPrecisionValue(PrecisionType precisionType);
    void getCompatibleFormat(PointFormat& inFormat, PointFormat addFormat);
    size_t sizeofPointFormat(PointFormat format);

    class OctreeDecoder;
    class OctreeCtor;

    class ImagePointCloud_p
    {
    public:
        ImagePointCloud_p(uint32_t _num, std::fstream& _fstr);

        bool is_valid();

        bool loadOctree();

        // Toutes les fonctions read de ImageFile sans '_pc_num'

        //bool getData(uint64_t _file_pos, void* _data_buf, uint64_t _data_size);

        //   getData(...) --> remove the '_file_pos' parameter, this is a file responsibility
        bool getPointsRenderData(uint32_t _cell_id, void* _data_buf, uint64_t& _data_size);

        //bool getCellRenderData(void* data_buf, uint64_t& data_size);

        uint32_t getCellCount() const;
        uint32_t getCellPointCount(uint32_t _cell_id) const;

        bool isLeaf(uint32_t _cell_id) const;
        bool readNextPoints(Point* dst_buf, uint64_t dst_size, uint64_t& point_count);

        // [[deprecated]] ??
        bool copyCellPoints(uint32_t _cell_id, Point* _dst_buf, uint64_t _dst_size, uint64_t& _dst_offset);

        void decodeCell(uint32_t _cell_id);

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

        // Read params
        uint32_t current_cell_ = 0;
        uint32_t current_point_ = 0;
    };


    class ImageFile_p
    {
    public:
        ImageFile_p(const std::filesystem::path& filepath, usage usage);
        ~ImageFile_p();

        bool is_valid_file();

        ImagePointCloud_p* getImagePointCloud(uint32_t _pc_num);
        bool getOctreeBase(uint32_t _pc_number, OctreeBase& _octree_base);
        bool getOctreeDecoder(uint32_t _pc_number, OctreeDecoder& _octree_decoder, bool _load_points);
        bool writeOctreeBase(uint32_t _pc_number, OctreeBase& _octree_ctor, ScanHeader _header);

    private:
        void open_file();
        void create_file();

        void read_headers();
        void write_headers();

        uint32_t getScanCount() const;
        uint64_t getPointCount() const;

        bool setCurrentPointCloud(uint32_t _pc_num);
        bool appendPointCloud(const tls::ScanHeader& info);
        bool finalizePointCloud(double add_x = 0.0, double add_y = 0.0, double add_z = 0.0);

        bool readNextPoints(Point* dst_buf, uint64_t dst_size, uint64_t& point_count);
        bool addPoints(Point const* src_buf, uint64_t src_size);
        bool mergePoints(Point const* src_buf, uint64_t src_size, const Transformation& src_transfo, tls::PointFormat src_format);

        bool getData(uint32_t _pc_num, uint64_t _file_pos, void* _data_buf, uint64_t _data_size);
        //   getData(...) --> remove the '_file_pos' parameter, this is a file responsibility
        bool getPointsRenderData(uint32_t _pc_num, uint32_t _cell_id, void* _data_buf, uint64_t& _data_size);
        bool getCellRenderData(uint32_t _pc_num, void* data_buf, uint64_t& data_size);

        // Refactor for migrating functions from EmbeddedScan
        // bool readPoints(uint32_t _cell_id, std::vector<Point>& _dst_points);
        // 

        //
        // If we want to introduce a multi component format
        // bool getPointsRenderData(uint32_t _cell_id, uint8_t _comp, void* _buf, uint64_t& _size);

    public:
        // File manipulation functions
        void overwriteTransformation(uint32_t _pc_num, const Transformation& new_transfo);

        bool copyRawData(uint32_t _pc_num, char** pointBuffer, uint64_t& pointBufferSize, char** instanceBuffer, uint64_t& instanceBufferSize);

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
            OctreeDecoder* octree_decoder_ = nullptr;
            OctreeCtor* octree_ctor_ = nullptr;
            uint64_t octree_data_addr_ = 0;
            uint64_t point_data_addr_ = 0;
            uint64_t cell_data_addr_ = 0;
            // Doublons avec les données de l’octree
            uint64_t point_count_ = 0;
            uint64_t cell_count_ = 0;
        };

        tls::FileHeader file_header_;
        std::vector<PCState> pcs_;
        std::vector<std::shared_ptr<ImagePointCloud>> point_clouds_;
        size_t file_size_ = 0;

        uint32_t current_pc_ = 0;
    };

}

#endif