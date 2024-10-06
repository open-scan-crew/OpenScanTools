#include "io/exports/TlsWriter.h"
#include "io/TlsMap.h"
#include "pointCloudEngine/OctreeCtor.h"
#include "pointCloudEngine/OctreeShredder.h"

#include "glm/glm.hpp"
#include "utils/Utils.h"


inline void seekScanHeaderPos(std::ofstream& _os, uint32_t _scanN, uint32_t _fieldPos)
{
    _os.seekp(TL_FILE_HEADER_SIZE + _scanN * TL_SCAN_HEADER_SIZE + _fieldPos);
}

//**********   Saving multiple octree in one file *********
// - The total number of Scanto be saved in the file must be defined from the beginning
bool tls::writer::writeFileHeader(std::ofstream& _os, uint32_t _scanCount)
{
    if (_scanCount == 0)
        return false;

    uint32_t MN = TLS_MAGIC_NUMBER;
    uint32_t ver = TLS_VERSION_0_4;

    _os.seekp(0u);
    _os.write((char*)&MN, sizeof(uint32_t));
    _os.write((char*)&ver, sizeof(uint32_t));

    _os.seekp(TL_FILE_ADDR_CREATION_DATE);
    std::time_t utcTime;
    _os.write((char*)&utcTime, sizeof(utcTime));

    // Random guid
    _os.seekp(TL_FILE_ADDR_GUID);
    auto guid = xg::newGuid();
    _os.write((char*)&guid.bytes(), sizeof(guid));

    // ScanCount
    _os.seekp(TL_FILE_ADDR_SCAN_COUNT);
    _os.write((char*)&_scanCount, sizeof(uint32_t));

    // Write some byte at the end of the header to mark the eof
    _os.seekp(_scanCount * 256 + 60u);
    _os.write((char*)&_scanCount, 4);

    return true;
}


bool tls::writer::writeOctreeCtor(std::ofstream& _os, uint32_t _scanNumber, OctreeCtor const* _pOctree, ScanHeader _header)
{
    if (_pOctree->m_vertexData == nullptr) {
        std::cerr << "Error: no point present in the octree." << std::endl;
        return false;
    }

    // *** Scaninfo ***
    seekScanHeaderPos(_os, _scanNumber, TL_SCAN_ADDR_GUID);
    _os.write((char*)&_header.guid, sizeof(_header.guid));

    seekScanHeaderPos(_os, _scanNumber, TL_SCAN_ADDR_SENSOR_MODEL);
    _os.write(Utils::to_utf8(_header.sensorModel).c_str(), 32u);

    seekScanHeaderPos(_os, _scanNumber, TL_SCAN_ADDR_SENSOR_SERIAL_NUMBER);
    _os.write(Utils::to_utf8(_header.sensorSerialNumber).c_str(), 32u);

    seekScanHeaderPos(_os, _scanNumber, TL_SCAN_ADDR_ACQUISITION_DATE);
    _os.write((char*)&_header.acquisitionDate, sizeof(_header.acquisitionDate));

    // *** Bounding Box ***
    seekScanHeaderPos(_os, _scanNumber, TL_SCAN_ADDR_BOUNDING_BOX);
    _os.write((char*)&_pOctree->m_bbox, sizeof(BoundingBox));

    // *** Transformation ***
    seekScanHeaderPos(_os, _scanNumber, TL_SCAN_ADDR_TRANSFORMATION);
    _os.write((char*)&_header.transfo.quaternion, sizeof(glm::dvec4));
    _os.write((char*)&_header.transfo.translation, sizeof(glm::dvec3));

    // *** Octree ***
    seekScanHeaderPos(_os, _scanNumber, TL_SCAN_ADDR_PRECISION);
    _os.write((char*)&_pOctree->m_precisionType, sizeof(PrecisionType));

    seekScanHeaderPos(_os, _scanNumber, TL_SCAN_ADDR_FORMAT);
    _os.write((char*)&_pOctree->m_ptFormat, sizeof(PointFormat));

    // Skip Addresses (octree, points & cells)
    seekScanHeaderPos(_os, _scanNumber, TL_SCAN_ADDR_POINT_COUNT);
    _os.write((char*)&_pOctree->m_pointCount, sizeof(uint64_t));
    seekScanHeaderPos(_os, _scanNumber, TL_SCAN_ADDR_OCTREE_PARAM);
    _os.write((char*)&_pOctree->m_cellCount, sizeof(uint64_t));
    _os.write((char*)&_pOctree->m_rootSize, sizeof(float));
    _os.write((char*)_pOctree->m_rootPosition, 3 * sizeof(float));
    _os.write((char*)&_pOctree->m_uRootCell, sizeof(uint32_t));

    // Write the big data at the current end of the file
    _os.seekp(0, std::ios_base::end);

    // *** Serialization of the nodes in one array ***
    uint64_t octreePos = _os.tellp();
    _os.write((char*)_pOctree->m_vTreeCells.data(), _pOctree->m_vTreeCells.size() * sizeof(TreeCell));

    uint64_t vertexPos = _os.tellp();
    _os.write((char*)_pOctree->m_vertexData, _pOctree->m_vertexDataSize);

    uint64_t instancePos = _os.tellp();
    _os.write((char*)_pOctree->m_instData, _pOctree->m_cellCount * 4 * sizeof(float));

    // Write back the data addresses in the header
    seekScanHeaderPos(_os, _scanNumber, TL_SCAN_ADDR_DATA_ADDR);
    _os.write((char*)&octreePos, sizeof(uint64_t));
    _os.write((char*)&vertexPos, sizeof(uint64_t));
    _os.write((char*)&instancePos, sizeof(uint64_t));

    return true;
}

void tls::writer::overwriteTransformation(std::ofstream& _os, double _translation[3], double _quaternion[4])
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

//void tls::writer::overwriteUuid(std::ofstream& _os);

// Almost the same function as writeOctreeCtor
bool tls::writer::writeOctreeShredder(std::ofstream& _os, uint32_t _scanNumber, OctreeShredder const* _pOctree, ScanHeader _header)
{
    if (_pOctree->m_pointData == nullptr) {
        std::cerr << "Error: no point present in the octree." << std::endl;
        return false;
    }

    // *** Scaninfo ***
    seekScanHeaderPos(_os, _scanNumber, TL_SCAN_ADDR_GUID);
    _os.write((char*)&_header.guid, sizeof(_header.guid));

    seekScanHeaderPos(_os, _scanNumber, TL_SCAN_ADDR_SENSOR_MODEL);
    _os.write(Utils::to_utf8(_header.sensorModel).c_str(), 32u);

    seekScanHeaderPos(_os, _scanNumber, TL_SCAN_ADDR_SENSOR_SERIAL_NUMBER);
    _os.write(Utils::to_utf8(_header.sensorSerialNumber).c_str(), 32u);

    seekScanHeaderPos(_os, _scanNumber, TL_SCAN_ADDR_ACQUISITION_DATE);
    _os.write((char*)&_header.acquisitionDate, sizeof(_header.acquisitionDate));

    // *** Bounding Box ***
    seekScanHeaderPos(_os, _scanNumber, TL_SCAN_ADDR_BOUNDING_BOX);
    _os.write((char*)&_header.bbox, sizeof(BoundingBox));

    // *** Transformation ***
    seekScanHeaderPos(_os, _scanNumber, TL_SCAN_ADDR_TRANSFORMATION);
    _os.write((char*)&_header.transfo.quaternion, sizeof(glm::dvec4));
    _os.write((char*)&_header.transfo.translation, sizeof(glm::dvec3));

    // *** Octree ***
    seekScanHeaderPos(_os, _scanNumber, TL_SCAN_ADDR_PRECISION);
    _os.write((char*)&_pOctree->m_precisionType, sizeof(PrecisionType));

    seekScanHeaderPos(_os, _scanNumber, TL_SCAN_ADDR_FORMAT);
    _os.write((char*)&_pOctree->m_ptFormat, sizeof(PointFormat));

    // Skip Addresses (octree, points & cells)
    seekScanHeaderPos(_os, _scanNumber, TL_SCAN_ADDR_POINT_COUNT);
    _os.write((char*)&_pOctree->m_pointCount, sizeof(uint64_t));
    seekScanHeaderPos(_os, _scanNumber, TL_SCAN_ADDR_OCTREE_PARAM);
    _os.write((char*)&_pOctree->m_cellCount, sizeof(uint64_t));
    _os.write((char*)&_pOctree->m_rootSize, sizeof(float));
    _os.write((char*)_pOctree->m_rootPosition, 3 * sizeof(float));
    _os.write((char*)&_pOctree->m_uRootCell, sizeof(uint32_t));

    // Write the big data at the current end of the file
    _os.seekp(0, std::ios_base::end);

    // *** Serialization of the nodes in one array ***
    uint64_t octreePos = _os.tellp();
    _os.write((char*)_pOctree->m_newTreeCells.data(), _pOctree->m_newTreeCells.size() * sizeof(TreeCell));

    uint64_t vertexPos = _os.tellp();
    _os.write((char*)_pOctree->m_pointData, _pOctree->m_pointDataSize);

    uint64_t instancePos = _os.tellp();
    _os.write((char*)_pOctree->m_newInstanceData, _pOctree->m_cellCount * 4 * sizeof(float));

    // Write back the data addresses in the header
    seekScanHeaderPos(_os, _scanNumber, TL_SCAN_ADDR_DATA_ADDR);
    _os.write((char*)&octreePos, sizeof(uint64_t));
    _os.write((char*)&vertexPos, sizeof(uint64_t));
    _os.write((char*)&instancePos, sizeof(uint64_t));

    return true;
}