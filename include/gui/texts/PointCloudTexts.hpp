#ifndef POINT_CLOUD_TEXTS_HPP
#define POINT_CLOUD_TEXTS_HPP

#include <QObject>

//ContextPointObject duplications
#define TEXT_POINT_CLOUD_OBJECT_START QObject::tr("Ready to create point cloud object. Click on the desired area in the point cloud.")
#define TEXT_POINT_CLOUD_OBJECT_DUPLICATION_DONE QObject::tr("Cloud object duplicated. Ready for the next one.")
//ContextPCOCreation
#define TEXT_POINT_CLOUD_OBJECT_FILE_EXIST QObject::tr("A file with that name already exist.")

//DialogImportAsciiPC
#define TEXT_ASCII_RED QObject::tr("Red (0-255)")
#define TEXT_ASCII_RED_FLOAT QObject::tr("Red (0.-1)")
#define TEXT_ASCII_GREEN QObject::tr("Green (0-255)")
#define TEXT_ASCII_GREEN_FLOAT QObject::tr("Green (0.-1)")
#define TEXT_ASCII_BLUE QObject::tr("Blue (0-255)")
#define TEXT_ASCII_BLUE_FLOAT QObject::tr("Blue (0.-1)")
#define TEXT_ASCII_INTENSITY QObject::tr("Intensity")
#define TEXT_ASCII_IGNORE QObject::tr("Ignore")

#endif