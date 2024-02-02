#ifndef DIALOG_SETTINGS_H
#define DIALOG_SETTINGS_H

#include "ui_DialogSettings.h"
#include "gui/Translator.h"
#include "gui/Dialog/ADialog.h"
#include "gui/Dialog/DialogGizmo.h"
#include "utils/Color32.hpp"
#include "pointCloudEngine/RenderingTypes.h"
#include "gui/UnitConverter.h"

class DialogSettings : public ADialog
{
    Q_OBJECT

public:
    DialogSettings(IDataDispatcher& dataDispatcher, Translator* translator, QWidget* parent = Q_NULLPTR);
    ~DialogSettings();

    // from IPanel
    void informData(IGuiData* idata) override;

    void initComboBox(QComboBox* comboBox, int currentData);
    void initConfigValues();
    void updateUnitValues();

    void blockSignal(bool active);

    void showEvent(QShowEvent* event) override;
    QCheckBox* getFramelessCheckBox();

public slots:
    void onLangageChanged(const uint32_t& index);
    void onSelectTempFolder();
    void onSelectProjsFolder();
    void onTempFolder();
    void onProjsFolder();
    void onUnitUsageChanged();
    void onColorPicking();
    void onOk();
    void onCancel();
    void onDecimationChanged();
    void onExamineOptions();
    void onFramelessChanged();
    void onExamineDisplayChanged();
    void onAutosaveCheckboxChanged();
    void onAutosaveComboxBoxChanged();
    void onIndexationMethodChoice();
    void onManipulatorSizeChanged(int manipulatorSize);

    void onNavigationParametersChanged();
    void onRenderPerspDistanceChanged();
    void onRenderOrthoDistanceChanged();

    void onUnlockScansManipulation();

private:
    void showDecimationOptions();
    void sendDecimationOptions();

private:
    Ui::DialogSettings      m_ui;
    Translator*             m_translator;
    LangageType             m_type;
    DecimationOptions       m_savedDecimationOptions;
    DialogGizmo            m_guizmoParams;
 };
#endif