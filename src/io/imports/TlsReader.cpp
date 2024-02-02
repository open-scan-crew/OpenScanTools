#include "io/imports/TlsReader.h"
#include "pointCloudEngine/OctreeBase.h"
#include "pointCloudEngine/EmbeddedScan.h"
#include "pointCloudEngine/OctreeDecoder.h"

#include "io/TlsMap.h"
#include "utils/Utils.h"


bool tls::reader::checkFile(std::ifstream& _is, FileHeader& _header)
{
    if (!_is.is_open())
        return false;

    _is.seekg(0);

    // Check the Magic Number
    uint32_t MN;
    _is.read((char*)&MN, sizeof(uint32_t));
    if (MN != TLS_MAGIC_NUMBER) {

        return false;
    }

    uint32_t version;

    // Check the file version
    _is.read((char*)&version, sizeof(uint32_t));
    switch (version)
    {
    case TLS_VERSION_0_3:
        _header.version = tls::FileVersion::V_0_3;
        break;
    case TLS_VERSION_0_4:
        _header.version = tls::FileVersion::V_0_4;
        break;
    case TLS_VERSION_0_5:
        _header.version = tls::FileVersion::V_0_5;
        break;
    default:
        _header.version = tls::FileVersion::V_UNKNOWN;
        return false;
    }

    // Read the remaining info
    switch (_header.version)
    {
    case tls::FileVersion::V_0_3:
    case tls::FileVersion::V_0_4:
        _is.seekg(TL_FILE_ADDR_SCAN_COUNT);
        _is.read((char*)&_header.scanCount, sizeof(uint32_t));
        // no break;
    case tls::FileVersion::V_0_5:
        _is.seekg(TL_FILE_ADDR_CREATION_DATE);
        _is.read((char*)&_header.creationDate, sizeof(uint64_t));

        _is.seekg(TL_FILE_ADDR_GUID);
        _is.read((char*)&_header.guid, sizeof(tls::FileGuid));
        return true;
    default:
        _header.scanCount = 0;
        return false;
    }
}

inline void seekScanHeaderPos(std::ifstream& _is, uint32_t _scanN, uint32_t _fieldPos)
{
    _is.seekg(TL_FILE_HEADER_SIZE + _scanN * TL_SCAN_HEADER_SIZE + _fieldPos);
}

bool tls::reader::getScanInfo(std::ifstream& _is, FileVersion _version, ScanHeader& _info, uint32_t scanNumber)
{
    if (!_is.is_open())
        return false;

    memset(&_info, 0, sizeof(tls::ScanHeader));

    _info.version = ScanVersion::SCAN_V_0_4;

    switch (_version)
    {
    case FileVersion::V_0_3:
        _info.version = ScanVersion::SCAN_V_0_3;
    case FileVersion::V_0_4:
    {
        seekScanHeaderPos(_is, scanNumber, TL_SCAN_ADDR_GUID);
        _is.read((char*)&_info.guid, sizeof(_info.guid));

        seekScanHeaderPos(_is, scanNumber, TL_SCAN_ADDR_SENSOR_MODEL);
        char inSensorM[33];
        _is.read(inSensorM, 32);
        inSensorM[32] = '\0';
        _info.sensorModel = Utils::from_utf8(std::string(inSensorM));

        seekScanHeaderPos(_is, scanNumber, TL_SCAN_ADDR_SENSOR_SERIAL_NUMBER);
        char inSensorSN[33];
        _is.read(inSensorSN, 32);
        inSensorSN[32] = '\0';
        _info.sensorSerialNumber = Utils::from_utf8(std::string(inSensorSN));

        seekScanHeaderPos(_is, scanNumber, TL_SCAN_ADDR_ACQUISITION_DATE);
        _is.read((char*)&_info.acquisitionDate, sizeof(_info.acquisitionDate));

        seekScanHeaderPos(_is, scanNumber, TL_SCAN_ADDR_BOUNDING_BOX);
        _is.read((char*)&_info.bbox, sizeof(tls::BoundingBox));

        seekScanHeaderPos(_is, scanNumber, TL_SCAN_ADDR_TRANSFORMATION);
        _is.read((char*)&_info.transfo.quaternion, 4 * sizeof(double));
        _is.read((char*)&_info.transfo.translation, 3 * sizeof(double));

        seekScanHeaderPos(_is, scanNumber, TL_SCAN_ADDR_PRECISION);
        _is.read((char*)&_info.precision, sizeof(PrecisionType));

        seekScanHeaderPos(_is, scanNumber, TL_SCAN_ADDR_FORMAT);
        _is.read((char*)&_info.format, sizeof(PointFormat));

        // jump to the "point count"
        seekScanHeaderPos(_is, scanNumber, TL_SCAN_ADDR_POINT_COUNT);
        _is.read((char*)&_info.pointCount, sizeof(uint64_t));
        break;
    }
    case FileVersion::V_0_5:
    {
        using namespace tls::v05;
        _is.seekg(SCAN_ADDR_GUID);
        _is.read((char*)&_info.guid, sizeof(_info.guid));

        char strBuffer[80];
        _is.seekg(SCAN_ADDR_NAME);
        _is.read(strBuffer, 64);
        strBuffer[64] = '\0';
        _info.name = Utils::from_utf8(std::string(strBuffer));

        _is.seekg(SCAN_ADDR_SENSOR_MODEL);
        _is.read(strBuffer, 32);
        strBuffer[32] = '\0';
        _info.sensorModel = Utils::from_utf8(std::string(strBuffer));

        _is.seekg(SCAN_ADDR_SENSOR_SERIAL_NUMBER);
        _is.read(strBuffer, 32);
        strBuffer[32] = '\0';
        _info.sensorSerialNumber = Utils::from_utf8(std::string(strBuffer));

        _is.seekg(SCAN_ADDR_ACQUISITION_DATE);
        _is.read((char*)&_info.acquisitionDate, sizeof(_info.acquisitionDate));

        _is.seekg(SCAN_ADDR_BOUNDING_BOX);
        _is.read((char*)&_info.bbox, sizeof(tls::BoundingBox));

        _is.seekg(SCAN_ADDR_TRANSFORMATION);
        _is.read((char*)&_info.transfo.quaternion, 4 * sizeof(double));
        _is.read((char*)&_info.transfo.translation, 3 * sizeof(double));

        // Double emploi avec l'octreeBase
        _is.seekg(SCAN_ADDR_PRECISION_ENUM);
        _is.read((char*)&_info.precision, sizeof(PrecisionType));

        // Double emploi avec l'octreeBase
        _is.seekg(SCAN_ADDR_FORMAT_ENUM);
        _is.read((char*)&_info.format, sizeof(PointFormat));

        // jump to the "point count"
        _is.seekg(ADDR_POINT_COUNT);
        _is.read((char*)&_info.pointCount, sizeof(uint64_t));
        break;
    }
    default:
        return false;
    }

    return true;
}

bool tls::reader::getOctreeBase(std::ifstream& _is, FileVersion _version, OctreeBase& _octreeBase)
{
    if (!_is.is_open())
        return false;

    switch (_version)
    {
    case FileVersion::V_0_3:
    case FileVersion::V_0_4:
    {
        seekScanHeaderPos(_is, 0, TL_SCAN_ADDR_PRECISION);
        _is.read((char*)&_octreeBase.m_precisionType, sizeof(PrecisionType));

        _octreeBase.m_precisionValue = tls::getPrecisionValue(_octreeBase.m_precisionType);

        seekScanHeaderPos(_is, 0, TL_SCAN_ADDR_FORMAT);
        _is.read((char*)&_octreeBase.m_ptFormat, sizeof(PointFormat));

        seekScanHeaderPos(_is, 0, TL_SCAN_ADDR_POINT_COUNT);
        _is.read((char*)&_octreeBase.m_pointCount, sizeof(uint64_t));

        seekScanHeaderPos(_is, 0, TL_SCAN_ADDR_OCTREE_PARAM);
        // NOTE(robin) - Réctifie une erreur dans le type de cellCount
        uint64_t cellCount64;
        _is.read((char*)&cellCount64, sizeof(uint64_t));
        _octreeBase.m_cellCount = (uint32_t)cellCount64;
        _is.read((char*)&_octreeBase.m_rootSize, sizeof(float));
        _is.read((char*)&_octreeBase.m_rootPosition, 3 * sizeof(float));
        _is.read((char*)&_octreeBase.m_uRootCell, sizeof(uint32_t));

        // Read the octree directly from the file
        uint64_t octreeDataOffset;
        seekScanHeaderPos(_is, 0, TL_SCAN_ADDR_DATA_ADDR);
        _is.read((char*)&octreeDataOffset, sizeof(uint64_t));
        _is.seekg(octreeDataOffset);

        _octreeBase.m_vTreeCells.clear();
        _octreeBase.m_vTreeCells.resize(_octreeBase.m_cellCount);
        _is.read((char*)_octreeBase.m_vTreeCells.data(), _octreeBase.m_cellCount * sizeof(TreeCell));

        return true;
    }
    case FileVersion::V_0_5:
    {
        using namespace tls::v05;

        _is.seekg(SCAN_ADDR_PRECISION_ENUM);
        _is.read((char*)&_octreeBase.m_precisionType, sizeof(PrecisionType));

        _is.seekg(SCAN_ADDR_PRECISION_VALUE);
        _is.read((char*)&_octreeBase.m_precisionValue, sizeof(float));

        _is.seekg(SCAN_ADDR_FORMAT_ENUM);
        _is.read((char*)&_octreeBase.m_ptFormat, sizeof(PointFormat));

        _is.seekg(ADDR_POINT_COUNT);
        _is.read((char*)&_octreeBase.m_pointCount, sizeof(uint64_t));

        _is.seekg(ADDR_OCTREE_CELL_COUNT);
        _is.read((char*)&_octreeBase.m_cellCount, sizeof(uint32_t));

        _is.seekg(ADDR_OCTREE_ROOT_ID);
        _is.read((char*)&_octreeBase.m_uRootCell, sizeof(uint32_t));

        _is.seekg(ADDR_OCTREE_ROOT_SIZE);
        _is.read((char*)&_octreeBase.m_rootSize, sizeof(float));

        _is.seekg(ADDR_OCTREE_ROOT_ANCHOR);
        _is.read((char*)&_octreeBase.m_rootPosition, 3 * sizeof(float));

        // Read the octree directly from the file
        uint64_t octreeDataOffset;
        _is.seekg(ADDR_OCTREE_DATA_ADDR);
        _is.read((char*)&octreeDataOffset, sizeof(uint64_t));
        _is.seekg(octreeDataOffset);

        _octreeBase.m_vTreeCells.clear();
        _octreeBase.m_vTreeCells.resize(_octreeBase.m_cellCount);
        _is.read((char*)_octreeBase.m_vTreeCells.data(), _octreeBase.m_cellCount * sizeof(TreeCell));

        return true;
    }
    default:
        return false;
    }
}

bool tls::reader::getEmbeddedScan(std::ifstream& _is, FileVersion _version, EmbeddedScan& _scan)
{
    if (!_is.is_open())
        return false;

    if (getOctreeBase(_is, _version, _scan) == false)
        return false;

    uint64_t octreeDataOffset;

    seekScanHeaderPos(_is, 0, TL_SCAN_ADDR_DATA_ADDR);
    _is.read((char*)&octreeDataOffset, sizeof(uint64_t));  // unused, just for shifting the read position
    _is.read((char*)&_scan.m_pointDataOffset, sizeof(uint64_t));
    _is.read((char*)&_scan.m_instanceDataOffset, sizeof(uint64_t));

    return true;
}

OctreeDecoder* tls::reader::getNewOctreeDecoder(std::ifstream& _is, const FileVersion& version, const uint32_t& scanNumber, bool loadPoints)
{
	if (!_is.is_open())
		return nullptr;

    OctreeBase* base = new OctreeBase();
    if (getOctreeBase(_is, version, *base) == false)
    {
        delete base;
        return nullptr;
    }

    uint64_t octreeDataOffset;
    uint64_t pointDataOffset;
    uint64_t instanceDataOffset;

    seekScanHeaderPos(_is, scanNumber, TL_SCAN_ADDR_DATA_ADDR);
    _is.read((char*)&octreeDataOffset, sizeof(uint64_t));
    _is.read((char*)&pointDataOffset, sizeof(uint64_t));
    _is.read((char*)&instanceDataOffset, sizeof(uint64_t));

    tls::BoundingBox bbox;
    seekScanHeaderPos(_is, scanNumber, TL_SCAN_ADDR_BOUNDING_BOX);
    _is.read((char*)&bbox, sizeof(tls::BoundingBox));

    OctreeDecoder* octreeDecoder = new OctreeDecoder(*base, bbox);

    if (loadPoints)
    {
        uint64_t dataSize = instanceDataOffset - pointDataOffset;
        octreeDecoder->readPointsFromFile(_is, pointDataOffset, dataSize);
    }

    delete base;
    return octreeDecoder;
}

bool tls::reader::copyRawData(std::ifstream& _is, FileVersion _version, char** pointBuffer, uint64_t& pointBufferSize, char** instanceBuffer, uint64_t& instanceBufferSize)
{
    if (!_is.is_open())
        return false;

    uint64_t octreeDataOffset;
    uint64_t pointDataOffset;
    uint64_t instanceDataOffset;
    uint64_t pointCount;

    seekScanHeaderPos(_is, 0, TL_SCAN_ADDR_DATA_ADDR);
    _is.read((char*)&octreeDataOffset, sizeof(uint64_t));  // unused, just for shifting the read position
    _is.read((char*)&pointDataOffset, sizeof(uint64_t));
    _is.read((char*)&instanceDataOffset, sizeof(uint64_t));
    seekScanHeaderPos(_is, 0, TL_SCAN_ADDR_POINT_COUNT);
    _is.read((char*)&pointCount, sizeof(uint64_t));

    pointBufferSize = instanceDataOffset - pointDataOffset;
    instanceBufferSize = pointCount * 4 * sizeof(float);
    if (*pointBuffer != nullptr)
        delete (*pointBuffer);
    *pointBuffer = new char[pointBufferSize];
    if (*instanceBuffer != nullptr)
        delete (*instanceBuffer);
    *instanceBuffer = new char[instanceBufferSize];

    _is.seekg(pointDataOffset);
    _is.read(*pointBuffer, pointBufferSize);
    _is.seekg(instanceDataOffset);
    _is.read(*instanceBuffer, instanceBufferSize);

    return true;
}
