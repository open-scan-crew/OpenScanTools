#ifndef DIALOG_EXPORT_PRIMITIVES_H
#define DIALOG_EXPORT_PRIMITIVES_H

#include "ui_DialogExportPrimitives.h"
#include "gui/Dialog/ADialog.h"
#include "io/exports/ExportParameters.hpp"

#include <unordered_set>

#include "models/Types.hpp"

class DialogExportPrimitives : public ADialog
{
    Q_OBJECT

public:
	DialogExportPrimitives(IDataDispatcher& dataDispatcher, QWidget *parent, float guiScale);
    ~DialogExportPrimitives();


	void setFormat(ObjectExportType format);
    void informData(IGuiData *data) override;
    void closeEvent(QCloseEvent* event);

public:
    void onSelectOutFolder();
	void onSelectOutFile();

    void startExport();
    void cancelExport();

private:
    void refreshUI();
    std::unordered_set<ElementType> getSelectedTypes();

private:
    Ui::DialogExportPrimitive m_ui;
    QString m_openPath;

	ObjectExportType m_format;
	std::filesystem::path m_exportFolder;
	std::wstring m_filename;

	PrimitivesExportParameters m_parameters;

};

#endif