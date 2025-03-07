#ifndef DIALOG_OPEN_PROJECT_CENTRAL_H
#define DIALOG_OPEN_PROJECT_CENTRAL_H

#include "ui_DialogOpenProjectCentral.h"
#include "gui/Dialog/ADialog.h"

#include <filesystem>

class DialogOpenProjectCentral : public ADialog
{
    Q_OBJECT

 public:
     DialogOpenProjectCentral(IDataDispatcher& dataDispatcher, QWidget* parent);
    ~DialogOpenProjectCentral();

public:
    void setCentralProjectPath(const std::filesystem::path& centralPath);
    void onCancel();
    void openProjectCentral();
    void openProjectLocal();
    void informData(IGuiData* keyValue);

private:
    Ui::DialogOpenProjectCentral m_ui;
    QString m_openPath;
    std::filesystem::path m_centralPath;
};

#endif //!DIALOG_OPEN_PROJECT_CENTRAL_H