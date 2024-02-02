#include "gui/UnitUsage.h"

QString unit_converter::getUnitText(UnitType type)
{
	if (unitTexts.find(type) != unitTexts.end())
		return unitTexts.at(type);
	else {
		//assert(!"Unit text not found");
		return QString("notfound");
	}
}
