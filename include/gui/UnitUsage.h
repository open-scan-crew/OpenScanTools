#ifndef UNIT_USAGE_H
#define UNIT_USAGE_H

#include <qobject.h>
#include <unordered_map>

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

namespace unit_converter {

	static const std::unordered_map<UnitType, QString> unitTexts = {
			{UnitType::NO_UNIT, QString()},

			{UnitType::M, QObject::tr(" m")},
			{UnitType::CM, QObject::tr(" cm")},
			{UnitType::MM, QObject::tr(" mm")},

			{UnitType::YD, QObject::tr(" yd")},
			{UnitType::FT, QObject::tr(" ft")},
			{UnitType::INC, QObject::tr(" in")},

			{UnitType::DEG, QString::fromStdString(" °")},
			{UnitType::PX, QString::fromStdString(" px")},

			{UnitType::M3, QString::fromStdString(" m³")},
			{UnitType::LITRE, QString::fromStdString(" L")}

			/*{UnitType::YD_US, TEXT_UNIT_YD_US},
			{UnitType::FT_US, TEXT_UNIT_FT_US},
			{UnitType::IN_US, TEXT_UNIT_IN_US}*/
	};

	QString getUnitText(UnitType type);
}

#endif
