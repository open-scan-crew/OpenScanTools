#ifndef TLS_MAP_H
#define TLS_MAP_H


// Changes in TLS file version 1.1:
// * Some general infos are added to the TLS:
//    - Creation date
//    - Generated unique identifier (GUID)
//    - Total Scancount
// * A single file can store multiple point clouds:
//    - each point cloud is referenced in the general infos
//    - each point cloud has its specific infos
// * A point cloud can be:
//    - a unorganized point cloud
//    - a Scanpoint cloud
// * The specific infos of a Scanpoint cloud are:
//    - Aquisition date
//    - Scanner Model
//    - Scanner Serial Number
//    - Position of the scanner (rotation and translation)
// * All the general infos of a Scanpoint cloud are optional
// * Additionnal infos stored for each point cloud:
//    - bounding box (included in the octree root) used for drawing purposes
//  

//************************ Version 0.3 & 0.4 *************************************//
//
//-------- File Header -------------------------------------------------------------
// |MN  |Ver.|Date UTC | GUID (16 byte)    |ScCt|   empty (28 byte)                |  64
// =================================================================================
//
//   Array of Point Cloud Header @(64 + 256 * p) , {p E [0;PCCount - 1]}
//
// ------- ScanHeader -------------------------------------------------------------
// |  GUID (16 byte)   |  Empty (Hash ??)  |  Sensor Model (32 char)               |  64
// =================================================================================
// |  Sensor Serial Number (32 char)       |Date| -- | Bounding Box (6 float)      | 128
// =================================================================================
// | Qx      | Qy      | Qz      | Qw      | Tx      | Ty      | Tz      |Prec|Frmt| 192
// =================================================================================
// |Octr Addr| Pt Addr |Cell Addr| Pt Cnt  |Cell Cnt | Root Size/Pos     |Root| -- | 256
// =================================================================================


//#define TLS_MAGIC_NUMBER (uint32_t) 0x534C54AA // \252 T L S
constexpr uint32_t TLS_MAGIC_NUMBER = 0x534C54AA; // \252 T L S

//#define TLS_VERSION_0_3 (uint32_t) 0x1C333023 // # 0 3 FS
constexpr uint32_t TLS_VERSION_0_3 = 0x1C333023; // # 0 3 FS
//#define TLS_VERSION_0_4 (uint32_t) 0x1C343023 // # 0 4 FS
constexpr uint32_t TLS_VERSION_0_4 = 0x1C343023; // # 0 4 FS


#define TL_FILE_ADDR_MAGIC_NUMBER 0u
#define TL_FILE_ADDR_VERSION 4u
#define TL_FILE_ADDR_CREATION_DATE 8u
#define TL_FILE_ADDR_GUID 16u
#define TL_FILE_ADDR_SCAN_COUNT 32u

#define TL_FILE_HEADER_SIZE 64u
#define TL_SCAN_HEADER_SIZE 256u

#define TL_SCAN_ADDR_GUID 0u
#define TL_SCAN_ADDR_SENSOR_MODEL 32u
#define TL_SCAN_ADDR_SENSOR_SERIAL_NUMBER 64u
#define TL_SCAN_ADDR_ACQUISITION_DATE 96u
#define TL_SCAN_ADDR_BOUNDING_BOX 104u
#define TL_SCAN_ADDR_TRANSFORMATION 128u
#define TL_SCAN_ADDR_PRECISION 184u
#define TL_SCAN_ADDR_FORMAT 188u
#define TL_SCAN_ADDR_DATA_ADDR 192u
#define TL_SCAN_ADDR_POINT_COUNT 216u
#define TL_SCAN_ADDR_OCTREE_PARAM 224u


//************************ Version 0.5 *******************************************//
//
//-------- File Header -------------------------------------------------------------
// |MN  |Ver.|Date UTC |  File GUID (16b)  |  Scan GUID        |  Empty (Hash ??)  |  64
// =================================================================================
// |  Name (64 char)                                                               | 128
// =================================================================================
// |  Sensor Model (32 char)               |  Sensor Serial Number (32 char)       | 192
// =================================================================================
// | Acq Date| Bounding Box (6 float)      |Pr E|Pr V|Ft E| -- |ClCt|Root| Pt Cnt  |
// =================================================================================
// | Qx      | Qy      | Qz      | Qw      | Tx      | Ty      | Tz      | --      |
// =================================================================================
// |Octr Addr| Pt Addr | Root Size & Anchor| 256
// =================================================================================

constexpr uint32_t TLS_VERSION_0_5 = 0x1C353023;

// TODO - put the right addresses
// NOTE - On peut se passer de l'octree_addr si on écrit l'octree linéarisé toujours au même endroit

namespace tls::v05
{
    // Sizes
    constexpr uint32_t FILE_HEADER_SIZE = 256u;

    // Header addresses
    constexpr uint32_t FILE_ADDR_MAGIC_NUMBER = 0u;
    constexpr uint32_t FILE_ADDR_VERSION = 4u;
    constexpr uint32_t FILE_ADDR_CREATION_DATE = 8u;
    constexpr uint32_t FILE_ADDR_GUID = 16u;
    constexpr uint32_t SCAN_ADDR_GUID = 32u;

    constexpr uint32_t SCAN_ADDR_NAME = 000u;
    constexpr uint32_t SCAN_ADDR_SENSOR_MODEL = 32u;
    constexpr uint32_t SCAN_ADDR_SENSOR_SERIAL_NUMBER = 64u;
    constexpr uint32_t SCAN_ADDR_ACQUISITION_DATE = 96u;
    constexpr uint32_t SCAN_ADDR_BOUNDING_BOX = 104u;
    constexpr uint32_t SCAN_ADDR_PRECISION_ENUM = 184u;
    constexpr uint32_t SCAN_ADDR_PRECISION_VALUE = 184u;
    constexpr uint32_t SCAN_ADDR_FORMAT_ENUM = 188u;
    constexpr uint32_t ADDR_OCTREE_CELL_COUNT = 0u;
    constexpr uint32_t ADDR_OCTREE_ROOT_ID = 0u;
    constexpr uint32_t ADDR_OCTREE_ROOT_SIZE = 0u;
    constexpr uint32_t ADDR_OCTREE_ROOT_ANCHOR = 0u;
    constexpr uint32_t ADDR_POINT_COUNT = 0u;

    constexpr uint32_t SCAN_ADDR_TRANSFORMATION = 128u;

    constexpr uint32_t ADDR_OCTREE_DATA_ADDR = 0u;
    constexpr uint32_t ADDR_POINT_DATA_ADDR = 0u;
}


#endif