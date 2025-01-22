#include "tls_impl.h"
#include "tls_file.h"

#include "OctreeBase_k.h"
#include "OctreeCtor_k.h"
#include "OctreeDecoder_k.h"

#include "../../ext/glm/glm.hpp"
#include "tls_transform.h"
#include "utils.h"


float tls::getPrecisionValue(PrecisionType precisionType)
{
    // !! CRITICAL !!
    // Do not edit the values without notification in the TLS file version
    switch (precisionType) {
    case PrecisionType::TL_OCTREE_1MM:
        return powf(2.f, -10);
    case PrecisionType::TL_OCTREE_100UM:
        return powf(2.f, -13);
    case PrecisionType::TL_OCTREE_10UM:
        return powf(2.f, -16);
    default:
        return 1.f;
        break;
    }
}

void tls::getCompatibleFormat(PointFormat& inFormat, PointFormat addFormat)
{
    switch (inFormat)
    {
    case TL_POINT_XYZ_I:
        if (addFormat == TL_POINT_XYZ_I)
            inFormat = TL_POINT_XYZ_I;
        else
            inFormat = TL_POINT_XYZ_I_RGB;
        break;

    case TL_POINT_XYZ_RGB:
        if (addFormat == TL_POINT_XYZ_RGB)
            inFormat = TL_POINT_XYZ_RGB;
        else
            inFormat = TL_POINT_XYZ_I_RGB;
        break;

    case TL_POINT_XYZ_I_RGB:
        break;

    case TL_POINT_FORMAT_UNDEFINED:
        inFormat = addFormat;
        break;

    default:
        inFormat = TL_POINT_NOT_COMPATIBLE;
        break;
    }
}


inline void seekScanHeaderPos(std::ofstream& _os, uint32_t _scanN, uint32_t _fieldPos)
{
    _os.seekp(TL_FILE_HEADER_SIZE + _scanN * TL_SCAN_HEADER_SIZE + _fieldPos);
}

tls::ImageFile_p::ImageFile_p(const std::filesystem::path& filepath, usage_options usage)
    : filepath_(filepath)
{
    switch (usage)
    {
    case usage_options::read:
        open_file();
        read_headers();
        break;
    case usage_options::write:
        create_file();
        write_headers();
        break;

    case usage_options::shred:
        break;
    case usage_options::render:
        break;
    default:
        return;
    }

}

tls::ImageFile_p::~ImageFile_p()
{

    assert(octree_ctor_ || octree_decoder_);
    if (octree_ctor_ != nullptr)
        delete octree_ctor_;

    if (octree_decoder_ != nullptr)
        delete octree_decoder_;

    fstr_.close();
}

bool tls::ImageFile_p::is_valid_file()
{
    return fstr_.is_open();
}

void tls::ImageFile_p::open_file()
{
    if (!std::filesystem::exists(filepath_))
    {
        std::string msg = "File \"" + filepath_.string() + "\" does not exist.";
        results_.push_back({ tls::result::INVALID_FILE, msg });
        return;
    }

    if (filepath_.extension() != ".tls")
    {
        std::string msg = "File \"" + filepath_.string() + "\" is not a tls.";
        results_.push_back({ tls::result::NOT_A_TLS, msg });
        return;
    }

    fstr_.open(filepath_, std::ios::in | std::ios::binary | std::ios::ate);
    if (fstr_.fail())
    {
        std::string msg = "An unexpected error occured while opening the file '" + filepath_.string();
        results_.push_back({ tls::result::ERROR, msg });
        return;
    }
}

void tls::ImageFile_p::create_file()
{
    if (std::filesystem::exists(filepath_))
    {
        std::string msg = "File \"" + filepath_.string() + "\" already exists.";
        results_.push_back({ tls::result::INVALID_FILE, msg });
        return;
    }

    fstr_.open(filepath_, std::ios::out | std::ios::binary);
    if (fstr_.fail()) {
        std::string msg = "Error: cannot save at " + filepath_.string() + ": no such file or directory.";
        results_.push_back({ tls::result::INVALID_FILE, msg });
        return;
    }
}


inline void seekScanHeaderPos(std::fstream& _fs, uint32_t _scanN, uint32_t _fieldPos)
{
    _fs.seekg(TL_FILE_HEADER_SIZE + _scanN * TL_SCAN_HEADER_SIZE + _fieldPos);
}

void tls::ImageFile_p::read_headers()
{
    if (!fstr_.is_open())
    {
        results_.push_back({ result::BAD_USAGE, "File stream not open." });
    }

    // Check the Magic Number
    uint32_t MN;
    fstr_.seekg(0);
    fstr_.read((char*)&MN, sizeof(uint32_t));
    if (MN != TLS_MAGIC_NUMBER) {
        results_.push_back({ result::INVALID_CONTENT, "Not a tls." });
        return;
    }

    uint32_t version;
        // Check the file version
    fstr_.read((char*)&version, sizeof(uint32_t));
    switch (version)
    {
    case TLS_VERSION_0_3:
        file_header_.version = tls::FileVersion::V_0_3;
        break;
    case TLS_VERSION_0_4:
        file_header_.version = tls::FileVersion::V_0_4;
        break;
    default:
        file_header_.version = tls::FileVersion::V_UNKNOWN;
        results_.push_back({ result::INVALID_CONTENT, "TLS version unsupported" });
        return;
    }

    // Read the remaining info
    switch (file_header_.version)
    {
    case tls::FileVersion::V_0_3:
    case tls::FileVersion::V_0_4:
        fstr_.seekg(TL_FILE_ADDR_SCAN_COUNT);
        fstr_.read((char*)&file_header_.scanCount, sizeof(uint32_t));

    //case tls::FileVersion::V_0_5:
    //    fstr_.seekg(TL_FILE_ADDR_CREATION_DATE);
    //    fstr_.read((char*)&_header.creationDate, sizeof(uint64_t));

    //    fstr_.seekg(TL_FILE_ADDR_GUID);
    //    fstr_.read((char*)&_header.guid, sizeof(tls::FileGuid));
    //    return result::OK;
    default:
        file_header_.scanCount = 0;
    }

    // Create and initialize all the scans contained in the file
    for (uint32_t pc_i = 0; pc_i < file_header_.scanCount; pc_i++)
    {
        tls::ScanHeader pc_header;
        memset(&pc_header, 0, sizeof(tls::ScanHeader));

        seekScanHeaderPos(fstr_, pc_i, TL_SCAN_ADDR_GUID);
        fstr_.read((char*)&pc_header.guid, sizeof(pc_header.guid));

        seekScanHeaderPos(fstr_, pc_i, TL_SCAN_ADDR_SENSOR_MODEL);
        char inSensorM[33];
        fstr_.read(inSensorM, 32);
        inSensorM[32] = '\0';
        pc_header.sensorModel = Utils::utf8_to_wstr(std::string(inSensorM));

        seekScanHeaderPos(fstr_, pc_i, TL_SCAN_ADDR_SENSOR_SERIAL_NUMBER);
        char inSensorSN[33];
        fstr_.read(inSensorSN, 32);
        inSensorSN[32] = '\0';
        pc_header.sensorSerialNumber = Utils::utf8_to_wstr(std::string(inSensorSN));

        seekScanHeaderPos(fstr_, pc_i, TL_SCAN_ADDR_ACQUISITION_DATE);
        fstr_.read((char*)&pc_header.acquisitionDate, sizeof(pc_header.acquisitionDate));

        seekScanHeaderPos(fstr_, pc_i, TL_SCAN_ADDR_LIMITS);
        fstr_.read((char*)&pc_header.limits, sizeof(Limits));

        seekScanHeaderPos(fstr_, pc_i, TL_SCAN_ADDR_TRANSFORMATION);
        fstr_.read((char*)&pc_header.transfo.quaternion, 4 * sizeof(double));
        fstr_.read((char*)&pc_header.transfo.translation, 3 * sizeof(double));

        seekScanHeaderPos(fstr_, pc_i, TL_SCAN_ADDR_PRECISION);
        fstr_.read((char*)&pc_header.precision, sizeof(PrecisionType));

        seekScanHeaderPos(fstr_, pc_i, TL_SCAN_ADDR_FORMAT);
        fstr_.read((char*)&pc_header.format, sizeof(PointFormat));

        // jump to the "point count"
        seekScanHeaderPos(fstr_, pc_i, TL_SCAN_ADDR_POINT_COUNT);
        fstr_.read((char*)&pc_header.pointCount, sizeof(uint64_t));

        pc_headers_.push_back(pc_header);
    }
}

void tls::ImageFile_p::write_headers()
{
    if (!fstr_.is_open())
        return;

    uint32_t MN = TLS_MAGIC_NUMBER;
    uint32_t ver = TLS_VERSION_0_4;

    fstr_.seekp(0u);
    fstr_.write((char*)&MN, sizeof(uint32_t));
    fstr_.write((char*)&ver, sizeof(uint32_t));

    fstr_.seekp(TL_FILE_ADDR_CREATION_DATE);
    std::time_t utcTime;
    fstr_.write((char*)&utcTime, sizeof(utcTime));

    // Random guid
    fstr_.seekp(TL_FILE_ADDR_GUID);
    auto guid = xg::newGuid();
    fstr_.write((char*)&guid.bytes(), sizeof(guid));

    // ScanCount
    uint32_t pc_count = (uint32_t)pc_headers_.size();
    fstr_.seekp(TL_FILE_ADDR_SCAN_COUNT);
    fstr_.write((char*)&pc_count, sizeof(uint32_t));

    // Write some byte at the end of the header to mark the end of headers
    fstr_.seekp(TL_FILE_HEADER_SIZE + pc_count * TL_SCAN_HEADER_SIZE);
    fstr_.write("_eoh", 4);
}

uint32_t tls::ImageFile_p::getScanCount() const
{
    return (uint32_t)pc_headers_.size();
}

uint64_t tls::ImageFile_p::getPointCount() const
{
    uint64_t totalPoints = 0;
    for (auto scanHeader : pc_headers_)
    {
        totalPoints += scanHeader.pointCount;
    }
    return totalPoints;
}

bool tls::ImageFile_p::setCurrentPointCloud(uint32_t n)
{
    if (n >= getScanCount())
        return false;

    if (n >= pc_headers_.size())
        return false;

    if (octree_decoder_ != nullptr && m_currentScan == n)
    {
        m_currentCell = 0;
        return true;
    }

    if (!fstr_.is_open())
    {
        fstr_.open(filepath_, std::ios::in | std::ios::binary | std::ios::ate);
        if (fstr_.fail()) {
            return false;
        }
    }

    // Read the octree structure but not the points (loadPoints = true)
    octree_decoder_ = new OctreeDecoder();
    if (!getOctreeDecoder(fstr_, n, *octree_decoder_, true))
        return false;

    m_currentScan = n;
    m_currentCell = 0;

    return true;

    return true;
}

bool tls::ImageFile_p::appendPointCloud(const tls::ScanHeader& info, const Transformation& transfo)
{
    // Finalize the octree and destroy it before creating a new one
    if (octree_ctor_ != nullptr)
        return false;

    pc_headers_.push_back(info);
    m_currentScan = (uint32_t)pc_headers_.size() - 1;

    octree_ctor_ = new OctreeCtor(info.precision, info.format);
    //scan_transfo_ = transfo;
    //m_scanPointCount = 0;
    return true;
}

bool tls::ImageFile_p::finalizePointCloud(uint32_t _pc_number, double add_x, double add_y, double add_z)
{
    if (octree_ctor_ == nullptr)
        return false;
    // No point in octree - Autodestruct the file
    if (octree_ctor_->getPointCount() == 0)
    {
        fstr_.close();
        std::filesystem::remove(filepath_);
    }
    else
    {
        // Write the octree and its data in the file
        if (fstr_.is_open() == false)
            return false;

        if (_pc_number >= pc_headers_.size())
        {
            results_.push_back({ result::BAD_USAGE, "Wrong point cloud number." });
            return false;
        }

        std::stringbuf buffer;
        // TODO(robin) - Do something with the log !
        std::ostream logStream(&buffer);

        // Encode the data contained in the octree in their final form
        if (octree_ctor_ != nullptr) {
            octree_ctor_->encode(logStream);
        }

        tls::ScanHeader& header = pc_headers_[_pc_number];
        // Generate a UUID for the scan
        if (header.guid == xg::Guid())
            header.guid = xg::newGuid();

        // Special option for translating a point cloud after inserting the points
        header.transfo.translation[0] += add_x;
        header.transfo.translation[1] += add_y;
        header.transfo.translation[2] += add_z;

        header.acquisitionDate = std::time(nullptr);

        if (!tls::ImageFile_p::writeOctreeCtor(fstr_, _pc_number, *octree_ctor_, header)) {
            std::cerr << "pcc: An error occured while saving the point cloud." << std::endl;
            delete octree_ctor_;
            octree_ctor_ = nullptr;
            return false;
        }

        fstr_.flush();
        fstr_.close();

        header.pointCount = octree_ctor_->getPointCount();
        header.limits = octree_ctor_->getLimits();
    }

    delete octree_ctor_;
    octree_ctor_ = nullptr;
    return true;
}

bool tls::ImageFile_p::readPoints(PointXYZIRGB* dst_buf, uint64_t dst_size, uint64_t& point_count)
{
    uint64_t bufferOffset = 0;

    if (octree_decoder_ == nullptr || !fstr_.is_open())
    {
        std::string msg = "ERROR: no point cloud to read from.";
        results_.push_back({ result::BAD_USAGE, msg });
        point_count = 0;
        return false;
    }

    for (; m_currentCell < octree_decoder_->getCellCount(); ++m_currentCell)
    {
        // Only leaves contain points
        if (octree_decoder_->isLeaf(m_currentCell) == false)
            continue;

        // Stop when there not enough space remaining in the buffer
        if (bufferOffset + octree_decoder_->getCellPointCount(m_currentCell) > dst_size)
            break;

        // Copy the points directly in the destination buffer
        if (octree_decoder_->copyCellPoints(m_currentCell, dst_buf, dst_size, bufferOffset) == false)
            break;
    }

    if (bufferOffset == 0)
    {
        delete octree_decoder_;
        octree_decoder_ = nullptr;
        fstr_.close();
    }

    point_count = bufferOffset;
    return true;
}

bool tls::ImageFile_p::addPoints(PointXYZIRGB const* src_buf, uint64_t src_size)
{
    if (octree_ctor_ == nullptr)
    {
        std::string msg = "ERROR: no point cloud to write to.";
        results_.push_back({ result::BAD_USAGE, msg });
        return false;
    }

    for (uint64_t n = 0; n < src_size; ++n)
    {
        octree_ctor_->insertPoint(src_buf[n]);
    }
    return true;
}

bool tls::ImageFile_p::mergePoints(PointXYZIRGB const* src_buf, uint64_t src_size, const Transformation& src_transfo, tls::PointFormat src_format)
{
    assert(octree_ctor_ != nullptr);
    if (octree_ctor_ == nullptr)
    {
        std::string msg = "ERROR: no point cloud to write to.";
        results_.push_back({ result::BAD_USAGE, msg });
        return false;
    }

    tls::ScanHeader& header = pc_headers_.back();
    // compare destination and source transformation
    if (header.transfo == src_transfo)
    {
        for (uint64_t n = 0; n < src_size; ++n)
        {
            octree_ctor_->insertPoint(src_buf[n]);
        }
    }
    else
    {
        glm::dmat4 inv_dst_dmat = tls::transform::getInverseTransformDMatrix(header.transfo.translation, header.transfo.quaternion);
        glm::dmat4 src_dmat = tls::transform::getTransformDMatrix(src_transfo.translation, src_transfo.quaternion);
        glm::dmat4 total_transfo = inv_dst_dmat * src_dmat;

        // Select the correct conversion function
        typedef PointXYZIRGB(*convert_fn_t)(const PointXYZIRGB&, const glm::dmat4&);
        convert_fn_t convert_fn = convert_transfo;
        if (src_format == header.format)
        {
            convert_fn = convert_transfo;
        }
        else if (src_format == tls::PointFormat::TL_POINT_XYZ_RGB)
        {
            convert_fn = convert_RGB_to_I_transfo;
        }
        else if (src_format == tls::PointFormat::TL_POINT_XYZ_I)
        {
            convert_fn = convert_I_to_RGB_transfo;
        }

        for (uint64_t n = 0; n < src_size; ++n)
        {
            PointXYZIRGB point = convert_fn(src_buf[n], total_transfo);
            octree_ctor_->insertPoint(point);
        }
    }

    return true;
}

bool tls::ImageFile_p::getOctreeBase(std::fstream& _fs, uint32_t _pc_number, OctreeBase& _octree_base)
{
    if (!_fs.is_open())
        return false;

    switch (file_header_.version)
    {
    case FileVersion::V_0_3:
    case FileVersion::V_0_4:
    {
        seekScanHeaderPos(_fs, _pc_number, TL_SCAN_ADDR_PRECISION);
        _fs.read((char*)&_octree_base.m_precisionType, sizeof(PrecisionType));

        _octree_base.m_precisionValue = tls::getPrecisionValue(_octree_base.m_precisionType);

        seekScanHeaderPos(_fs, _pc_number, TL_SCAN_ADDR_LIMITS);
        _fs.read((char*)&_octree_base.m_limits, sizeof(Limits));

        seekScanHeaderPos(_fs, _pc_number, TL_SCAN_ADDR_FORMAT);
        _fs.read((char*)&_octree_base.m_ptFormat, sizeof(PointFormat));

        seekScanHeaderPos(_fs, _pc_number, TL_SCAN_ADDR_POINT_COUNT);
        _fs.read((char*)&_octree_base.m_pointCount, sizeof(uint64_t));

        seekScanHeaderPos(_fs, _pc_number, TL_SCAN_ADDR_OCTREE_PARAM);
        // NOTE(robin) - Réctifie une erreur dans le type de cellCount
        uint64_t cellCount64;
        _fs.read((char*)&cellCount64, sizeof(uint64_t));
        _octree_base.m_cellCount = (uint32_t)cellCount64;
        _fs.read((char*)&_octree_base.m_rootSize, sizeof(float));
        _fs.read((char*)&_octree_base.m_rootPosition, 3 * sizeof(float));
        _fs.read((char*)&_octree_base.m_uRootCell, sizeof(uint32_t));

        // Read the octree directly from the file
        uint64_t octreeDataOffset;
        seekScanHeaderPos(_fs, _pc_number, TL_SCAN_ADDR_DATA_ADDR);
        _fs.read((char*)&octreeDataOffset, sizeof(uint64_t));
        _fs.seekg(octreeDataOffset);

        _octree_base.m_vTreeCells.clear();
        _octree_base.m_vTreeCells.resize(_octree_base.m_cellCount);
        _fs.read((char*)_octree_base.m_vTreeCells.data(), _octree_base.m_cellCount * sizeof(TreeCell));

        return true;
    }
    default:
        return false;
    }
}

bool tls::ImageFile_p::getOctreeDecoder(std::fstream& _fs, uint32_t _pc_number, OctreeDecoder& _octree_decoder, bool _load_points)
{
    if (!_fs.is_open())
        return false;

    if (getOctreeBase(_fs, _pc_number, (OctreeBase&)_octree_decoder) == false)
    {
        return false;
    }

    _octree_decoder.initBuffers();

    uint64_t octreeDataOffset;
    uint64_t pointDataOffset;
    uint64_t instanceDataOffset;

    seekScanHeaderPos(_fs, _pc_number, TL_SCAN_ADDR_DATA_ADDR);
    _fs.read((char*)&octreeDataOffset, sizeof(uint64_t));
    _fs.read((char*)&pointDataOffset, sizeof(uint64_t));
    _fs.read((char*)&instanceDataOffset, sizeof(uint64_t));

    if (_load_points)
    {
        uint64_t dataSize = instanceDataOffset - pointDataOffset;
        _octree_decoder.readPointsFromFile(_fs, pointDataOffset, dataSize);
    }

    return true;
}

bool tls::ImageFile_p::getDataLocation(std::fstream& _fs, uint32_t _pc_number, uint64_t& _point_data_offset, uint64_t& _instance_data_offset)
{
    if (!_fs.is_open())
        return false;

    uint64_t octreeDataOffset;

    seekScanHeaderPos(_fs, _pc_number, TL_SCAN_ADDR_DATA_ADDR);
    _fs.read((char*)&octreeDataOffset, sizeof(uint64_t));  // unused, just for shifting the read position
    _fs.read((char*)&_point_data_offset, sizeof(uint64_t));
    _fs.read((char*)&_instance_data_offset, sizeof(uint64_t));

    return true;
}

bool tls::ImageFile_p::copyRawData(std::fstream& _fs, char** pointBuffer, uint64_t& pointBufferSize, char** instanceBuffer, uint64_t& instanceBufferSize)
{
    if (!_fs.is_open())
        return false;

    uint64_t octreeDataOffset;
    uint64_t pointDataOffset;
    uint64_t instanceDataOffset;
    uint64_t pointCount;

    seekScanHeaderPos(_fs, 0, TL_SCAN_ADDR_DATA_ADDR);
    _fs.read((char*)&octreeDataOffset, sizeof(uint64_t));  // unused, just for shifting the read position
    _fs.read((char*)&pointDataOffset, sizeof(uint64_t));
    _fs.read((char*)&instanceDataOffset, sizeof(uint64_t));
    seekScanHeaderPos(_fs, 0, TL_SCAN_ADDR_POINT_COUNT);
    _fs.read((char*)&pointCount, sizeof(uint64_t));

    pointBufferSize = instanceDataOffset - pointDataOffset;
    instanceBufferSize = pointCount * 4 * sizeof(float);
    if (*pointBuffer != nullptr)
        delete (*pointBuffer);
    *pointBuffer = new char[pointBufferSize];
    if (*instanceBuffer != nullptr)
        delete (*instanceBuffer);
    *instanceBuffer = new char[instanceBufferSize];

    _fs.seekg(pointDataOffset);
    _fs.read(*pointBuffer, pointBufferSize);
    _fs.seekg(instanceDataOffset);
    _fs.read(*instanceBuffer, instanceBufferSize);

    return true;
}

bool tls::ImageFile_p::writeOctreeCtor(std::fstream& _os, uint32_t _pc_number, OctreeCtor& _octree_ctor, ScanHeader _header)
{
    if (_octree_ctor.m_vertexData == nullptr) {
        std::cerr << "Error: no point present in the octree." << std::endl;
        return false;
    }

    // *** Scaninfo ***
    seekScanHeaderPos(_os, _pc_number, TL_SCAN_ADDR_GUID);
    _os.write((char*)&_header.guid, sizeof(_header.guid));

    seekScanHeaderPos(_os, _pc_number, TL_SCAN_ADDR_SENSOR_MODEL);
    _os.write(Utils::wstr_to_utf8(_header.sensorModel).c_str(), 32u);

    seekScanHeaderPos(_os, _pc_number, TL_SCAN_ADDR_SENSOR_SERIAL_NUMBER);
    _os.write(Utils::wstr_to_utf8(_header.sensorSerialNumber).c_str(), 32u);

    seekScanHeaderPos(_os, _pc_number, TL_SCAN_ADDR_ACQUISITION_DATE);
    _os.write((char*)&_header.acquisitionDate, sizeof(_header.acquisitionDate));

    // *** Bounding Box ***
    seekScanHeaderPos(_os, _pc_number, TL_SCAN_ADDR_LIMITS);
    _os.write((char*)&_octree_ctor.m_limits, sizeof(tls::Limits));

    // *** Transformation ***
    seekScanHeaderPos(_os, _pc_number, TL_SCAN_ADDR_TRANSFORMATION);
    _os.write((char*)&_header.transfo.quaternion, sizeof(glm::dvec4));
    _os.write((char*)&_header.transfo.translation, sizeof(glm::dvec3));

    // *** Octree ***
    seekScanHeaderPos(_os, _pc_number, TL_SCAN_ADDR_PRECISION);
    _os.write((char*)&_octree_ctor.m_precisionType, sizeof(PrecisionType));

    seekScanHeaderPos(_os, _pc_number, TL_SCAN_ADDR_FORMAT);
    _os.write((char*)&_octree_ctor.m_ptFormat, sizeof(PointFormat));

    // Skip Addresses (octree, points & cells)
    seekScanHeaderPos(_os, _pc_number, TL_SCAN_ADDR_POINT_COUNT);
    _os.write((char*)&_octree_ctor.m_pointCount, sizeof(uint64_t));
    seekScanHeaderPos(_os, _pc_number, TL_SCAN_ADDR_OCTREE_PARAM);
    _os.write((char*)&_octree_ctor.m_cellCount, sizeof(uint64_t));
    _os.write((char*)&_octree_ctor.m_rootSize, sizeof(float));
    _os.write((char*)_octree_ctor.m_rootPosition, 3 * sizeof(float));
    _os.write((char*)&_octree_ctor.m_uRootCell, sizeof(uint32_t));

    // Write the big data at the current end of the file
    _os.seekp(0, std::ios_base::end);

    // *** Serialization of the nodes in one array ***
    uint64_t octreePos = _os.tellp();
    _os.write((char*)_octree_ctor.m_vTreeCells.data(), _octree_ctor.m_vTreeCells.size() * sizeof(TreeCell));

    uint64_t vertexPos = _os.tellp();
    _os.write((char*)_octree_ctor.m_vertexData, _octree_ctor.m_vertexDataSize);

    uint64_t instancePos = _os.tellp();
    _os.write((char*)_octree_ctor.m_instData, _octree_ctor.m_cellCount * 4 * sizeof(float));

    // Write back the data addresses in the header
    seekScanHeaderPos(_os, _pc_number, TL_SCAN_ADDR_DATA_ADDR);
    _os.write((char*)&octreePos, sizeof(uint64_t));
    _os.write((char*)&vertexPos, sizeof(uint64_t));
    _os.write((char*)&instancePos, sizeof(uint64_t));

    return true;
}

void tls::ImageFile_p::overwriteTransformation(std::fstream& _os, double _translation[3], double _quaternion[4])
{
    if (_os.is_open() == false)
        return;

    seekScanHeaderPos(_os, 0, TL_SCAN_ADDR_TRANSFORMATION);
    _os.write((char*)_quaternion, sizeof(glm::dvec4));
    _os.write((char*)_translation, sizeof(glm::dvec3));

    xg::Guid newUUID = xg::newGuid();
    seekScanHeaderPos(_os, 0, TL_SCAN_ADDR_GUID);
    _os.write((char*)&newUUID, sizeof(newUUID));
}

// Almost the same function as writeOctreeCtor
/*
bool tls::ImageFile_p::writeOctreeShredder(std::fstream& _os, uint32_t _pc_number, OctreeShredder & _octree_shredder, ScanHeader _header)
{
    if (_octree_shredder.m_pointData == nullptr) {
        std::cerr << "Error: no point present in the octree." << std::endl;
        return false;
    }

    // *** Scaninfo ***
    seekScanHeaderPos(_os, _pc_number, TL_SCAN_ADDR_GUID);
    _os.write((char*)&_header.guid, sizeof(_header.guid));

    seekScanHeaderPos(_os, _pc_number, TL_SCAN_ADDR_SENSOR_MODEL);
    _os.write(Utils::wstr_to_utf8(_header.sensorModel).c_str(), 32u);

    seekScanHeaderPos(_os, _pc_number, TL_SCAN_ADDR_SENSOR_SERIAL_NUMBER);
    _os.write(Utils::wstr_to_utf8(_header.sensorSerialNumber).c_str(), 32u);

    seekScanHeaderPos(_os, _pc_number, TL_SCAN_ADDR_ACQUISITION_DATE);
    _os.write((char*)&_header.acquisitionDate, sizeof(_header.acquisitionDate));

    // *** Bounding Box ***
    seekScanHeaderPos(_os, _pc_number, TL_SCAN_ADDR_LIMITS);
    _os.write((char*)&_header.limits, sizeof(tls::Limits));

    // *** Transformation ***
    seekScanHeaderPos(_os, _pc_number, TL_SCAN_ADDR_TRANSFORMATION);
    _os.write((char*)&_header.transfo.quaternion, sizeof(glm::dvec4));
    _os.write((char*)&_header.transfo.translation, sizeof(glm::dvec3));

    // *** Octree ***
    seekScanHeaderPos(_os, _pc_number, TL_SCAN_ADDR_PRECISION);
    _os.write((char*)&_octree_shredder.m_precisionType, sizeof(PrecisionType));

    seekScanHeaderPos(_os, _pc_number, TL_SCAN_ADDR_FORMAT);
    _os.write((char*)&_octree_shredder.m_ptFormat, sizeof(PointFormat));

    // Skip Addresses (octree, points & cells)
    seekScanHeaderPos(_os, _pc_number, TL_SCAN_ADDR_POINT_COUNT);
    _os.write((char*)&_octree_shredder.m_pointCount, sizeof(uint64_t));
    seekScanHeaderPos(_os, _pc_number, TL_SCAN_ADDR_OCTREE_PARAM);
    _os.write((char*)&_octree_shredder.m_cellCount, sizeof(uint64_t));
    _os.write((char*)&_octree_shredder.m_rootSize, sizeof(float));
    _os.write((char*)_octree_shredder.m_rootPosition, 3 * sizeof(float));
    _os.write((char*)&_octree_shredder.m_uRootCell, sizeof(uint32_t));

    // Write the big data at the current end of the file
    _os.seekp(0, std::ios_base::end);

    // *** Serialization of the nodes in one array ***
    uint64_t octreePos = _os.tellp();
    _os.write((char*)_octree_shredder.m_newTreeCells.data(), _octree_shredder.m_newTreeCells.size() * sizeof(TreeCell));

    uint64_t vertexPos = _os.tellp();
    _os.write((char*)_octree_shredder.m_pointData, _octree_shredder.m_pointDataSize);

    uint64_t instancePos = _os.tellp();
    _os.write((char*)_octree_shredder.m_newInstanceData, _octree_shredder.m_cellCount * 4 * sizeof(float));

    // Write back the data addresses in the header
    seekScanHeaderPos(_os, _pc_number, TL_SCAN_ADDR_DATA_ADDR);
    _os.write((char*)&octreePos, sizeof(uint64_t));
    _os.write((char*)&vertexPos, sizeof(uint64_t));
    _os.write((char*)&instancePos, sizeof(uint64_t));

    return true;
}
*/