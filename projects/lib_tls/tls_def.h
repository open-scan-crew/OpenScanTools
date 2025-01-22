#ifndef TLS_DEF_H
#define TLS_DEF_H

#include "crossguid/guid.hpp"

#include <cstdint>
#include <string>

namespace tls
{
    enum class FileVersion
    {
        V_0_3,
        V_0_4,
        //V_0_5,
        V_UNKNOWN
    };

    //enum class ScanVersion
    //{
    //    SCAN_V_0_3,
    //    SCAN_V_0_4,
    //    V_MAX_ENUM
    //};

    typedef xg::Guid FileGuid;
    typedef xg::Guid ScanGuid;

    struct Transformation
    {
        Transformation()
            : quaternion{ 0.0, 0.0, 0.0, 1.0 }
            , translation{ 0.0, 0.0, 0.0 }
        {
        }
        // the quaternion components order is x, y, z, w
        double quaternion[4];
        double translation[3];
        // double scale[3];

        bool operator==(const Transformation& rhs) const
        {
            return (quaternion[0] == rhs.quaternion[0] &&
                quaternion[1] == rhs.quaternion[1] &&
                quaternion[2] == rhs.quaternion[2] &&
                quaternion[3] == rhs.quaternion[3] &&
                translation[0] == rhs.translation[0] &&
                translation[1] == rhs.translation[1] &&
                translation[2] == rhs.translation[2]);
        }
    };

    enum class PrecisionType {
        TL_OCTREE_1MM = 0,
        TL_OCTREE_100UM,
        TL_OCTREE_10UM
    };

    enum PointFormat
    {
        TL_POINT_XYZ_I = 0,
        TL_POINT_XYZ_RGB,
        TL_POINT_XYZ_I_RGB,
        TL_POINT_MAX_ENUM,
        TL_POINT_FORMAT_UNDEFINED = 1001001,
        TL_POINT_NOT_COMPATIBLE
    };



    struct Limits
    {
        float xMin;
        float xMax;
        float yMin;
        float yMax;
        float zMin;
        float zMax;
    };

    struct ScanHeader
    {
        // File/encoding infos
        ScanGuid guid;
        //ScanVersion version;
        // ScanStation infos
        uint64_t acquisitionDate;
        std::wstring name;
        std::wstring sensorModel;
        std::wstring sensorSerialNumber;
        Transformation transfo;
        // Octree infos
        Limits limits;
        uint64_t pointCount;
        PrecisionType precision;
        PointFormat format;
    };

    struct FileHeader
    {
        FileGuid guid;
        uint64_t creationDate;
        FileVersion version;
        uint32_t scanCount; // TODO - remove the scan count in version 0.5
    };
}

#endif