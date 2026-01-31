#include "gui/UnitConverter.h"

#include <QtCore/qobject.h>

#include <unordered_map>
#include <string>

static const std::unordered_map<UnitType, QString> unitTexts = {
        {UnitType::NO_UNIT, QString()},

        {UnitType::M, QObject::tr(" m")},
        {UnitType::CM, QObject::tr(" cm")},
        {UnitType::MM, QObject::tr(" mm")},

        {UnitType::YD, QObject::tr(" yd")},
        {UnitType::FT, QObject::tr(" ft")},
        {UnitType::INC, QObject::tr(" in")},

QString UnitConverter::getTemperatureUnitText()
{
    return QString::fromUtf8(u8" \u00b0C");
}

std::string UnitConverter::getTemperatureUnitTextStd()
{
    return getTemperatureUnitText().toUtf8().toStdString();
}


        {UnitType::DEG, QString::fromStdWString(L" °")},
        {UnitType::PX, QString::fromStdWString(L" px")},

        {UnitType::M3, QString::fromStdWString(L" m³")},
        {UnitType::LITRE, QString::fromStdWString(L" L")}

        /*{UnitType::YD_US, TEXT_UNIT_YD_US},
        {UnitType::FT_US, TEXT_UNIT_FT_US},
        {UnitType::IN_US, TEXT_UNIT_IN_US}*/
};

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

QString UnitConverter::getUnitText(UnitType type)
{
    if (unitTexts.find(type) != unitTexts.end())
        return unitTexts.at(type);
    else {
        //assert(!"Unit text not found");
        return QString("notfound");
    }
}

double UnitConverter::meterToX(double value, UnitType X)
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

glm::dvec3 UnitConverter::meterToX(glm::dvec3 v, UnitType T)
{
    return v * m_to_unit[(int)T];
}

double UnitConverter::XToMeter(double value, UnitType X)
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