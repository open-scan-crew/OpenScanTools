#ifndef DIALOG_GIZMO_H
#define DIALOG_GIZMO_H

#include "ui_DialogGizmo.h"
#include "gui/Dialog/ADialog.h"

#include <glm/glm.hpp>

class DialogGizmo : public ADialog
{
    Q_OBJECT

public:
    DialogGizmo(IDataDispatcher& dataDispatcher, QWidget* parent = Q_NULLPTR);
    ~DialogGizmo();

    // from IPanel
    void informData(IGuiData* idata) override;

    void initConfigValues();

    void accept() override;
    void reject() override;

public slots:
    void onSizeDoubleSpin(const double& value);
    void onHorizontalSlider(const int& value);
    void onVerticalSlider(const int& value);

private:
    void sendUpdate();

private:
    Ui::GizmoDialog        m_ui;
    glm::vec3              m_parameters;
    glm::vec3              m_oldParameters;
};
#endif