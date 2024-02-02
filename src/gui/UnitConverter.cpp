#include "gui\UnitConverter.h"

constexpr double m_to_unit[(size_t)UnitType::MAX_UNIT] = {
    1.0,         // no_unit
    1.0,         // M
    100.0,       // CM
    1000.0,      // MM
    1.09361,     // YD
    3.28084,     // FT
    39.37007874, // INCH
    1.0,         // DEG
    1.0,         // PX
    1.0,         // M3
    1000.0       // LITRE
};


double unit_converter::meterToX(double value, UnitType X)
{
    switch (X) {
        case UnitType::M:
            return value;
        case UnitType::CM:
            return value * 100.;
        case UnitType::MM:
            return value * 1000.;
        case UnitType::YD:
            return value / 0.914397;
        /*case UNIT_YD_US:
            return value / 0.9144;*/
        case UnitType::FT:
            return value / 0.3047990;
        /*case UNIT_FT_US:
            return value / 0.30480061;*/
        case UnitType::INC:
            return value / 0.0254;
        /*case UNIT_IN_US:
            return value / 0.0254;*/
        case UnitType::LITRE: //from m3 to litre
            return value * 1000.;
        default:
            return value;
    }
}

glm::dvec3 unit_converter::meterToX(glm::dvec3 v, UnitType T)
{
    return v * m_to_unit[(int)T];
}

double unit_converter::XToMeter(double value, UnitType X)
{
    switch (X) {
        case UnitType::M:
            return value;
        case UnitType::CM:
            return value / 100.;
        case UnitType::MM:
            return value / 1000.;
        case UnitType::YD:
            return value * 0.914397;
            /*case UNIT_YD_US:
                return value * 0.9144;*/
        case UnitType::FT:
            return value * 0.3047990;
            /*case UNIT_FT_US:
                return value * 0.30480061;*/
        case UnitType::INC:
            return value * 0.0254;
            /*case UNIT_IN_US:
                return value * 0.0254;*/
        case UnitType::LITRE: //from litre to m3
            return value / 1000.;
        default:
            return value;
    }
}