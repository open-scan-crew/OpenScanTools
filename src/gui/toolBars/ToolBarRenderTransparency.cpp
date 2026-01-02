#include "gui/toolBars/ToolBarRenderTransparency.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/UITransparencyConverter.h"

#include "models/graph/CameraNode.h"

#include <algorithm>
#include <cmath>

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
    connect(m_ui.basicEnhanceContrastRadioButton, &QRadioButton::toggled, this, &ToolBarRenderTransparency::slotEnhanceContrastModeChanged);
    connect(m_ui.expEnhanceContrastRadioButton, &QRadioButton::toggled, this, &ToolBarRenderTransparency::slotEnhanceContrastModeChanged);
    connect(m_ui.slider_flashControl, &QSlider::valueChanged, m_ui.spinBox_flashControl, &QSpinBox::setValue);
    connect(m_ui.spinBox_flashControl, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.slider_flashControl, &QSlider::setValue);
    connect(m_ui.slider_flashControl, &QSlider::valueChanged, this, &ToolBarRenderTransparency::slotFlashControlChanged);

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
    m_ui.checkBox_enhanceContrast->setChecked(displayParameters.m_reduceFlash);
    m_ui.basicEnhanceContrastRadioButton->setChecked(displayParameters.m_enhanceContrastMode != EnhanceContrastMode::Exponential);
    m_ui.expEnhanceContrastRadioButton->setChecked(displayParameters.m_enhanceContrastMode == EnhanceContrastMode::Exponential);
    int flashControlUi = std::clamp(static_cast<int>(std::round(displayParameters.m_flashControl)), 0, 100);
    m_ui.spinBox_flashControl->setValue(flashControlUi);
    m_ui.slider_flashControl->setValue(flashControlUi);
    enableUI(transparencyActive);

	m_ui.checkBox_negativeEffect->setChecked(displayParameters.m_negativeEffect);

	blockAllSignals(false);
}


void ToolBarRenderTransparency::blockAllSignals(bool block)
{
    m_ui.spinBox_transparency->blockSignals(block);
    m_ui.slider_transparency->blockSignals(block);
    m_ui.checkBox_transparency->blockSignals(block);
    m_ui.checkBox_negativeEffect->blockSignals(block);
    m_ui.checkBox_enhanceContrast->blockSignals(block);
    m_ui.basicEnhanceContrastRadioButton->blockSignals(block);
    m_ui.expEnhanceContrastRadioButton->blockSignals(block);
    m_ui.slider_flashControl->blockSignals(block);
    m_ui.spinBox_flashControl->blockSignals(block);
}

void ToolBarRenderTransparency::enableUI(bool transparencyActive)
{
    m_ui.spinBox_transparency->setEnabled(transparencyActive);
    m_ui.slider_transparency->setEnabled(transparencyActive);

    m_ui.checkBox_enhanceContrast->setEnabled(transparencyActive);
    m_ui.checkBox_negativeEffect->setEnabled(transparencyActive);

    bool enhanceActive = transparencyActive && m_ui.checkBox_enhanceContrast->isChecked();
    m_ui.basicEnhanceContrastRadioButton->setEnabled(enhanceActive);
    m_ui.expEnhanceContrastRadioButton->setEnabled(enhanceActive);

    bool exponentialMode = enhanceActive && m_ui.expEnhanceContrastRadioButton->isChecked();
    m_ui.slider_flashControl->setEnabled(exponentialMode);
    m_ui.spinBox_flashControl->setEnabled(exponentialMode);
    m_ui.label_flashControl->setEnabled(exponentialMode);
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
    EnhanceContrastMode mode = m_ui.expEnhanceContrastRadioButton->isChecked() ? EnhanceContrastMode::Exponential : EnhanceContrastMode::Basic;
    m_dataDispatcher.updateInformation(new GuiDataRenderTransparencyOptions(m_ui.checkBox_negativeEffect->isChecked(),
                                                                           m_ui.checkBox_enhanceContrast->isChecked(),
                                                                           mode,
                                                                           static_cast<float>(m_ui.slider_flashControl->value()),
                                                                           0.f,
                                                                           m_focusCamera));
}

void ToolBarRenderTransparency::slotTranparencyActivationChanged(int value)
{
    enableUI(value > 0);
    sendTransparency();
}

void ToolBarRenderTransparency::slotTransparencyValueChanged(int value)
{
    sendTransparency();
}

void ToolBarRenderTransparency::slotTransparencyOptionsChanged(int value)
{
    Q_UNUSED(value);
    enableUI(m_ui.checkBox_transparency->isChecked());
    sendTransparencyOptions();
}

void ToolBarRenderTransparency::slotEnhanceContrastModeChanged(bool)
{
    enableUI(m_ui.checkBox_transparency->isChecked());
    sendTransparencyOptions();
}

void ToolBarRenderTransparency::slotFlashControlChanged(int)
{
    sendTransparencyOptions();
}
