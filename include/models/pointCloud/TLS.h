#ifndef TLS_H
#define TLS_H

#include "crossguid/guid.hpp"
#include "models/3d/BoundingBox.h"
#include "models/graph/TransformationModule.h"

#include <cstdint>
#include <string>
#include <list>

namespace tls
{
    enum class FileVersion
    {
        V_0_3,
        V_0_4,
        V_0_5,
        V_UNKNOWN
    };

    enum class ScanVersion
    {
        SCAN_V_0_3,
        SCAN_V_0_4,
        //SCAN_V_0_5, // for future use with the new intermidiary points packing
        V_MAX_ENUM
    };

    typedef xg::Guid FileGuid;
    typedef xg::Guid ScanGuid;

    struct Transformation
    {
        // the quaternion components are x, y, z, w
        double quaternion[4];
        double translation[3];

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

    enum PrecisionType {
        TL_OCTREE_1MM = 0,
        TL_OCTREE_100UM,
        TL_OCTREE_10UM
    };

    // !! CRITICAL !!
    // Do not edit the values without notification in the TLS file version
    inline float getPrecisionValue(tls::PrecisionType precisionType) {
        switch (precisionType) {
        case tls::TL_OCTREE_1MM:
            return powf(2.f, -10);
        case tls::TL_OCTREE_100UM:
            return powf(2.f, -13);
        case tls::TL_OCTREE_10UM:
            return powf(2.f, -16);
        default:
            return 1.f;
            break;
        }
    }

    enum PointFormat
    {
        TL_POINT_XYZ_I = 0,
        TL_POINT_XYZ_RGB,
        TL_POINT_XYZ_I_RGB,
        TL_POINT_MAX_ENUM,
        TL_POINT_FORMAT_UNDEFINED = 1001001,
        TL_POINT_NOT_COMPATIBLE
    };

    inline void getCompatibleFormat(tls::PointFormat& inFormat, tls::PointFormat addFormat)
    {
        switch (inFormat)
        {
        case tls::TL_POINT_XYZ_I:
            if (addFormat == tls::TL_POINT_XYZ_I)
                inFormat = tls::TL_POINT_XYZ_I;
            else
                inFormat = tls::TL_POINT_XYZ_I_RGB;
            break;

        case tls::TL_POINT_XYZ_RGB:
            if (addFormat == tls::TL_POINT_XYZ_RGB)
                inFormat = tls::TL_POINT_XYZ_RGB;
            else
                inFormat = tls::TL_POINT_XYZ_I_RGB;
            break;

        case tls::TL_POINT_XYZ_I_RGB:
            break;

        case tls::TL_POINT_FORMAT_UNDEFINED:
            inFormat = addFormat;
            break;

        default:
            inFormat = tls::TL_POINT_NOT_COMPATIBLE;
            break;
        }
    }

    struct ScanHeader
    {
        // File/encoding infos
        ScanGuid guid;
        ScanVersion version;
        // ScanStation infos
        uint64_t acquisitionDate;
        std::wstring name;
        std::wstring sensorModel;
        std::wstring sensorSerialNumber;
        Transformation transfo;
        // Octree infos
        BoundingBox bbox;
        uint64_t pointCount;
        PrecisionType precision;
        PointFormat format;
    };

    struct FileHeader
    {
        FileGuid guid;
        uint64_t creationDate; // use a uint32 ??
        FileVersion version;
        uint32_t scanCount; // TODO - remove the scan count in version 0.5
    };

    struct FileInfo
    {
        FileHeader fileHeader;
        std::list<ScanHeader> scansHeaders;
    };
}

#endif