#ifndef DIALOG_IMPORT_WAVEFRONT_H
#define DIALOG_IMPORT_WAVEFRONT_H

#include "gui/Dialog/ADialog.h"
#include "ui_DialogImportMeshObject.h"

#include "io/FileUtils.h"
#include "gui/Dialog/DialogImportStepSimplification.h"

#include <glm/glm.hpp>
#include <set>

enum class Selection;

class DialogImportMeshObject : public ADialog
{
    Q_OBJECT

public:
    DialogImportMeshObject(IDataDispatcher& dataDispatcher, QWidget *parent);
    ~DialogImportMeshObject();

    void informData(IGuiData *data) override;
    void closeEvent(QCloseEvent* close) override;

private:
    void resetCoordinates();

    void onFileBrowser();
    void onOk();
	void onPositionEdit();
    void onCancel();
    void onComboBoxChanged(int comboBox, int index);
    void onOverrideScaleUpdate();
    void onTruncateCoorOnImport();

	void initComboBoxes();

    void onProjectPath(IGuiData* data);
    void setTruncCoor(const glm::dvec3& truncCoor);

    bool checkDirs(const Selection& up, const Selection& fw) const;
    std::set<Selection> getIncompatibleAxes(const Selection& dir) const;

    void updateFileType();

private:
    Ui::DialogImportMeshObject		m_ui;
	glm::vec3						m_storedPosition;
    std::filesystem::path           m_storedPath;
    FileType                        m_storedExtension;
    QString							m_openPath;
	DialogImportStepSimplification	m_dialogStepSimplification;
    glm::dvec3                      m_storedScanTruncateCoor;

    Selection                       m_forward;
    Selection                       m_up;
};

#endif
