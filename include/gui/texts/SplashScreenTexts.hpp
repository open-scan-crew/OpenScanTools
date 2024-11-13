#ifndef SPLASH_SCREEN_TEXTS_HPP
#define SPLASH_SCREEN_TEXTS_HPP

#include <QObject>

// ProcessingSplashScreen
#define TEXT_SPLASH_SCREEN_SCAN_PROCESSING QObject::tr("Scan done: %1/%2")
#define TEXT_SPLASH_SCREEN_CLIPPING_PROCESSING QObject::tr("Clipping done: %1/%2")
#define TEXT_SPLASH_SCREEN_PIXEL_PROCESSING QObject::tr("Pixel done: %1/%2")
#define TEXT_SPLASH_SCREEN_FILE_PROCESSING QObject::tr("File done: %1/%2")
#define TEXT_SPLASH_SCREEN_BOX_PROCESSING QObject::tr("Box done: %1/%2")
#define TEXT_SPLASH_SCREEN_IMPORT_PROCESSING_LOG_ERROR QObject::tr("Import error: %1")
#define TEXT_SPLASH_SCREEN_SIMPLIFY_STEP_DATA_TITLE QObject::tr("Step Simplification")
#define TEXT_SPLASH_SCREEN_SIMPLIFY_STEP_DATA_PROCESSING QObject::tr("Simplify step data, please wait...")

#define TEXT_SPLASH_SCREEN_IMPORT_GEOMETRIES QObject::tr("Geometries done: %1/%2")
#define TEXT_SPLASH_SCREEN_IMPORT_LINES QObject::tr("Lines done: %1/%2")
#define TEXT_SPLASH_SCREEN_IMPORT_PROCESSING QObject::tr("Mesh(es) Allocation, please wait")
#define TEXT_SPLASH_SCREEN_WRITE_INTERN_FILE_PROCESSING QObject::tr("Write intern(s) file(s), please wait")
#define TEXT_SPLASH_SCREEN_WRITE_INTERN_FILE_ERROR QObject::tr("Intern file writing error: %1")
#define TEXT_SPLASH_SCREEN_WRITE_INTERN_FILE_RESULT QObject::tr("%1 files written to %2")

#define TEXT_SPLASH_SCREEN_IMPORT QObject::tr("Import...")
#define TEXT_SPLASH_SCREEN_IMPORT_EXTERNAL_DATA_TITLE_WITH_NAME QObject::tr("Import %1")
#define TEXT_SPLASH_SCREEN_DONE QObject::tr("Processing done")
#define TEXT_SPLASH_SCREEN_ABORT QObject::tr("Processing abort")

// MessageSplashScreen
#define TEXT_MESSAGE_SPLASH_SCREEN_NAME QObject::tr("Information")
#define TEXT_MESSAGE_SPLASH_SCREEN_READING_DATA QObject::tr("Reading data, please wait...")
#define TEXT_MESSAGE_SPLASH_SCREEN_READING_DATA_WITH_NAME QObject::tr("Reading %1, please wait...")
#define TEXT_MESSAGE_SPLASH_SCREEN_IMPORTING_DATA_WITH_NAME QObject::tr("Importing %1, please wait...")

#endif