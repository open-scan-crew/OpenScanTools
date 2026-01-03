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
    connect(m_ui.slider_flashControl, &QSlider::valueChanged, this, [this](int value) {
        m_ui.spinBox_flashControl->setValue(sliderToFlashControl(value));
    });
    connect(m_ui.spinBox_flashControl, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        m_ui.slider_flashControl->setValue(flashControlToSlider(static_cast<float>(value)));
        slotTransparencyOptionsChanged(0);
    });
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
    enableUI(transparencyActive);

    m_ui.checkBox_negativeEffect->setChecked(displayParameters.m_negativeEffect);
    m_ui.checkBox_enhanceContrast->setChecked(displayParameters.m_reduceFlash);
    m_ui.basicEnhanceContrastRadioButton->setChecked(!displayParameters.m_flashAdvanced);
    m_ui.advEnhanceContrastRadioButton->setChecked(displayParameters.m_flashAdvanced);
    m_ui.spinBox_flashControl->setValue(displayParameters.m_flashControl);
    m_ui.slider_flashControl->setValue(flashControlToSlider(displayParameters.m_flashControl));

    blockAllSignals(false);
    updateFlashControlState();
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
    m_ui.slider_flashControl->blockSignals(block);
    m_ui.spinBox_flashControl->blockSignals(block);
}

void ToolBarRenderTransparency::enableUI(bool transparencyActive)
{
    m_ui.spinBox_transparency->setEnabled(transparencyActive);
    m_ui.slider_transparency->setEnabled(transparencyActive);
    m_ui.checkBox_enhanceContrast->setEnabled(transparencyActive);
    m_ui.checkBox_negativeEffect->setEnabled(transparencyActive);
    updateFlashControlState();
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
    float flashControl = sliderToFlashControl(m_ui.slider_flashControl->value());
    m_dataDispatcher.updateInformation(new GuiDataRenderTransparencyOptions(m_ui.checkBox_negativeEffect->isChecked(), m_ui.checkBox_enhanceContrast->isChecked(), advanced, flashControl, 0.f, m_focusCamera));
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
    updateFlashControlState();
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
