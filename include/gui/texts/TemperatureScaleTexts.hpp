#ifndef TEMPERATURE_SCALE_TEXTS_HPP
#define TEMPERATURE_SCALE_TEXTS_HPP

#include <QObject>

#define TEXT_TEMPERATURE_SCALE_INVALID_FILE QObject::tr("The file structure is incorrect. It must be in the form of columns R G B T.\nRGB are integers (0-255) and the temperature T is in °C, with decimals accepted (dot as a separator). Separator between columns: tab, space, or ;. File encoding must be either UTF-8 or ANSI")

#endif
