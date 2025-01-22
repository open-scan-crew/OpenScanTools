#ifndef TLS_IMPL_H
#define TLS_IMPL_H

#include "tls_def.h"
#include "tls_core.h"

#include <fstream>

#define TLS_READER_DECLARE(klass) \
friend bool tls::ImageFile_p::get##klass(std::fstream& _is, uint32_t _pc_number, klass& object);
#define TLS_WRITER_DECLARE(klass) \
friend bool tls::ImageFile_p::write##klass(std::fstream& _os, uint32_t _scanNumber, klass& object, ScanHeader _header);

class OctreeBase;
class OctreeCtor;
class OctreeDecoder;
class OctreeShredder;

namespace tls
{

    float getPrecisionValue(PrecisionType precisionType);
    void getCompatibleFormat(PointFormat& inFormat, PointFormat addFormat);

    class ImageFile_p
    {
    public:
        ImageFile_p(const std::filesystem::path& filepath, usage_options usage);
        ~ImageFile_p();

        bool is_valid_file();

        bool getOctreeBase(std::fstream& _fs, uint32_t _pc_number, OctreeBase& _octree_base);
        bool getOctreeDecoder(std::fstream& _is, uint32_t _pc_number, OctreeDecoder& _octree_decoder, bool _load_points);
        bool writeOctreeCtor(std::fstream& _fs, uint32_t _pc_number, OctreeCtor& _octree_ctor, ScanHeader _header);

    private:
        void open_file();
        void create_file();

        void read_headers();
        void write_headers();

        uint32_t getScanCount() const;
        uint64_t getPointCount() const;

        bool setCurrentPointCloud(uint32_t _pc_number);
        bool appendPointCloud(const tls::ScanHeader& info, const Transformation& transfo);
        bool finalizePointCloud(uint32_t _pc_number, double add_x = 0.0, double add_y = 0.0, double add_z = 0.0);

        bool readPoints(PointXYZIRGB* dst_buf, uint64_t dst_size, uint64_t& point_count);
        bool addPoints(PointXYZIRGB const* src_buf, uint64_t src_size);
        bool mergePoints(PointXYZIRGB const* src_buf, uint64_t src_size, const Transformation& src_transfo, tls::PointFormat src_format);

        // File manipulation functions


        bool getDataLocation(std::fstream& _fs, uint32_t _pc_number, uint64_t& _point_data_offset, uint64_t& _instance_data_offset);


        void overwriteTransformation(std::fstream& _fs, double _translation[3], double _quaternion[4]);

        // For the octree shredder
        bool writeOctreeShredder(std::fstream& _fs, uint32_t _scanNumber, OctreeShredder& _octree_shredder, ScanHeader _header);
        bool copyRawData(std::fstream& _fs, char** pointBuffer, uint64_t& pointBufferSize, char** instanceBuffer, uint64_t& instanceBufferSize);

        friend ImageFile;

    protected:
        // Attributes
        const std::filesystem::path filepath_;
        std::fstream fstr_;

        struct ResultMessage {
            tls::result res;
            std::string msg;
        };

        std::vector<ResultMessage> results_;

        struct PC_Pack
        {
            tls::ScanHeader infos_;
            OctreeDecoder* octree_decoder_ = nullptr;
            OctreeCtor* octree_ctor_ = nullptr;
        };

        tls::FileHeader file_header_;
        std::vector<tls::ScanHeader> pc_headers_;

        uint32_t m_currentScan;
        OctreeDecoder* octree_decoder_;
        uint32_t m_currentCell;
        OctreeCtor* octree_ctor_;
    };
}

#endif