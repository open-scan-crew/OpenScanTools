#ifndef DIALOG_EXPORT_SUBPROJECT_H
#define DIALOG_EXPORT_SUBPROJECT_H

#include "ui_DialogExportSubProject.h"
#include "gui/Dialog/ADialog.h"

#include "gui/Dialog/DialogSubProjectInfo.h"


class ProjectInfos;

class PropertiesProjectPanel;

class DialogExportSubProject : public ADialog
{
    Q_OBJECT

public:
    DialogExportSubProject(IDataDispatcher& dataDispatcher, QWidget *parent, float guiScale);
    ~DialogExportSubProject();

    void informData(IGuiData *data) override;
    void closeEvent(QCloseEvent* event);

public:
    void onSelectOutFolder();
    void onSubProjectInfo();

    void startExport();
    void cancelExport();

private:
    void refreshUI();

private:
    Ui::DialogExportSubProject m_ui;
    QString m_openPath;

	std::filesystem::path m_exportFolder;

    DialogSubProjectInfo* m_subProjectInfoDialog;
    ProjectInfos m_storedProjectInfo;

};

#endif