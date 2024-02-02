#ifndef SCREENSHOT_TEXTS_HPP
#define SCREENSHOT_TEXTS_HPP

#include <qobject>

//ImageWriter
#define TEXT_SCREENSHOT_FAILED QObject::tr("Failed to save the screenshot.")
#define TEXT_SCREENSHOT_TEXT_FAILED QObject::tr("Failed to save the text metadata file.")
#define TEXT_SCREENSHOT_DONE QObject::tr("%1 saved!")
#define TEXT_SCREENSHOT_START QObject::tr("Generating image, please wait...")
#define TEXT_SCREENSHOT_PROCESSING QObject::tr("Generating image, %1/%2 done")
#define TEXT_SCREENSHOT_PROCESSING_DONE QObject::tr("Image done")
#define TEXT_HD_FAILED_NOT_ENOUGH_MEMORY QObject::tr("Generation HD failed. Not Enough memory for the current settings. Please, try to lower the tile size.")

#endif