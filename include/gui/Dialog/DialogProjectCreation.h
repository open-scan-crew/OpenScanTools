#ifndef DIALOG_PROJECT_CREATION_H
#define DIALOG_PROJECT_CREATION_H

#include "ui_DialogProjectCreation.h"
#include "models/project/ProjectTypes.h"
#include "gui/Dialog/ADialog.h"

#include <unordered_map>

enum LangageType;

class DialogProjectCreation : public ADialog
{
    Q_OBJECT

 public:
    DialogProjectCreation(IDataDispatcher& dataDispatcher, QString folderPath, const std::vector<std::filesystem::path>& templates, const std::unordered_map<LangageType, ProjectTemplate>& projectTemplates, QWidget* parent);
    ~DialogProjectCreation();

private:
    void informData(IGuiData* keyValue);
    void onCancel();
    void createProject();
    void launchFileBrowser();
    void launchFileBrowserCustomScanFolder();

private:
    Ui::DialogProjectCreation m_ui;
    std::unordered_map<LangageType, ProjectTemplate> m_projectTemplates;

    QString m_openPath;
};

#endif //!DIALOG_PROJECT_CREATION_H