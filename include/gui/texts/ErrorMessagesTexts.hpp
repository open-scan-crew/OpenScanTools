#ifndef ERROR_MESSAGES_TEXTS_HPP
#define ERROR_MESSAGES_TEXTS_HPP

#include <QtCore/qobject.h>

#define TEXT_ERROR QObject::tr("Error")

//ControlSignal
#define TEXT_APPLICATION_CRASHED QObject::tr("Error : the application crashed. Please send us the log file for support. You can find the log file in C:\\Documents\\OpenScanTools.")

#define TEXT_WRITE_FAILED_PERMISSION QObject::tr("Error : the file could not be created. It is possible that OpenScanTools does not have the write permission.")

#define TEXT_EXPORT_FILES_EXPORTED_TO QObject::tr("File(s) exported to %1")

#endif