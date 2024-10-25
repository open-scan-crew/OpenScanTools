#ifndef DIALOG_IMPORT_PCOBJECT_H
#define DIALOG_IMPORT_PCOBJECT_H

#include "gui/Dialog/ADialog.h"
#include "ui_DialogImportPCObject.h"

#include <glm/glm.hpp>
#include <filesystem>

class DialogImportPCObject : public ADialog
{
    Q_OBJECT

public:
    DialogImportPCObject(IDataDispatcher& dataDispatcher, QWidget *parent);
    ~DialogImportPCObject();

    void informData(IGuiData *data) override;
    void closeEvent(QCloseEvent* close) override;

private:

    void onFileBrowser();
    void onOk();
	void onPositionEdit();
    void onCancel();

    void onProjectPath(IGuiData* data);

private:
    Ui::DialogImportPCObject		                m_ui;
	glm::vec3						                m_storedPosition;
    std::vector<std::filesystem::path>              m_storedPaths;
    QString							                m_openPath;

};

#endif
