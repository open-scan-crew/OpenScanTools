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
    m_ui.slider_kneeStart->setMinimumWidth(100.f * guiScale);
    m_ui.slider_kneeSoftness->setMinimumWidth(100.f * guiScale);

    connect(m_ui.slider_transparency, &QSlider::valueChanged, m_ui.spinBox_transparency, &QSpinBox::setValue);
    connect(m_ui.spinBox_transparency, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.slider_transparency, &QSlider::setValue);
    connect(m_ui.slider_transparency, &QSlider::valueChanged, this, &ToolBarRenderTransparency::slotTransparencyValueChanged);

    connect(m_ui.checkBox_transparency, &QCheckBox::stateChanged, this, &ToolBarRenderTransparency::slotTranparencyActivationChanged);

    connect(m_ui.checkBox_enhanceContrast, &QCheckBox::stateChanged, this, &ToolBarRenderTransparency::slotTransparencyOptionsChanged);
    connect(m_ui.checkBox_negativeEffect, &QCheckBox::stateChanged, this, &ToolBarRenderTransparency::slotTransparencyOptionsChanged);
    connect(m_ui.slider_kneeStart, &QSlider::valueChanged, m_ui.spinBox_kneeStart, &QSpinBox::setValue);
    connect(m_ui.spinBox_kneeStart, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.slider_kneeStart, &QSlider::setValue);
    connect(m_ui.slider_kneeStart, &QSlider::valueChanged, this, &ToolBarRenderTransparency::slotTransparencyOptionsChanged);
    connect(m_ui.slider_kneeSoftness, &QSlider::valueChanged, m_ui.spinBox_kneeSoftness, &QSpinBox::setValue);
    connect(m_ui.spinBox_kneeSoftness, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.slider_kneeSoftness, &QSlider::setValue);
    connect(m_ui.slider_kneeSoftness, &QSlider::valueChanged, this, &ToolBarRenderTransparency::slotTransparencyOptionsChanged);

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
    enableUI(transparencyActive);

    m_ui.checkBox_negativeEffect->setChecked(displayParameters.m_negativeEffect);
    m_ui.checkBox_enhanceContrast->setChecked(displayParameters.m_reduceFlash);
    m_ui.spinBox_kneeStart->setValue(static_cast<int>(displayParameters.m_kneeStart));
    m_ui.slider_kneeStart->setValue(static_cast<int>(displayParameters.m_kneeStart));
    m_ui.spinBox_kneeSoftness->setValue(static_cast<int>(displayParameters.m_kneeSoftness));
    m_ui.slider_kneeSoftness->setValue(static_cast<int>(displayParameters.m_kneeSoftness));

    blockAllSignals(false);
    updateKneeControlsState();
}


void ToolBarRenderTransparency::blockAllSignals(bool block)
{
    m_ui.spinBox_transparency->blockSignals(block);
    m_ui.slider_transparency->blockSignals(block);
    m_ui.checkBox_transparency->blockSignals(block);
    m_ui.checkBox_negativeEffect->blockSignals(block);
    m_ui.checkBox_enhanceContrast->blockSignals(block);
    m_ui.slider_kneeStart->blockSignals(block);
    m_ui.spinBox_kneeStart->blockSignals(block);
    m_ui.slider_kneeSoftness->blockSignals(block);
    m_ui.spinBox_kneeSoftness->blockSignals(block);
}

void ToolBarRenderTransparency::enableUI(bool transparencyActive)
{
    m_ui.spinBox_transparency->setEnabled(transparencyActive);
    m_ui.slider_transparency->setEnabled(transparencyActive);
    m_ui.checkBox_enhanceContrast->setEnabled(transparencyActive);
    m_ui.checkBox_negativeEffect->setEnabled(transparencyActive);
    updateKneeControlsState();
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
    float kneeStart = static_cast<float>(m_ui.slider_kneeStart->value());
    float kneeSoftness = static_cast<float>(m_ui.slider_kneeSoftness->value());
    m_dataDispatcher.updateInformation(new GuiDataRenderTransparencyOptions(m_ui.checkBox_negativeEffect->isChecked(), m_ui.checkBox_enhanceContrast->isChecked(), kneeStart, kneeSoftness, 0.f, m_focusCamera));
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
    updateKneeControlsState();
    sendTransparencyOptions();
}

void ToolBarRenderTransparency::updateKneeControlsState()
{
    bool transparencyActive = m_ui.checkBox_transparency->isChecked();
    bool kneeEnabled = transparencyActive && m_ui.checkBox_enhanceContrast->isChecked();
    m_ui.slider_kneeStart->setEnabled(kneeEnabled);
    m_ui.spinBox_kneeStart->setEnabled(kneeEnabled);
    m_ui.label_kneeStart->setEnabled(kneeEnabled);
    m_ui.slider_kneeSoftness->setEnabled(kneeEnabled);
    m_ui.spinBox_kneeSoftness->setEnabled(kneeEnabled);
    m_ui.label_kneeSoftness->setEnabled(kneeEnabled);
}
