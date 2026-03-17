#include "gui/toolBars/ToolBarRenderTransparency.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/UITransparencyConverter.h"

#include "models/graph/CameraNode.h"

ToolBarRenderTransparency::ToolBarRenderTransparency(IDataDispatcher& dataDispatcher, QWidget* parent, float guiScale)
    : QWidget(parent)
    , m_dataDispatcher(dataDispatcher)
    , m_focusCamera()
{
    m_ui.setupUi(this);
    setEnabled(false);

    m_ui.slider_transparency->setMinimumWidth(100.f * guiScale);
    m_ui.slider_flashControl->setMinimumWidth(100.f * guiScale);

    connect(m_ui.slider_transparency, &QSlider::valueChanged, m_ui.spinBox_transparency, &QSpinBox::setValue);
    connect(m_ui.spinBox_transparency, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.slider_transparency, &QSlider::setValue);
    connect(m_ui.slider_transparency, &QSlider::valueChanged, this, &ToolBarRenderTransparency::slotTransparencyValueChanged);

    connect(m_ui.checkBox_transparency, &QCheckBox::stateChanged, this, &ToolBarRenderTransparency::slotTranparencyActivationChanged);

    connect(m_ui.checkBox_enhanceContrast, &QCheckBox::stateChanged, this, &ToolBarRenderTransparency::slotTransparencyOptionsChanged);
    connect(m_ui.checkBox_negativeEffect, &QCheckBox::stateChanged, this, &ToolBarRenderTransparency::slotTransparencyOptionsChanged);
    connect(m_ui.basicEnhanceContrastRadioButton, &QRadioButton::toggled, this, &ToolBarRenderTransparency::slotTransparencyOptionsChanged);
    connect(m_ui.advEnhanceContrastRadioButton, &QRadioButton::toggled, this, &ToolBarRenderTransparency::slotTransparencyOptionsChanged);
    connect(m_ui.checkBox_adaptiveTransparency, &QCheckBox::stateChanged, this, &ToolBarRenderTransparency::slotAdaptiveTransparencyChanged);
    connect(m_ui.pushButton_adaptiveTransparencySettings, &QPushButton::clicked, this, &ToolBarRenderTransparency::slotAdaptiveTransparencySettings);
    connect(m_ui.slider_flashControl, &QSlider::valueChanged, m_ui.spinBox_flashControl, &QSpinBox::setValue);
    connect(m_ui.spinBox_flashControl, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.slider_flashControl, &QSlider::setValue);
    connect(m_ui.slider_flashControl, &QSlider::valueChanged, this, &ToolBarRenderTransparency::slotTransparencyOptionsChanged);

    registerGuiDataFunction(guiDType::projectLoaded, &ToolBarRenderTransparency::onProjectLoad);
    registerGuiDataFunction(guiDType::renderActiveCamera, &ToolBarRenderTransparency::onActiveCamera);
    registerGuiDataFunction(guiDType::focusViewport, &ToolBarRenderTransparency::onFocusViewport);
}

ToolBarRenderTransparency::~ToolBarRenderTransparency()
{
    m_dataDispatcher.unregisterObserver(this);
}

void ToolBarRenderTransparency::informData(IGuiData* data)
{
    if (m_methods.find(data->getType()) != m_methods.end())
    {
        GuiDataFunction method = m_methods.at(data->getType());
        (this->*method)(data);
    }
}

void ToolBarRenderTransparency::onProjectLoad(IGuiData* idata)
{
    GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(idata);
    setEnabled(plData->m_isProjectLoad);
}

void ToolBarRenderTransparency::onFocusViewport(IGuiData* data)
{
    GuiDataFocusViewport* castData = static_cast<GuiDataFocusViewport*>(data);
    if (castData->m_forceFocus && castData->m_camera)
        m_focusCamera = castData->m_camera;
}

void ToolBarRenderTransparency::onActiveCamera(IGuiData* idata)
{
	auto infos = static_cast<GuiDataCameraInfo*>(idata);

    if (infos->m_camera && m_focusCamera != infos->m_camera)
        return;

    ReadPtr<CameraNode> rCam = m_focusCamera.cget();
    if (!rCam)
        return;
	const DisplayParameters& displayParameters = rCam->getDisplayParameters();

	blockAllSignals(true);

    bool transparencyActive = displayParameters.m_blendMode == BlendMode::Transparent;
	m_ui.checkBox_transparency->setChecked(transparencyActive);
    float uiTransparency = ui::transparency::trueValue_to_uiValue(displayParameters.m_transparency);
	m_ui.spinBox_transparency->setValue(uiTransparency);
	m_ui.slider_transparency->setValue(uiTransparency);

    m_ui.checkBox_negativeEffect->setChecked(displayParameters.m_negativeEffect);
    m_ui.checkBox_enhanceContrast->setChecked(displayParameters.m_reduceFlash);
    m_ui.basicEnhanceContrastRadioButton->setChecked(!displayParameters.m_flashAdvanced);
    m_ui.advEnhanceContrastRadioButton->setChecked(displayParameters.m_flashAdvanced);
    m_ui.spinBox_flashControl->setValue(static_cast<int>(displayParameters.m_flashControl));
    m_ui.slider_flashControl->setValue(static_cast<int>(displayParameters.m_flashControl));

    m_ui.checkBox_adaptiveTransparency->setChecked(displayParameters.m_adaptiveTransparency);
    m_adaptiveSettings.transparencyNear = displayParameters.m_adaptiveTransparencyNear;
    m_adaptiveSettings.transparencyFar = displayParameters.m_adaptiveTransparencyFar;
    m_adaptiveSettings.distanceNear = displayParameters.m_adaptiveTransparencyDistNear;
    m_adaptiveSettings.distanceFar = displayParameters.m_adaptiveTransparencyDistFar;
    m_distanceUnit = displayParameters.m_unitUsage.distanceUnit;

    blockAllSignals(false);
    enableUI(transparencyActive);
}


void ToolBarRenderTransparency::blockAllSignals(bool block)
{
    m_ui.spinBox_transparency->blockSignals(block);
    m_ui.slider_transparency->blockSignals(block);
    m_ui.checkBox_transparency->blockSignals(block);
    m_ui.checkBox_negativeEffect->blockSignals(block);
    m_ui.checkBox_enhanceContrast->blockSignals(block);
    m_ui.basicEnhanceContrastRadioButton->blockSignals(block);
    m_ui.advEnhanceContrastRadioButton->blockSignals(block);
    m_ui.checkBox_adaptiveTransparency->blockSignals(block);
    m_ui.slider_flashControl->blockSignals(block);
    m_ui.spinBox_flashControl->blockSignals(block);
}

void ToolBarRenderTransparency::enableUI(bool transparencyActive)
{
    m_ui.checkBox_adaptiveTransparency->setEnabled(transparencyActive);
    m_ui.pushButton_adaptiveTransparencySettings->setEnabled(transparencyActive && m_ui.checkBox_adaptiveTransparency->isChecked());
    m_ui.spinBox_transparency->setEnabled(transparencyActive && !m_ui.checkBox_adaptiveTransparency->isChecked());
    m_ui.slider_transparency->setEnabled(transparencyActive && !m_ui.checkBox_adaptiveTransparency->isChecked());
    m_ui.checkBox_enhanceContrast->setEnabled(transparencyActive);
    m_ui.checkBox_negativeEffect->setEnabled(transparencyActive);
    updateFlashControlState();
    updateAdaptiveControlsState();
}

void ToolBarRenderTransparency::sendTransparency()
{
    int uiTransparency = m_ui.slider_transparency->value();
    float t = ui::transparency::uiValue_to_trueValue(uiTransparency);
    BlendMode blend = m_ui.checkBox_transparency->isChecked() ? BlendMode::Transparent : BlendMode::Opaque;
    m_dataDispatcher.updateInformation(new GuiDataRenderTransparency(blend, t, m_focusCamera), this);
}

void ToolBarRenderTransparency::sendTransparencyOptions()
{
    bool advanced = m_ui.advEnhanceContrastRadioButton->isChecked();
    float flashControl = static_cast<float>(m_ui.slider_flashControl->value());
    m_dataDispatcher.updateInformation(new GuiDataRenderTransparencyOptions(
        m_ui.checkBox_negativeEffect->isChecked(),
        m_ui.checkBox_enhanceContrast->isChecked(),
        advanced,
        flashControl,
        m_ui.checkBox_adaptiveTransparency->isChecked(),
        m_adaptiveSettings.transparencyNear,
        m_adaptiveSettings.transparencyFar,
        m_adaptiveSettings.distanceNear,
        m_adaptiveSettings.distanceFar,
        0.f,
        m_focusCamera), this);
}

void ToolBarRenderTransparency::slotTranparencyActivationChanged(int value)
{
    enableUI(value > 0);
    sendTransparency();
    sendTransparencyOptions();
}

void ToolBarRenderTransparency::slotTransparencyValueChanged(int value)
{
    sendTransparency();
}

void ToolBarRenderTransparency::slotTransparencyOptionsChanged(int value)
{
    updateFlashControlState();
    sendTransparencyOptions();
}

void ToolBarRenderTransparency::slotAdaptiveTransparencyChanged(int value)
{
    updateAdaptiveControlsState();
    sendTransparencyOptions();
}

void ToolBarRenderTransparency::slotAdaptiveTransparencySettings()
{
    DialogAdaptiveTransparencySettings dialog(m_adaptiveSettings, m_distanceUnit, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    m_adaptiveSettings = dialog.getSettings();
    sendTransparencyOptions();
}

void ToolBarRenderTransparency::updateFlashControlState()
{
    bool transparencyActive = m_ui.checkBox_transparency->isChecked();
    bool enhance = transparencyActive && m_ui.checkBox_enhanceContrast->isChecked();
    m_ui.basicEnhanceContrastRadioButton->setEnabled(enhance);
    m_ui.advEnhanceContrastRadioButton->setEnabled(enhance);

    bool advanced = enhance && m_ui.advEnhanceContrastRadioButton->isChecked();
    m_ui.slider_flashControl->setEnabled(advanced);
    m_ui.spinBox_flashControl->setEnabled(advanced);
    m_ui.label_flashControl->setEnabled(advanced);
}

void ToolBarRenderTransparency::updateAdaptiveControlsState()
{
    const bool transparencyActive = m_ui.checkBox_transparency->isChecked();
    const bool adaptive = transparencyActive && m_ui.checkBox_adaptiveTransparency->isChecked();
    m_ui.slider_transparency->setEnabled(transparencyActive && !adaptive);
    m_ui.spinBox_transparency->setEnabled(transparencyActive && !adaptive);
    m_ui.pushButton_adaptiveTransparencySettings->setEnabled(adaptive);
}
