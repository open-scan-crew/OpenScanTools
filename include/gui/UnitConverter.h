#ifndef UNITCONVERTER_H_
#define UNITCONVERTER_H_

#include "gui/UnitUsage.h"
#include "glm/glm.hpp"

namespace unit_converter {

	double meterToX(double value, UnitType X);
	glm::dvec3 meterToX(glm::dvec3 value, UnitType T);
	double XToMeter(double value, UnitType X);
}

#endif
