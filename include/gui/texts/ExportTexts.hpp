#ifndef EXPORT_TEXTS_HPP
#define EXPORT_TEXTS_HPP

#include <QtCore/qobject.h>

#define TEXT_EXPORT_SAVE_QUESTION QObject::tr("It is recommended to save the project before exporting. Do you want to save now?")

#define TEXT_EXPORT_METHOD_SCAN_SEPARATED QObject::tr("One scan per file")
#define TEXT_EXPORT_METHOD_CLIPPING_SEPARATED QObject::tr("One file per clipping (only interior mode)")
#define TEXT_EXPORT_METHOD_SCAN_MERGED QObject::tr("Merged")
#define TEXT_EXPORT_CLIPPING_SOURCE_NONE QObject::tr("No Clipping")
#define TEXT_EXPORT_CLIPPING_SOURCE_SELECTED QObject::tr("Selected")
#define TEXT_EXPORT_CLIPPING_SOURCE_ACTIVE QObject::tr("Active")
#define TEXT_EXPORT_LABEL_MISSING  QObject::tr("<Label Missing>")
#define TEXT_EXPORT_TITLE_NORMAL QObject::tr("Scans Export")
#define TEXT_EXPORT_TITLE_PCOS QObject::tr("Point Cloud Objects Export");
#define TEXT_EXPORT_TITLE_CLIPPING QObject::tr("Clipping Export")
#define TEXT_EXPORT_TITLE_GRID QObject::tr("Grids Export")
#define TEXT_EXPORT_TITLE_DELETE_POINTS QObject::tr("Delete Scan Points")
#define TEXT_EXPORT_ERROR QObject::tr("Something wrong happened...")
#define TEXT_EXPORT_ERROR_FILE  QObject::tr("Failed to create destination file: %1")
#define TEXT_EXPORT_SUCCESS_TIME  QObject::tr("\n\nScan export done in: %1 seconds")
#define TEXT_EXPORT_CLIPPING_SUCCESS_TIME  QObject::tr("\n\nClipping export done in: %1 seconds")
#define TEXT_EXPORT_ERROR_TIME  QObject::tr("\n\nScan export failed in: %1 seconds")
#define TEXT_EXPORT_CLIPPING_ERROR_TIME  QObject::tr("\n\nClipping export failed in: %1 seconds")
#define TEXT_EXPORT_NO_USABLE_CLIPPING_OBJECTS QObject::tr("You must activate or select some clippings first")
#define TEXT_EXPORT_TITLE_GRID QObject::tr("Grids Export")
#define TEXT_EXPORT_GRID_SELECT_FIRST QObject::tr("Select grid(s) first")
#define TEXT_EXPORT_NO_SCAN_SELECTED QObject::tr("No visible scan to export")
#define TEXT_EXPORT_CLIPPING_TITLE_PROGESS QObject::tr("Export Point Cloud in clippings")
#define TEXT_EXPORT_GRID_TITLE_PROGESS QObject::tr("Export Point Cloud in gridded boxes")
#define TEXT_EXPORT_DIALOG_NAME QObject::tr("Dialog export point cloud")
#define TEXT_EXPORT_INVALID_DIRECTORY QObject::tr("The selected directory is not valid for exports (invalid path OR no write permission)")

#define TEXT_EXPORT_FILTER_ALL QObject::tr("All")
#define TEXT_EXPORT_FILTER_SELECTED QObject::tr("Selected")
#define TEXT_EXPORT_FILTER_DISPLAYED QObject::tr("Displayed")
#define TEXT_EXPORT_FILTER_NONE QObject::tr("None")

#define TEXT_EXPORT_GENERAL_FILE QObject::tr("Export %1 file(s)")
#define TEXT_EXPORT_OPENSCANTOOLS_FILE QObject::tr("Export shared files")

#endif