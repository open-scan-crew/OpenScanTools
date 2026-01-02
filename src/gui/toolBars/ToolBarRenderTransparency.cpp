#include "gui/toolBars/ToolBarRenderTransparency.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/UITransparencyConverter.h"

#include "models/graph/CameraNode.h"

#include <algorithm>
#include <cmath>

namespace
{
    constexpr float k_flashExposureMin = 0.2f;
    constexpr float k_flashExposureMax = 5.0f;

    inline float slider_to_exposure(int sliderValue)
    {
        float clamped = std::clamp(sliderValue, 0, 100) / 100.f;
        float range = std::log(k_flashExposureMax / k_flashExposureMin);
        return k_flashExposureMin * std::exp(clamped * range);
    }

    inline int exposure_to_slider(float exposure)
    {
        float clamped = std::clamp(exposure, k_flashExposureMin, k_flashExposureMax);
        float range = std::log(k_flashExposureMax / k_flashExposureMin);
        float ratio = std::log(clamped / k_flashExposureMin) / range;
        return static_cast<int>(std::round(ratio * 100.f));
    }
}

ToolBarRenderTransparency::ToolBarRenderTransparency(IDataDispatcher& dataDispatcher, QWidget* parent, float guiScale)
    : QWidget(parent)
    , m_dataDispatcher(dataDispatcher)
    , m_focusCamera()
{
    m_ui.setupUi(this);
    setEnabled(false);

    m_ui.slider_transparency->setMinimumWidth(100.f * guiScale);

    connect(m_ui.slider_transparency, &QSlider::valueChanged, m_ui.spinBox_transparency, &QSpinBox::setValue);
    connect(m_ui.spinBox_transparency, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.slider_transparency, &QSlider::setValue);
    connect(m_ui.slider_transparency, &QSlider::valueChanged, this, &ToolBarRenderTransparency::slotTransparencyValueChanged);

    connect(m_ui.checkBox_transparency, &QCheckBox::stateChanged, this, &ToolBarRenderTransparency::slotTranparencyActivationChanged);

    connect(m_ui.checkBox_enhanceContrast, &QCheckBox::stateChanged, this, &ToolBarRenderTransparency::slotTransparencyOptionsChanged);
    connect(m_ui.checkBox_negativeEffect, &QCheckBox::stateChanged, this, &ToolBarRenderTransparency::slotTransparencyOptionsChanged);
    connect(m_ui.slider_flashControl, &QSlider::valueChanged, this, &ToolBarRenderTransparency::slotTransparencyOptionsChanged);
    connect(m_ui.reinhardRadioButton, &QRadioButton::toggled, this, &ToolBarRenderTransparency::slotTransparencyOptionsChanged);
    connect(m_ui.exponentialRadioButton, &QRadioButton::toggled, this, &ToolBarRenderTransparency::slotTransparencyOptionsChanged);

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
    int flashSlider = exposure_to_slider(displayParameters.m_flashExposure);
    m_ui.slider_flashControl->setValue(flashSlider);
    if (displayParameters.m_highlightCompressionMode == HighlightCompressionMode::Exponential)
        m_ui.exponentialRadioButton->setChecked(true);
    else
        m_ui.reinhardRadioButton->setChecked(true);

    enableUI(transparencyActive);

	blockAllSignals(false);
}


void ToolBarRenderTransparency::blockAllSignals(bool block)
{
    m_ui.spinBox_transparency->blockSignals(block);
    m_ui.slider_transparency->blockSignals(block);
    m_ui.checkBox_transparency->blockSignals(block);
    m_ui.checkBox_negativeEffect->blockSignals(block);
    m_ui.checkBox_enhanceContrast->blockSignals(block);
    m_ui.slider_flashControl->blockSignals(block);
    m_ui.reinhardRadioButton->blockSignals(block);
    m_ui.exponentialRadioButton->blockSignals(block);
}

void ToolBarRenderTransparency::enableUI(bool transparencyActive)
{
    m_ui.spinBox_transparency->setEnabled(transparencyActive);
    m_ui.slider_transparency->setEnabled(transparencyActive);
    bool flashEnabled = transparencyActive && m_ui.checkBox_enhanceContrast->isChecked();
    m_ui.slider_flashControl->setEnabled(flashEnabled);
    m_ui.reinhardRadioButton->setEnabled(flashEnabled);
    m_ui.exponentialRadioButton->setEnabled(flashEnabled);
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
    float exposure = slider_to_exposure(m_ui.slider_flashControl->value());
    HighlightCompressionMode mode = m_ui.exponentialRadioButton->isChecked() ? HighlightCompressionMode::Exponential : HighlightCompressionMode::Reinhard;
    m_dataDispatcher.updateInformation(new GuiDataRenderTransparencyOptions(m_ui.checkBox_negativeEffect->isChecked(), m_ui.checkBox_enhanceContrast->isChecked(), exposure, mode, 0.f, m_focusCamera));
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
