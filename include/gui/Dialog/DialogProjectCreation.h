#ifndef DIALOG_PROJECT_CREATION_H
#define DIALOG_PROJECT_CREATION_H

#include "ui_DialogProjectCreation.h"
#include "gui/Dialog/ADialog.h"

#include <filesystem>

class DialogProjectCreation : public ADialog
{
    Q_OBJECT

 public:
    DialogProjectCreation(IDataDispatcher& dataDispatcher, QWidget* parent);
    ~DialogProjectCreation();

    void setDefaultValues(const std::filesystem::path& folderPath, std::wstring default_name, std::wstring default_company);
    void setAdditionalTemplatesPath(const std::vector<std::filesystem::path>& templates_path);

private:
    void informData(IGuiData* keyValue);
    void onCancel();
    void createProject();
    void launchFileBrowser();
    void launchFileBrowserCustomScanFolder();

private:
    Ui::DialogProjectCreation m_ui;

    QString m_openPath;
};

#endif //!DIALOG_PROJECT_CREATION_H