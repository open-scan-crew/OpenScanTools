#ifndef FILE_SYSTEM_TEXTS_H
#define FILE_SYSTEM_TEXTS_H

#include <QtCore/qobject.h>

#define TEXT_SELECT_DIRECTORY  QObject::tr("Select folder") //directory renommé en folder, plus couramment utilisé
#define TEXT_NO_DIRECTORY_SELECTED QObject::tr("Please select a destination folder.")

#define TEXT_SAVE_FILENAME QObject::tr("Select destination")
#define TEXT_ERROR_FILENAME QObject::tr("Filename is incorrect")
#define TEXT_ERROR_OPEN_FILE QObject::tr("Error : The file :\n '%1'\ncannot be opened.")
#define TEXT_ERROR_EMPTY_FILE QObject::tr("Error : The file :\n '%1'\nis empty.")


#define TEXT_MISSING_FILE_NAME QObject::tr("Please enter a file name or header name.")
#define TEXT_SELECT_CUSTOM_SCAN_FOLDER  QObject::tr("Select custom scan folder")

#define TEXT_DIALOG_BROWSER_TITLE_IMPORT QObject::tr("Select input file")
#define TEXT_DIALOG_BROWSER_TITLE_EXPORT QObject::tr("Select output file")
#define TEXT_DIALOG_ERROR_INPUT_PATH QObject::tr("Input file doesn't exist")
#define TEXT_DIALOG_ERROR_OUTPUT_PATH QObject::tr("Empty output file")
#define TEXT_DIALOG_ERROR_OUTPUT_FOLDER QObject::tr("Empty output folder")

//#define TEXT_DIALOG_ERROR_FORMAT QObject::tr("File format not supported")

#endif