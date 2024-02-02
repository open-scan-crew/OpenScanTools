#ifndef NEW_PROJECT_DIALOG_H
#define NEW_PROJECT_DIALOG_H

#include <filesystem>

#include <QtWidgets/QDialog>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTextEdit>

#include "models/project/ProjectInfo.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

class NewProjectDialog : public QDialog, public IPanel
{
    Q_OBJECT

public:
    NewProjectDialog(IDataDispatcher &dataDispatcher, QString folderPath, QWidget *parent);
    ~NewProjectDialog();

private:
    void informData(IGuiData *keyValue);
    QString getName() const override;
    void onCancel();

    void createProject();
    void launchFileBrowser();

private:
    IDataDispatcher &m_dataDispatcher;

    QLineEdit* m_projectName;
    QLineEdit* m_projectDirPath;
    QLineEdit* m_companyName;
    QLineEdit* m_location;
    QTextEdit* m_description;
};

#endif