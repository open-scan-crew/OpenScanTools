#ifndef TLS_FILE_H
#define TLS_FILE_H

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

#include <cstdint>

constexpr uint32_t TLS_MAGIC_NUMBER = 0x534C54AA; // \252 T L S
constexpr uint32_t TLS_VERSION_0_3 = 0x1C333023; // # 0 3 FS
constexpr uint32_t TLS_VERSION_0_4 = 0x1C343023; // # 0 4 FS

constexpr uint32_t TL_FILE_ADDR_MAGIC_NUMBER = 0u;
constexpr uint32_t TL_FILE_ADDR_VERSION = 4u;
constexpr uint32_t TL_FILE_ADDR_CREATION_DATE = 8u;
constexpr uint32_t TL_FILE_ADDR_GUID = 16u;
constexpr uint32_t TL_FILE_ADDR_SCAN_COUNT = 32u;

constexpr uint32_t TL_FILE_HEADER_SIZE = 64u;
constexpr uint32_t TL_SCAN_HEADER_SIZE = 256u;

constexpr uint32_t TL_SCAN_ADDR_GUID = 0u;
constexpr uint32_t TL_SCAN_ADDR_SENSOR_MODEL = 32u;
constexpr uint32_t TL_SCAN_ADDR_SENSOR_SERIAL_NUMBER = 64u;
constexpr uint32_t TL_SCAN_ADDR_ACQUISITION_DATE = 96u;
constexpr uint32_t TL_SCAN_ADDR_LIMITS = 104u;
constexpr uint32_t TL_SCAN_ADDR_TRANSFORMATION = 128u;
constexpr uint32_t TL_SCAN_ADDR_PRECISION = 184u;
constexpr uint32_t TL_SCAN_ADDR_FORMAT = 188u;
constexpr uint32_t TL_SCAN_ADDR_DATA_ADDR = 192u;
constexpr uint32_t TL_SCAN_ADDR_POINT_COUNT = 216u;
constexpr uint32_t TL_SCAN_ADDR_OCTREE_PARAM = 224u;


//************************ File Header -- v0.5 ****************************************//
//
// Change:
//  + Add the scale (Sx, Sy, Sz) to the transformation
//  + Add a "Name"
//  + Use a date in uint64_t
// 
//     =================================================================================
// 0   |MN  |Ver.|Date UTC |  File GUID (16b)  |  Scan GUID        |  Empty (Hash ??)  |
//     =================================================================================
// 64  |  Name (64 char)                                                               |
//     =================================================================================
// 128 |  Sensor Model (32 char)               |  Sensor Serial Number (32 char)       |
//     =================================================================================
// 192 | Acq Date| Bounding Box (6 float)      |Pr E|Pr V|Ft E| -- |ClCt|Root| Pt Cnt  |
//     =================================================================================
// 256 | Qx      | Qy      | Qz      | Qw      | Tx      | Ty      | Tz      | Sx      |
//     =================================================================================
// 320 | Sy      | Sz      |Octr Addr| Pt Addr | Root Size & Anchor|                   |
//     =================================================================================

#endif