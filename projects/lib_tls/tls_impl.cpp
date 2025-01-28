#include "tls_impl.h"
#include "tls_file.h"

#include "OctreeBase.h"
#include "OctreeCtor.h"
#include "OctreeDecoder.h"

#include "glm/glm.hpp"
#include "tls_transform.h"
#include "utils.h"


using namespace tls;

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

size_t tls::sizeofPointFormat(PointFormat format)
{
    switch (format)
    {
    case TL_POINT_XYZ_I:
        return 7ull;
    case TL_POINT_XYZ_RGB:
        return 9ull;
    case TL_POINT_XYZ_I_RGB:
        return 10ull;
    default:
        return 10ull;
    }
}

ImageFile_p::ImageFile_p(const std::filesystem::path& filepath, usage usage)
    : filepath_(filepath)
    , usg_(usage)
{
    switch (usg_)
    {
    case usage::read:
        open_file();
        read_headers();
        break;
    case usage::write:
        create_file();
        write_headers();
        break;
    default:
        return;
    }

}

ImageFile_p::~ImageFile_p()
{
    for (PCState pack : pcs_)
    {
        if (pack.octree_ctor_ != nullptr)
            delete pack.octree_ctor_;

        if (pack.octree_decoder_ != nullptr)
            delete pack.octree_decoder_;
    }

    fstr_.close();
}

bool ImageFile_p::is_valid_file()
{
    if (usg_ == usage::read && pcs_.empty())
        return false;

    if (usg_ == usage::write && !fstr_.is_open())
        return false;
    return true;
}

void ImageFile_p::open_file()
{
    if (!std::filesystem::exists(filepath_))
    {
        std::string msg = "File \"" + filepath_.string() + "\" does not exist.";
        results_.push_back({ result::INVALID_FILE, msg });
        return;
    }

    if (filepath_.extension() != ".tls")
    {
        std::string msg = "File \"" + filepath_.string() + "\" is not a tls.";
        results_.push_back({ result::NOT_A_TLS, msg });
        return;
    }

    fstr_.open(filepath_, std::ios::in | std::ios::binary | std::ios::ate);
    if (fstr_.fail())
    {
        std::string msg = "An unexpected error occured while opening the file '" + filepath_.string();
        results_.push_back({ result::ERROR, msg });
        return;
    }

    file_size_ = fstr_.tellg();
}

void ImageFile_p::create_file()
{
    if (std::filesystem::exists(filepath_))
    {
        std::string msg = "File \"" + filepath_.string() + "\" already exists.";
        results_.push_back({ result::INVALID_FILE, msg });
        return;
    }

    fstr_.open(filepath_, std::ios::out | std::ios::binary);
    if (fstr_.fail()) {
        std::string msg = "Error: cannot save at " + filepath_.string() + ": no such file or directory.";
        results_.push_back({ result::INVALID_FILE, msg });
        return;
    }
}


inline void seekScanHeaderPos(std::fstream& _fs, uint32_t _scanN, uint32_t _fieldPos)
{
    _fs.seekg(TL_FILE_HEADER_SIZE + _scanN * TL_SCAN_HEADER_SIZE + _fieldPos);
}

void ImageFile_p::read_headers()
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
        file_header_.version = FileVersion::V_0_3;
        break;
    case TLS_VERSION_0_4:
        file_header_.version = FileVersion::V_0_4;
        break;
    default:
        file_header_.version = FileVersion::V_UNKNOWN;
        results_.push_back({ result::INVALID_CONTENT, "TLS version unsupported" });
        return;
    }

    // Read the remaining info
    switch (file_header_.version)
    {
    case FileVersion::V_0_3:
    case FileVersion::V_0_4:
        fstr_.seekg(TL_FILE_ADDR_SCAN_COUNT);
        fstr_.read((char*)&file_header_.scanCount, sizeof(uint32_t));
        break;
    //case FileVersion::V_0_5:
    //    fstr_.seekg(TL_FILE_ADDR_CREATION_DATE);
    //    fstr_.read((char*)&_header.creationDate, sizeof(uint64_t));

    //    fstr_.seekg(TL_FILE_ADDR_GUID);
    //    fstr_.read((char*)&_header.guid, sizeof(FileGuid));
    default:
        file_header_.scanCount = 0;
    }

    // Create and initialize all the scans contained in the file
    for (uint32_t pc_i = 0; pc_i < file_header_.scanCount; pc_i++)
    {
        PCState state;
        tls::ScanHeader& infos = state.infos_;
        memset(&infos, 0, sizeof(ScanHeader));

        seekScanHeaderPos(fstr_, pc_i, TL_SCAN_ADDR_GUID);
        fstr_.read((char*)&infos.guid, sizeof(infos.guid));

        // In version 0.4, the TLS do not contain a name for each scan.
        // So we get the name of the file.
        // If there is more than one scan in the file, append with the scan number.
        infos.name = filepath_.stem().wstring();

        seekScanHeaderPos(fstr_, pc_i, TL_SCAN_ADDR_SENSOR_MODEL);
        char inSensorM[33];
        fstr_.read(inSensorM, 32);
        inSensorM[32] = '\0';
        infos.sensorModel = Utils::utf8_to_wstr(std::string(inSensorM));

        seekScanHeaderPos(fstr_, pc_i, TL_SCAN_ADDR_SENSOR_SERIAL_NUMBER);
        char inSensorSN[33];
        fstr_.read(inSensorSN, 32);
        inSensorSN[32] = '\0';
        infos.sensorSerialNumber = Utils::utf8_to_wstr(std::string(inSensorSN));

        seekScanHeaderPos(fstr_, pc_i, TL_SCAN_ADDR_ACQUISITION_DATE);
        fstr_.read((char*)&infos.acquisitionDate, sizeof(infos.acquisitionDate));

        seekScanHeaderPos(fstr_, pc_i, TL_SCAN_ADDR_LIMITS);
        fstr_.read((char*)&infos.limits, sizeof(Limits));

        seekScanHeaderPos(fstr_, pc_i, TL_SCAN_ADDR_TRANSFORMATION);
        fstr_.read((char*)&infos.transfo.quaternion, 4 * sizeof(double));
        fstr_.read((char*)&infos.transfo.translation, 3 * sizeof(double));

        seekScanHeaderPos(fstr_, pc_i, TL_SCAN_ADDR_PRECISION);
        fstr_.read((char*)&infos.precision, sizeof(PrecisionType));

        seekScanHeaderPos(fstr_, pc_i, TL_SCAN_ADDR_FORMAT);
        fstr_.read((char*)&infos.format, sizeof(PointFormat));

        seekScanHeaderPos(fstr_, pc_i, TL_SCAN_ADDR_DATA_ADDR);
        fstr_.read((char*)&state.octree_data_addr_, sizeof(uint64_t));
        fstr_.read((char*)&state.point_data_addr_, sizeof(uint64_t));
        fstr_.read((char*)&state.cell_data_addr_, sizeof(uint64_t));

        seekScanHeaderPos(fstr_, pc_i, TL_SCAN_ADDR_POINT_COUNT);
        fstr_.read((char*)&state.point_count_, sizeof(uint64_t));
        infos.pointCount = state.point_count_;
        seekScanHeaderPos(fstr_, pc_i, TL_SCAN_ADDR_OCTREE_PARAM);
        fstr_.read((char*)&state.cell_count_, sizeof(uint64_t));

        pcs_.push_back(state);
    }
}

void ImageFile_p::write_headers()
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
    uint32_t pc_count = (uint32_t)pcs_.size();
    fstr_.seekp(TL_FILE_ADDR_SCAN_COUNT);
    fstr_.write((char*)&pc_count, sizeof(uint32_t));

    // Write some byte at the end of the header to mark the end of headers
    fstr_.seekp(TL_FILE_HEADER_SIZE + pc_count * TL_SCAN_HEADER_SIZE);
    fstr_.write("_eoh", 4);
}

uint32_t ImageFile_p::getScanCount() const
{
    return (uint32_t)pcs_.size();
}

uint64_t ImageFile_p::getPointCount() const
{
    uint64_t totalPoints = 0;
    for (const PCState& state : pcs_)
    {
        totalPoints += state.point_count_;
    }
    return totalPoints;
}

bool ImageFile_p::setCurrentPointCloud(uint32_t _pc_num)
{
    if (usg_ != usage::read || _pc_num >= pcs_.size())
        return false;

    if (pcs_[_pc_num].octree_decoder_ != nullptr && current_pc_ == _pc_num)
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
    pcs_[_pc_num].octree_decoder_ = new OctreeDecoder();
    if (!getOctreeDecoder(_pc_num, *(pcs_[_pc_num].octree_decoder_), true))
        return false;

    current_pc_ = _pc_num;
    m_currentCell = 0;

    return true;
}

bool ImageFile_p::appendPointCloud(const ScanHeader& info)
{
    if (usg_ != usage::write)
        return false;

    PCState state;
    state.infos_ = info;
    state.octree_ctor_ = new OctreeCtor(info.precision, info.format);

    pcs_.push_back(state);
    current_pc_ = (uint32_t)pcs_.size() - 1;
    return true;
}

bool ImageFile_p::finalizePointCloud(double add_x, double add_y, double add_z)
{
    if (current_pc_ >= pcs_.size())
    {
        results_.push_back({ result::BAD_USAGE, "Wrong point cloud number." });
        return false;
    }

    OctreeCtor*& octree_ctor = pcs_[current_pc_].octree_ctor_;
    if (octree_ctor == nullptr)
        return false;

    // No point in octree - Autodestruct the file
    if (octree_ctor->getPointCount() == 0)
    {
        fstr_.close();
        std::filesystem::remove(filepath_);
    }
    else
    {
        // Write the octree and its data in the file
        if (fstr_.is_open() == false)
            return false;

        std::stringbuf buffer;
        // TODO(robin) - Do something with the log !
        std::ostream logStream(&buffer);

        // Encode the data contained in the octree in their final form
        if (octree_ctor != nullptr) {
            octree_ctor->encode(logStream);
        }

        ScanHeader& header = pcs_[current_pc_].infos_;
        // Generate a UUID for the scan
        if (header.guid == xg::Guid())
            header.guid = xg::newGuid();

        // Special option for translating a point cloud after inserting the points
        header.transfo.translation[0] += add_x;
        header.transfo.translation[1] += add_y;
        header.transfo.translation[2] += add_z;

        header.acquisitionDate = std::time(nullptr);

        if (!ImageFile_p::writeOctreeBase(current_pc_, *(OctreeBase*)octree_ctor, header)) {
            std::cerr << "pcc: An error occured while saving the point cloud." << std::endl;
            delete octree_ctor;
            octree_ctor = nullptr;
            return false;
        }

        fstr_.flush();
        fstr_.close();

        header.pointCount = octree_ctor->getPointCount();
        header.limits = octree_ctor->getLimits();
    }

    delete octree_ctor;
    octree_ctor = nullptr;
    return true;
}

bool ImageFile_p::readPoints(Point* dst_buf, uint64_t dst_size, uint64_t& point_count)
{
    if (current_pc_ >= pcs_.size())
        return false;

    OctreeDecoder* decoder = pcs_[current_pc_].octree_decoder_;

    if (decoder == nullptr)
    {
        std::string msg = "ERROR: no point cloud to read from.";
        results_.push_back({ result::BAD_USAGE, msg });
        point_count = 0;
        return false;
    }

    uint64_t bufferOffset = 0;

    for (; m_currentCell < decoder->getCellCount(); ++m_currentCell)
    {
        // Only leaves contain points
        if (decoder->isLeaf(m_currentCell) == false)
            continue;

        // Stop when there not enough space remaining in the buffer
        if (bufferOffset + decoder->getCellPointCount(m_currentCell) > dst_size)
            break;

        // Copy the points directly in the destination buffer
        if (decoder->copyCellPoints(m_currentCell, dst_buf, dst_size, bufferOffset) == false)
            break;
    }

    if (bufferOffset == 0)
    {
        delete decoder;
        pcs_[current_pc_].octree_decoder_ = nullptr;
    }

    point_count = bufferOffset;
    return true;
}

bool ImageFile_p::addPoints(Point const* src_buf, uint64_t src_size)
{
    if (current_pc_ >= pcs_.size() || pcs_[current_pc_].octree_ctor_ == nullptr)
    {
        std::string msg = "ERROR: no point cloud to write to.";
        results_.push_back({ result::BAD_USAGE, msg });
        return false;
    }

    for (uint64_t n = 0; n < src_size; ++n)
    {
        pcs_[current_pc_].octree_ctor_->insertPoint(src_buf[n]);
    }
    return true;
}

bool ImageFile_p::mergePoints(Point const* src_buf, uint64_t src_size, const Transformation& src_transfo, PointFormat src_format)
{
    if (current_pc_ >= pcs_.size() || pcs_[current_pc_].octree_ctor_ == nullptr)
    {
        std::string msg = "ERROR: no point cloud to write to.";
        results_.push_back({ result::BAD_USAGE, msg });
        return false;
    }

    OctreeCtor* ctor = pcs_[current_pc_].octree_ctor_;
    ScanHeader& header = pcs_[current_pc_].infos_;
    // compare destination and source transformation
    if (header.transfo == src_transfo)
    {
        for (uint64_t n = 0; n < src_size; ++n)
        {
            ctor->insertPoint(src_buf[n]);
        }
    }
    else
    {
        glm::dmat4 inv_dst_dmat = tls::transform::getInverseTransformDMatrix(header.transfo.translation, header.transfo.quaternion);
        glm::dmat4 src_dmat = tls::transform::getTransformDMatrix(src_transfo.translation, src_transfo.quaternion);
        glm::dmat4 total_transfo = inv_dst_dmat * src_dmat;

        // Select the correct conversion function
        typedef Point(*convert_fn_t)(const Point&, const glm::dmat4&);
        convert_fn_t convert_fn = convert_transfo;
        if (src_format == header.format)
        {
            convert_fn = convert_transfo;
        }
        else if (src_format == PointFormat::TL_POINT_XYZ_RGB)
        {
            convert_fn = convert_RGB_to_I_transfo;
        }
        else if (src_format == PointFormat::TL_POINT_XYZ_I)
        {
            convert_fn = convert_I_to_RGB_transfo;
        }

        for (uint64_t n = 0; n < src_size; ++n)
        {
            Point point = convert_fn(src_buf[n], total_transfo);
            ctor->insertPoint(point);
        }
    }

    return true;
}

bool ImageFile_p::getOctreeBase(uint32_t _pc_num, OctreeBase& _octree_base)
{
    if (!fstr_.is_open() || _pc_num >= pcs_.size())
        return false;

    switch (file_header_.version)
    {
    case FileVersion::V_0_3:
    case FileVersion::V_0_4:
    {
        seekScanHeaderPos(fstr_, _pc_num, TL_SCAN_ADDR_PRECISION);
        fstr_.read((char*)&_octree_base.m_precisionType, sizeof(PrecisionType));

        _octree_base.m_precisionValue = getPrecisionValue(_octree_base.m_precisionType);

        seekScanHeaderPos(fstr_, _pc_num, TL_SCAN_ADDR_LIMITS);
        fstr_.read((char*)&_octree_base.m_limits, sizeof(Limits));

        seekScanHeaderPos(fstr_, _pc_num, TL_SCAN_ADDR_FORMAT);
        fstr_.read((char*)&_octree_base.m_ptFormat, sizeof(PointFormat));

        seekScanHeaderPos(fstr_, _pc_num, TL_SCAN_ADDR_POINT_COUNT);
        fstr_.read((char*)&_octree_base.m_pointCount, sizeof(uint64_t));

        seekScanHeaderPos(fstr_, _pc_num, TL_SCAN_ADDR_OCTREE_PARAM);
        uint64_t cellCount64;
        fstr_.read((char*)&cellCount64, sizeof(uint64_t));
        _octree_base.m_cellCount = (uint32_t)cellCount64;
        fstr_.read((char*)&_octree_base.m_rootSize, sizeof(float));
        fstr_.read((char*)&_octree_base.m_rootPosition, 3 * sizeof(float));
        fstr_.read((char*)&_octree_base.m_uRootCell, sizeof(uint32_t));

        fstr_.seekg(pcs_[_pc_num].octree_data_addr_);
        _octree_base.m_vTreeCells.clear();
        _octree_base.m_vTreeCells.resize(_octree_base.m_cellCount);
        fstr_.read((char*)_octree_base.m_vTreeCells.data(), _octree_base.m_cellCount * sizeof(TreeCell));

        return true;
    }
    default:
        return false;
    }
}

bool ImageFile_p::getOctreeDecoder(uint32_t _pc_num, OctreeDecoder& _octree_decoder, bool _load_points)
{
    if (!fstr_.is_open() || _pc_num >= pcs_.size())
        return false;

    if (getOctreeBase(_pc_num, (OctreeBase&)_octree_decoder) == false)
    {
        return false;
    }

    _octree_decoder.initBuffers();

    if (_load_points)
    {
        _octree_decoder.readPointsFromFile(fstr_, pcs_[_pc_num].point_data_addr_);
    }

    return true;
}

bool ImageFile_p::writeOctreeBase(uint32_t _pc_num, OctreeBase& _octree, ScanHeader _header)
{
    if (!fstr_.is_open())
        return false;

    if (_octree.m_vertexData == nullptr) {
        std::cerr << "Error: no point present in the octree." << std::endl;
        return false;
    }

    // Actualize ScanCount in file header
    uint32_t pc_count = (uint32_t)pcs_.size();
    fstr_.seekp(TL_FILE_ADDR_SCAN_COUNT);
    fstr_.write((char*)&pc_count, sizeof(uint32_t));

    // *** Scaninfo ***
    seekScanHeaderPos(fstr_, _pc_num, TL_SCAN_ADDR_GUID);
    fstr_.write((char*)&_header.guid, sizeof(_header.guid));

    seekScanHeaderPos(fstr_, _pc_num, TL_SCAN_ADDR_SENSOR_MODEL);
    fstr_.write(Utils::wstr_to_utf8(_header.sensorModel).c_str(), 32u);

    seekScanHeaderPos(fstr_, _pc_num, TL_SCAN_ADDR_SENSOR_SERIAL_NUMBER);
    fstr_.write(Utils::wstr_to_utf8(_header.sensorSerialNumber).c_str(), 32u);

    seekScanHeaderPos(fstr_, _pc_num, TL_SCAN_ADDR_ACQUISITION_DATE);
    fstr_.write((char*)&_header.acquisitionDate, sizeof(_header.acquisitionDate));

    // *** Bounding Box ***
    seekScanHeaderPos(fstr_, _pc_num, TL_SCAN_ADDR_LIMITS);
    fstr_.write((char*)&_octree.m_limits, sizeof(Limits));

    // *** Transformation ***
    seekScanHeaderPos(fstr_, _pc_num, TL_SCAN_ADDR_TRANSFORMATION);
    fstr_.write((char*)&_header.transfo.quaternion, sizeof(glm::dvec4));
    fstr_.write((char*)&_header.transfo.translation, sizeof(glm::dvec3));

    // *** Octree ***
    seekScanHeaderPos(fstr_, _pc_num, TL_SCAN_ADDR_PRECISION);
    fstr_.write((char*)&_octree.m_precisionType, sizeof(PrecisionType));

    seekScanHeaderPos(fstr_, _pc_num, TL_SCAN_ADDR_FORMAT);
    fstr_.write((char*)&_octree.m_ptFormat, sizeof(PointFormat));

    // Skip Addresses (octree, points & cells)
    seekScanHeaderPos(fstr_, _pc_num, TL_SCAN_ADDR_POINT_COUNT);
    fstr_.write((char*)&_octree.m_pointCount, sizeof(uint64_t));
    seekScanHeaderPos(fstr_, _pc_num, TL_SCAN_ADDR_OCTREE_PARAM);
    fstr_.write((char*)&_octree.m_cellCount, sizeof(uint64_t));
    fstr_.write((char*)&_octree.m_rootSize, sizeof(float));
    fstr_.write((char*)_octree.m_rootPosition, 3 * sizeof(float));
    fstr_.write((char*)&_octree.m_uRootCell, sizeof(uint32_t));

    // Write the big data at the current end of the file
    fstr_.seekp(0, std::ios_base::end);

    // *** Serialization of the nodes in one array ***
    uint64_t octreePos = fstr_.tellp();
    fstr_.write((char*)_octree.m_vTreeCells.data(), _octree.m_vTreeCells.size() * sizeof(TreeCell));

    uint64_t vertexPos = fstr_.tellp();
    fstr_.write((char*)_octree.m_vertexData, _octree.m_vertexDataSize);

    uint64_t instancePos = fstr_.tellp();
    fstr_.write((char*)_octree.m_instData, _octree.m_cellCount * 4 * sizeof(float));

    // Write back the data addresses in the header
    seekScanHeaderPos(fstr_, _pc_num, TL_SCAN_ADDR_DATA_ADDR);
    fstr_.write((char*)&octreePos, sizeof(uint64_t));
    fstr_.write((char*)&vertexPos, sizeof(uint64_t));
    fstr_.write((char*)&instancePos, sizeof(uint64_t));

    return true;
}

bool ImageFile_p::getData(uint32_t _pc_num, uint64_t _file_offset, void* _data_buf, uint64_t _data_size)
{
    if (usg_ != usage::read || !fstr_.is_open())
        return false;

    if (_pc_num >= pcs_.size() ||
        pcs_[_pc_num].point_data_addr_ + _file_offset + _data_size > file_size_)
        return false;

    fstr_.seekg(pcs_[_pc_num].point_data_addr_ + _file_offset);
    fstr_.read((char*)_data_buf, _data_size);
    return fstr_.good();
}

bool ImageFile_p::getCellRenderData(uint32_t _pc_num, void* data_buf, size_t& data_size)
{
    if (usg_!= usage::read || !fstr_.is_open())
        return false;

    uint64_t cell_data_size = pcs_[_pc_num].cell_count_ * 16;

    if (data_buf == nullptr)
    {
        data_size = cell_data_size;
        return true;
    }
    else if (data_size <= cell_data_size)
    {
        fstr_.seekg(pcs_[_pc_num].cell_data_addr_);
        fstr_.read((char*)data_buf, cell_data_size);
        return true;
    }

    return false;
}

bool ImageFile_p::copyRawData(uint32_t _pc_num, char** pointBuffer, uint64_t& pointBufferSize, char** instanceBuffer, uint64_t& instanceBufferSize)
{
    if (_pc_num >= pcs_.size() || !fstr_.is_open())
        return false;

    PCState state = pcs_[_pc_num];

    pointBufferSize = state.point_count_ * sizeofPointFormat(state.infos_.format);
    instanceBufferSize = state.cell_count_ * 4 * sizeof(float);
    if (*pointBuffer != nullptr)
        delete (*pointBuffer);
    *pointBuffer = new char[pointBufferSize];
    if (*instanceBuffer != nullptr)
        delete (*instanceBuffer);
    *instanceBuffer = new char[instanceBufferSize];

    fstr_.seekg(state.point_data_addr_);
    fstr_.read(*pointBuffer, pointBufferSize);
    fstr_.seekg(state.cell_data_addr_);
    fstr_.read(*instanceBuffer, instanceBufferSize);

    return true;
}

void ImageFile_p::overwriteTransformation(uint32_t _pc_num, const Transformation& new_transfo)
{
    if (_pc_num >= pcs_.size() || !fstr_.is_open())
        return;

    seekScanHeaderPos(fstr_, _pc_num, TL_SCAN_ADDR_TRANSFORMATION);
    fstr_.write((char*)new_transfo.quaternion, sizeof(glm::dvec4));
    fstr_.write((char*)new_transfo.translation, sizeof(glm::dvec3));

    xg::Guid newUUID = xg::newGuid();
    seekScanHeaderPos(fstr_, _pc_num, TL_SCAN_ADDR_GUID);
    fstr_.write((char*)&newUUID, sizeof(newUUID));
}
