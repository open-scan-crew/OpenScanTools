#ifndef MARKER_H
#define MARKER_H

#include <stdint.h>

namespace scs
{
    enum class MarkerIcon
    {
        Scan_Base,
        Target,
        BeamBending,
        ColumnTilt,
        PointObject,
        ViewPoint,
        // Tag Category
        Tag_Base,
        Tag_Attention,
        Tag_Cone,
        Tag_Delete,
        Tag_Drop,
        Tag_Flag,
        Tag_Information,
        Tag_Instrumentation,
        Tag_Recycle,
        Tag_Repair,
        Tag_Tick,
        Tag_Trashcan,
        // Danger_1 Category
        Picto_Danger_EX,
        Picto_Danger_Fall,
        Picto_Danger_Electricity,
        Picto_Danger_Laser,
        Picto_Danger_Radiation,
        Picto_Danger_Hot_Surface,
        Picto_Danger_Falling_Objects,
        Picto_Danger_Camera,
        Picto_Danger_Forklift,
        // Danger_2 Category
        Picto_Danger_Attention,
        Picto_Danger_Comburant,
        Picto_Danger_Combustible,
        Picto_Danger_Explosive,
        Picto_Danger_Mortal,
        Picto_Danger_Pression,
        Picto_Danger_Health,
        Picto_Danger_Toxic,
        // Epi Category
        Picto_Epi_Ari,
        Picto_Epi_Helmet,
        Picto_Epi_Shoes,
        Picto_Epi_Filter,
        Picto_Epi_Gloves,
        Picto_Epi_Harness,
        Picto_Epi_Goggles,
        Picto_Epi_Mask,
        Picto_Epi_NoiseProtection,
        Picto_Epi_Uniform,
        Picto_Epi_Hygiene_Cap,
        Picto_Epi_Overshoes,
        // Rescue Category
        Picto_Rescue_Ari,
        Picto_Rescue_Stop,
        Picto_Rescue_Defibrillator,
        Picto_Rescue_Shower,
        Picto_Rescue_RallyPoint,
        Picto_Rescue_EyeShower,
        Picto_Rescue_FirstAid,
        Picto_Rescue_Exit,
        Picto_Rescue_Phone,
        // Security Category
        Picto_Security_Alarm,
        Picto_Security_Extinguisher,
        Picto_Security_Ria,
        Picto_Security_Phone,
        // Arborescence
        Torus,
        Sphere,
        Cylinder,
        Box,
        Grid,
        PCO,
        Simple_Measure,
        Polyline_Measure,
        PointToPlane_Measure,
        PipeToPipe_Measure,
        PointToPipe_Measure,
        PipeToPlane_Measure,
        MeshObject,
        Max_Enum
    };

    enum class MarkerCategory
    {
        Tag,
        Epi,
        Danger_2,
        Danger_1,
        Security,
        Rescue,
        Max_Enum
    };

    enum class MarkerShape
    {
        Centered = 0,
        Top_No_Arrow,
        Top_Arrow,
        Max_Enum
    };

    struct PrimitiveDef
    {
        uint16_t firstVertex;
        uint16_t vertexCount;
    };
}

#endif
