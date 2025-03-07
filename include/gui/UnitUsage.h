#ifndef UNIT_USAGE_H
#define UNIT_USAGE_H

#include <cstdint>

enum class UnitType {
    NO_UNIT,
    M,
    CM,
    MM,
    YD,
    FT,
    INC,
    DEG,
    PX,

    //VolumeUnit
    M3,
    LITRE,
    /*YD_US,
    FT_US,
    IN_US*/
    MAX_UNIT
};

// {UnitType distanceUnit, UnitType diameterUnit, uint32_t displayedDigits}
struct UnitUsage {
    UnitType distanceUnit;
    UnitType diameterUnit;
    UnitType volumeUnit;
    uint32_t displayedDigits;
};

namespace unit_usage {
    static const UnitUsage by_default = { UnitType::M, UnitType::M, UnitType::M3, 3};
}

#endif
