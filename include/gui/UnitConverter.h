#ifndef UNITCONVERTER_H_
#define UNITCONVERTER_H_

#include "gui/UnitUsage.h"
#include "glm/glm.hpp"

#include <QtCore/qstring.h>
#include <string>

class UnitConverter
{
public:
    static QString getUnitText(UnitType type);
    static QString getTemperatureUnitText();
    static std::string getTemperatureUnitTextStd();
    static double meterToX(double value, UnitType X);
    static glm::dvec3 meterToX(glm::dvec3 value, UnitType T);
    static double XToMeter(double value, UnitType X);
};

#endif
