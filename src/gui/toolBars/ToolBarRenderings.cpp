#include "gui/toolBars/ToolBarRenderings.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "controller/controls/ControlApplication.h"
#include "gui/texts/RenderingTexts.hpp"
#include "gui/UITransparencyConverter.h"

#include <algorithm>
#include <cmath>
#include <qcolordialog.h>

#include "models/graph/CameraNode.h"

namespace
{
    constexpr float DEPTH_LINING_STRENGTH_UI_SCALE = 1.5f;
    constexpr float DEPTH_LINING_SENSITIVITY_UI_SCALE = 3.0f;
    constexpr float kAoSizeMin = 4.0f;
    constexpr float kAoSizeMax = 32.0f;
    constexpr int kAoSliderMax = 100;

    float toNormalizedAoSize(float radius)
    {
        if (kAoSizeMax <= kAoSizeMin)
            return 0.0f;
        return std::clamp((radius - kAoSizeMin) / (kAoSizeMax - kAoSizeMin), 0.0f, 1.0f);
    }

    float fromNormalizedAoSize(float normalized)
    {
        return kAoSizeMin + std::clamp(normalized, 0.0f, 1.0f) * (kAoSizeMax - kAoSizeMin);
    }
}

ToolBarRenderings::ToolBarRenderings(IDataDispatcher& dataDispatcher, QWidget* parent, float guiScale)
    : QWidget(parent)
    , m_dataDispatcher(dataDispatcher)
    , m_currentRenderMode(UiRenderMode::Fake_Color)
    , m_brightness(0)
    , m_contrast(0)
    , m_lumiance(0)
    , m_saturation(0)
    , m_selectedColor(128, 128, 128)
    , m_intensityActive(true)
{
    m_ui.setupUi(this);
    setEnabled(false);

    populateEdgeAwareResolutionCombo();

    std::unordered_map<UiRenderMode, std::string> tradUiRenderMode(getTradUiRenderMode());
    for (uint32_t iterator(0); iterator < (uint32_t)UiRenderMode::UiRenderMode_MaxEnum; iterator++)
    {
#ifndef _DEBUG
        if (iterator == 2)
            continue;
#endif // !_DEBUG
        m_ui.comboBox_renderMode->addItem(QString::fromStdString(tradUiRenderMode.at(UiRenderMode(iterator))), QVariant(iterator));
    }

    m_ui.brightnessLuminanceSlider->setMinimumWidth(100.f * guiScale);
    m_ui.contrastSaturationSlider->setMinimumWidth(100.f * guiScale);
    m_ui.slider_transparency->setMinimumWidth(100.f * guiScale);
    m_ui.slider_flashControl->setMinimumWidth(100.f * guiScale);

    m_ui.lineEdit_rampMax->setType(NumericType::DISTANCE);
    m_ui.lineEdit_rampMin->setType(NumericType::DISTANCE);

    m_ui.pushButton_color->setPalette(QPalette(m_selectedColor));
    switchRenderMode((int)m_currentRenderMode);

    m_ui.normals_checkBox->setChecked(true);
    m_ui.spinBox_normalStrength->setEnabled(false);
    m_ui.slider_normalStrength->setEnabled(false);
    m_ui.doubleSpinBox_sharpness->setEnabled(false);
    m_ui.checkBox_blendColor->setEnabled(true);
    m_ui.aoSizeSlider->setRange(0, kAoSliderMax);
    m_ui.aoIntensitySlider->setRange(0, kAoSliderMax);

    m_ui.spinBox_normalStrength->setValue(50);
    m_ui.doubleSpinBox_sharpness->setValue(1.0);
    m_ui.aoSizeSpinBox->setValue(toNormalizedAoSize(16.0f));
    m_ui.aoIntensitySpinBox->setValue(0.5);
    m_ui.aoSizeSlider->setValue(static_cast<int>(std::round(m_ui.aoSizeSpinBox->value() * kAoSliderMax)));
    m_ui.aoIntensitySlider->setValue(static_cast<int>(std::round(m_ui.aoIntensitySpinBox->value() * kAoSliderMax)));
    updateAmbientOcclusionUi(m_ui.checkBox_ao->isChecked());

    connect(m_ui.comboBox_renderMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ToolBarRenderings::slotSetRenderMode);
    connect(m_ui.spinBox_pointSize, qOverload<int>(&QSpinBox::valueChanged), this, &ToolBarRenderings::slotSetPointSize);
    connect(m_ui.pushButton_color, &QPushButton::clicked, this, &ToolBarRenderings::slotColorPicking);

    connect(m_ui.brightnessLuminanceSlider, &QSlider::valueChanged, m_ui.brightnessLuminanceSpinBox, &QSpinBox::setValue);
    connect(m_ui.brightnessLuminanceSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.brightnessLuminanceSlider, &QSlider::setValue);
    connect(m_ui.brightnessLuminanceSlider, &QSlider::valueChanged, this, &ToolBarRenderings::slotBrightnessLuminanceValueChanged);

    connect(m_ui.contrastSaturationSlider, &QSlider::valueChanged, m_ui.contrastSaturationSpinBox, &QSpinBox::setValue);
    connect(m_ui.contrastSaturationSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.contrastSaturationSlider, &QSlider::setValue);
    connect(m_ui.contrastSaturationSlider, &QSlider::valueChanged, this, &ToolBarRenderings::slotContrastSaturationValueChanged);

    connect(m_ui.falseColorSlider, &QSlider::valueChanged, m_ui.falseColorSpinBox, &QSpinBox::setValue);
    connect(m_ui.falseColorSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.falseColorSlider, &QSlider::setValue);
    connect(m_ui.falseColorSlider, &QSlider::valueChanged, this, &ToolBarRenderings::slotFakeColorValueChanged);

    connect(m_ui.alphaObjectsSlider, &QSlider::valueChanged, m_ui.alphaObjectsSpinBox, &QSpinBox::setValue);
    connect(m_ui.alphaObjectsSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.alphaObjectsSlider, &QSlider::setValue);
    connect(m_ui.alphaObjectsSlider, &QSlider::valueChanged, this, &ToolBarRenderings::slotAlphaBoxesValueChanged);

    connect(m_ui.slider_transparency, &QSlider::valueChanged, m_ui.spinBox_transparency, &QSpinBox::setValue);
    connect(m_ui.spinBox_transparency, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.slider_transparency, &QSlider::setValue);
    connect(m_ui.slider_transparency, &QSlider::valueChanged, this, &ToolBarRenderings::slotTransparencyValueChanged);

    connect(m_ui.checkBox_transparency, &QCheckBox::stateChanged, this, &ToolBarRenderings::slotTranparencyActivationChanged);

    connect(m_ui.checkBox_enhanceContrast, &QCheckBox::stateChanged, this, &ToolBarRenderings::slotTransparencyOptionsChanged);
    connect(m_ui.checkBox_negativeEffect, &QCheckBox::stateChanged, this, &ToolBarRenderings::slotTransparencyOptionsChanged);
    connect(m_ui.basicEnhanceContrastRadioButton, &QRadioButton::toggled, this, &ToolBarRenderings::slotTransparencyOptionsChanged);
    connect(m_ui.advEnhanceContrastRadioButton, &QRadioButton::toggled, this, &ToolBarRenderings::slotTransparencyOptionsChanged);
    connect(m_ui.slider_flashControl, &QSlider::valueChanged, m_ui.spinBox_flashControl, &QSpinBox::setValue);
    connect(m_ui.spinBox_flashControl, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.slider_flashControl, &QSlider::setValue);
    connect(m_ui.slider_flashControl, &QSlider::valueChanged, this, &ToolBarRenderings::slotTransparencyOptionsChanged);

    connect(m_ui.lineEdit_rampMin, &QLineEdit::editingFinished, this, &ToolBarRenderings::slotRampValues);
    connect(m_ui.lineEdit_rampMax, &QLineEdit::editingFinished, this, &ToolBarRenderings::slotRampValues);
    connect(m_ui.spinBox_rampStep, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ToolBarRenderings::slotRampValues);

    connect(m_ui.normals_checkBox, &QCheckBox::stateChanged, this, &ToolBarRenderings::slotNormalsChanged);
    connect(m_ui.checkBox_blendColor, &QCheckBox::stateChanged, this, &ToolBarRenderings::slotNormalsChanged);
    connect(m_ui.checkBox_ao, &QCheckBox::stateChanged, this, &ToolBarRenderings::slotAmbientOcclusionChanged);

    connect(m_ui.slider_normalStrength, &QSlider::valueChanged, m_ui.spinBox_normalStrength, &QSpinBox::setValue);
    connect(m_ui.slider_normalStrength, &QSlider::valueChanged, this, &ToolBarRenderings::slotNormalsChanged);
    connect(m_ui.spinBox_normalStrength, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.slider_normalStrength, &QSlider::setValue);

    connect(m_ui.doubleSpinBox_sharpness, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolBarRenderings::slotSharpnessChanged);

    connect(m_ui.aoSizeSlider, &QSlider::valueChanged, this, &ToolBarRenderings::slotAmbientOcclusionChanged);
    connect(m_ui.aoIntensitySlider, &QSlider::valueChanged, this, &ToolBarRenderings::slotAmbientOcclusionChanged);
    connect(m_ui.aoSizeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolBarRenderings::slotAmbientOcclusionChanged);
    connect(m_ui.aoIntensitySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolBarRenderings::slotAmbientOcclusionChanged);

    connect(m_ui.checkBox_edgeAwareBlur, &QCheckBox::stateChanged, this, &ToolBarRenderings::slotEdgeAwareBlurToggled);
    connect(m_ui.slider_edgeAwareRadius, &QSlider::valueChanged, m_ui.spinBox_edgeAwareRadius, &QSpinBox::setValue);
    connect(m_ui.spinBox_edgeAwareRadius, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.slider_edgeAwareRadius, &QSlider::setValue);
    connect(m_ui.slider_edgeAwareRadius, &QSlider::valueChanged, this, &ToolBarRenderings::slotEdgeAwareBlurValueChanged);

    connect(m_ui.slider_edgeAwareDepth, &QSlider::valueChanged, m_ui.spinBox_edgeAwareDepth, &QSpinBox::setValue);
    connect(m_ui.spinBox_edgeAwareDepth, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.slider_edgeAwareDepth, &QSlider::setValue);
    connect(m_ui.slider_edgeAwareDepth, &QSlider::valueChanged, this, &ToolBarRenderings::slotEdgeAwareBlurValueChanged);

    connect(m_ui.slider_edgeAwareBlend, &QSlider::valueChanged, m_ui.spinBox_edgeAwareBlend, &QSpinBox::setValue);
    connect(m_ui.spinBox_edgeAwareBlend, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.slider_edgeAwareBlend, &QSlider::setValue);
    connect(m_ui.slider_edgeAwareBlend, &QSlider::valueChanged, this, &ToolBarRenderings::slotEdgeAwareBlurValueChanged);

    connect(m_ui.comboBox_edgeAwareResolution, qOverload<int>(&QComboBox::currentIndexChanged), this, &ToolBarRenderings::slotEdgeAwareBlurResolutionChanged);

    connect(m_ui.checkBox_depthLining, &QCheckBox::stateChanged, this, &ToolBarRenderings::slotDepthLiningToggled);
    connect(m_ui.slider_depthLiningStrength, &QSlider::valueChanged, m_ui.spinBox_depthLiningStrength, &QSpinBox::setValue);
    connect(m_ui.spinBox_depthLiningStrength, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.slider_depthLiningStrength, &QSlider::setValue);
    connect(m_ui.slider_depthLiningStrength, &QSlider::valueChanged, this, &ToolBarRenderings::slotDepthLiningValueChanged);
    connect(m_ui.slider_depthLiningSensitivity, &QSlider::valueChanged, m_ui.spinBox_depthLiningSensitivity, &QSpinBox::setValue);
    connect(m_ui.spinBox_depthLiningSensitivity, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.slider_depthLiningSensitivity, &QSlider::setValue);
    connect(m_ui.slider_depthLiningSensitivity, &QSlider::valueChanged, this, &ToolBarRenderings::slotDepthLiningSensitivityChanged);
    connect(m_ui.checkBox_depthLiningStrongMode, &QCheckBox::stateChanged, this, &ToolBarRenderings::slotDepthLiningStrongModeToggled);

    updateEdgeAwareBlurUi(m_ui.checkBox_edgeAwareBlur->isChecked());
    updateDepthLiningUi(m_ui.checkBox_depthLining->isChecked());

    registerGuiDataFunction(guiDType::renderBrightness, &ToolBarRenderings::onRenderBrightness);
    registerGuiDataFunction(guiDType::renderContrast, &ToolBarRenderings::onRenderContrast);
    registerGuiDataFunction(guiDType::renderColorMode, &ToolBarRenderings::onRenderColorMode);
    registerGuiDataFunction(guiDType::renderLuminance, &ToolBarRenderings::onRenderLuminance);
    registerGuiDataFunction(guiDType::renderBlending, &ToolBarRenderings::onRenderBlending);
    registerGuiDataFunction(guiDType::renderPointSize, &ToolBarRenderings::onRenderPointSize);
    registerGuiDataFunction(guiDType::renderSaturation, &ToolBarRenderings::onRenderSaturation);
    registerGuiDataFunction(guiDType::renderValueDisplay, &ToolBarRenderings::onRenderUnitUsage);
    registerGuiDataFunction(guiDType::renderActiveCamera, &ToolBarRenderings::onActiveCamera);
    registerGuiDataFunction(guiDType::focusViewport, &ToolBarRenderings::onFocusViewport);
    registerGuiDataFunction(guiDType::renderTransparency, &ToolBarRenderings::onRenderTransparency);
    registerGuiDataFunction(guiDType::projectLoaded, &ToolBarRenderings::onProjectLoad);
}

ToolBarRenderings::~ToolBarRenderings()
{
    m_dataDispatcher.unregisterObserver(this);
}

void ToolBarRenderings::changeEvent(QEvent* event)
{
}

void ToolBarRenderings::informData(IGuiData* data)
{
    if (m_methods.find(data->getType()) != m_methods.end())
    {
        GuiDataFunction method = m_methods.at(data->getType());
        (this->*method)(data);
    }
}

void ToolBarRenderings::onRenderBrightness(IGuiData* idata)
{
    GuiDataRenderBrightness* data = static_cast<GuiDataRenderBrightness*>(idata);
    m_ui.brightnessLuminanceSpinBox->setValue(data->m_brightness);
    m_ui.brightnessLuminanceSlider->setValue(data->m_brightness);
}

void ToolBarRenderings::onRenderContrast(IGuiData* idata)
{
    GuiDataRenderContrast* data = static_cast<GuiDataRenderContrast*>(idata);
    m_ui.contrastSaturationSpinBox->setValue(data->m_contrast);
    m_ui.contrastSaturationSlider->setValue(data->m_contrast);
}

void ToolBarRenderings::onRenderColorMode(IGuiData* idata)
{
    GuiDataRenderColorMode* data = static_cast<GuiDataRenderColorMode*>(idata);
    std::unordered_map<UiRenderMode, std::string> tradUiRenderMode(getTradUiRenderMode());
    m_ui.comboBox_renderMode->setCurrentText(QString::fromStdString(tradUiRenderMode.at(UiRenderMode(static_cast<int>(data->m_mode)))));
}

void ToolBarRenderings::onRenderLuminance(IGuiData* idata)
{
    GuiDataRenderLuminance* data = static_cast<GuiDataRenderLuminance*>(idata);
    m_ui.brightnessLuminanceSpinBox->setValue(data->m_luminance);
    m_ui.brightnessLuminanceSlider->setValue(data->m_luminance);
}

void ToolBarRenderings::onRenderBlending(IGuiData* idata)
{
    GuiDataRenderBlending* data = static_cast<GuiDataRenderBlending*>(idata);
    m_ui.falseColorSpinBox->setValue(data->m_hue);
    m_ui.falseColorSlider->setValue(data->m_hue);
}

void ToolBarRenderings::onRenderPointSize(IGuiData* idata)
{
    GuiDataRenderPointSize* data = static_cast<GuiDataRenderPointSize*>(idata);
    if (data->m_pointSize != m_ui.spinBox_pointSize->value())
    {
        m_ui.spinBox_pointSize->blockSignals(true);
        m_ui.spinBox_pointSize->setValue(data->m_pointSize);
        m_ui.spinBox_pointSize->blockSignals(false);
    }
}

void ToolBarRenderings::onRenderSaturation(IGuiData* idata)
{
    GuiDataRenderSaturation* data = static_cast<GuiDataRenderSaturation*>(idata);
    m_ui.contrastSaturationSpinBox->setValue(data->m_saturation);
    m_ui.contrastSaturationSlider->setValue(data->m_saturation);
}

void ToolBarRenderings::onRenderAlphaObjects(IGuiData* idata)
{
    GuiDataAlphaObjectsRendering* data = static_cast<GuiDataAlphaObjectsRendering*>(idata);
    int value((data->m_alpha * 100) + 1);
    m_ui.alphaObjectsSpinBox->setValue(value);
    m_ui.alphaObjectsSlider->setValue(value);
}

void ToolBarRenderings::onRenderUnitUsage(IGuiData* idata)
{
    auto castdata = static_cast<GuiDataRenderUnitUsage*>(idata);

    m_ui.lineEdit_rampMin->setUnit(castdata->m_valueParameters.distanceUnit);
    m_ui.lineEdit_rampMax->setUnit(castdata->m_valueParameters.distanceUnit);
}

void ToolBarRenderings::onProjectLoad(IGuiData* idata)
{
    GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(idata);
    setEnabled(plData->m_isProjectLoad);
}

void ToolBarRenderings::onFocusViewport(IGuiData* data)
{
    GuiDataFocusViewport* castData = static_cast<GuiDataFocusViewport*>(data);
    if (castData->m_forceFocus && castData->m_camera)
        m_focusCamera = castData->m_camera;
}

void ToolBarRenderings::onActiveCamera(IGuiData* idata)
{
    auto infos = static_cast<GuiDataCameraInfo*>(idata);
    if (infos->m_camera && m_focusCamera != infos->m_camera)
        return;

    ReadPtr<CameraNode> rCam = m_focusCamera.cget();
    if (!rCam)
        return;
    const DisplayParameters& displayParameters = rCam->getDisplayParameters();

    blockAllSignals(true);

    std::unordered_map<UiRenderMode, std::string> tradUiRenderMode(getTradUiRenderMode());
    m_ui.comboBox_renderMode->setCurrentText(QString::fromStdString(tradUiRenderMode.at(displayParameters.m_mode)));
    m_selectedColor.setRgbF(displayParameters.m_flatColor.x, displayParameters.m_flatColor.y, displayParameters.m_flatColor.z);
    m_ui.pushButton_color->setPalette(QPalette(m_selectedColor));
    switchRenderMode(static_cast<int>(displayParameters.m_mode));

    m_contrast = displayParameters.m_contrast;
    m_brightness = displayParameters.m_brightness;

    m_lumiance = displayParameters.m_luminance;
    m_saturation = displayParameters.m_saturation;

    if (m_intensityActive)
    {
        m_ui.brightnessLuminanceSpinBox->setValue(displayParameters.m_brightness);
        m_ui.brightnessLuminanceSlider->setValue(displayParameters.m_brightness);

        m_ui.contrastSaturationSpinBox->setValue(displayParameters.m_contrast);
        m_ui.contrastSaturationSlider->setValue(displayParameters.m_contrast);
    }
    else
    {
        m_ui.brightnessLuminanceSpinBox->setValue(displayParameters.m_luminance);
        m_ui.brightnessLuminanceSlider->setValue(displayParameters.m_luminance);

        m_ui.contrastSaturationSpinBox->setValue(displayParameters.m_saturation);
        m_ui.contrastSaturationSlider->setValue(displayParameters.m_saturation);
    }

    m_ui.falseColorSpinBox->setValue(displayParameters.m_hue);
    m_ui.falseColorSlider->setValue(displayParameters.m_hue);

    if (displayParameters.m_pointSize != m_ui.spinBox_pointSize->value())
        m_ui.spinBox_pointSize->setValue(displayParameters.m_pointSize);

    int value(100 - (int)(displayParameters.m_alphaObject * 100));
    m_ui.alphaObjectsSpinBox->setValue(value);
    m_ui.alphaObjectsSlider->setValue(value);

    m_ui.lineEdit_rampMin->setValue(displayParameters.m_distRampMin);
    m_ui.lineEdit_rampMax->setValue(displayParameters.m_distRampMax);
    m_ui.spinBox_rampStep->setValue(displayParameters.m_distRampSteps);

    updateNormals(displayParameters.m_postRenderingNormals);
    updateAmbientOcclusion(displayParameters.m_postRenderingAmbientOcclusion);
    m_transparencyActive = (displayParameters.m_blendMode == BlendMode::Transparent);
    updateAmbientOcclusionUi(m_ui.checkBox_ao->isChecked());

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
    updateFlashControlState();

    const EdgeAwareBlur& blurSettings = displayParameters.m_edgeAwareBlur;
    int radiusValue = std::clamp(static_cast<int>(std::round(blurSettings.radius)), m_ui.slider_edgeAwareRadius->minimum(), m_ui.slider_edgeAwareRadius->maximum());
    int depthValue = std::clamp(static_cast<int>(std::round(blurSettings.depthThreshold * 100.f)), m_ui.slider_edgeAwareDepth->minimum(), m_ui.slider_edgeAwareDepth->maximum());
    int blendValue = std::clamp(static_cast<int>(std::round(blurSettings.blendStrength * 100.f)), m_ui.slider_edgeAwareBlend->minimum(), m_ui.slider_edgeAwareBlend->maximum());
    const DepthLining& liningSettings = displayParameters.m_depthLining;
    int liningStrength = std::clamp(static_cast<int>(std::round((liningSettings.strength / DEPTH_LINING_STRENGTH_UI_SCALE) * 100.f)), m_ui.slider_depthLiningStrength->minimum(), m_ui.slider_depthLiningStrength->maximum());
    int liningSensitivity = std::clamp(static_cast<int>(std::round((liningSettings.sensitivity / DEPTH_LINING_SENSITIVITY_UI_SCALE) * 100.f)), m_ui.slider_depthLiningSensitivity->minimum(), m_ui.slider_depthLiningSensitivity->maximum());

    m_ui.checkBox_edgeAwareBlur->setChecked(blurSettings.enabled);
    m_ui.slider_edgeAwareRadius->setValue(radiusValue);
    m_ui.spinBox_edgeAwareRadius->setValue(radiusValue);

    m_ui.slider_edgeAwareDepth->setValue(depthValue);
    m_ui.spinBox_edgeAwareDepth->setValue(depthValue);

    m_ui.slider_edgeAwareBlend->setValue(blendValue);
    m_ui.spinBox_edgeAwareBlend->setValue(blendValue);

    if (blurSettings.resolutionScale <= 0.35f)
        m_ui.comboBox_edgeAwareResolution->setCurrentIndex(2);
    else if (blurSettings.resolutionScale <= 0.75f)
        m_ui.comboBox_edgeAwareResolution->setCurrentIndex(1);
    else
        m_ui.comboBox_edgeAwareResolution->setCurrentIndex(0);
    updateEdgeAwareBlurUi(blurSettings.enabled);

    m_ui.checkBox_depthLining->setChecked(liningSettings.enabled);
    m_ui.slider_depthLiningStrength->setValue(liningStrength);
    m_ui.spinBox_depthLiningStrength->setValue(liningStrength);
    m_ui.slider_depthLiningSensitivity->setValue(liningSensitivity);
    m_ui.spinBox_depthLiningSensitivity->setValue(liningSensitivity);
    m_ui.checkBox_depthLiningStrongMode->setChecked(liningSettings.strongMode);
    updateDepthLiningUi(liningSettings.enabled);

    blockAllSignals(false);
}

void ToolBarRenderings::onRenderTransparency(IGuiData* data)
{
    auto transparencyData = static_cast<GuiDataRenderTransparency*>(data);
    m_transparencyActive = (transparencyData->m_mode == BlendMode::Transparent);
    updateAmbientOcclusionUi(m_ui.checkBox_ao->isChecked());
}

void ToolBarRenderings::updateNormals(const PostRenderingNormals& normalsParams)
{
    Qt::CheckState normalState = Qt::CheckState::Unchecked;
    if (normalsParams.show)
    {
        if (normalsParams.inverseTone)
            normalState = Qt::CheckState::PartiallyChecked;
        else
            normalState = Qt::CheckState::Checked;
    }
    blockAllSignals(true);

    m_ui.normals_checkBox->setCheckState(normalState);

    int newNormalValue = (int)std::round(normalsParams.normalStrength * 100.f);

    m_ui.slider_normalStrength->setEnabled(normalState != Qt::CheckState::Unchecked);
    m_ui.slider_normalStrength->setValue(newNormalValue);
    m_ui.spinBox_normalStrength->setEnabled(normalState != Qt::CheckState::Unchecked);
    m_ui.spinBox_normalStrength->setValue(newNormalValue);

    m_ui.doubleSpinBox_sharpness->setEnabled(normalState != Qt::CheckState::Unchecked);
    m_ui.checkBox_blendColor->setEnabled(normalState != Qt::CheckState::Unchecked);

    m_ui.doubleSpinBox_sharpness->setValue(normalsParams.gloss);
    m_ui.checkBox_blendColor->setChecked(normalsParams.blendColor);

    blockAllSignals(false);
}

void ToolBarRenderings::updateAmbientOcclusion(const PostRenderingAmbientOcclusion& aoParams)
{
    blockAllSignals(true);

    m_ui.checkBox_ao->setChecked(aoParams.enabled);

    const float sizeNormalized = toNormalizedAoSize(aoParams.radius);
    m_ui.aoSizeSpinBox->setValue(sizeNormalized);
    m_ui.aoSizeSlider->setValue(static_cast<int>(std::round(sizeNormalized * kAoSliderMax)));

    m_ui.aoIntensitySpinBox->setValue(std::clamp(static_cast<double>(aoParams.intensity), 0.0, 1.0));
    m_ui.aoIntensitySlider->setValue(static_cast<int>(std::round(std::clamp(aoParams.intensity, 0.0f, 1.0f) * kAoSliderMax)));

    blockAllSignals(false);
    updateAmbientOcclusionUi(aoParams.enabled);
}

void ToolBarRenderings::updateAmbientOcclusionUi(bool aoEnabled)
{
    const bool enableControls = aoEnabled && !m_transparencyActive;
    m_ui.checkBox_ao->setEnabled(!m_transparencyActive);
    m_ui.aoSizeSlider->setEnabled(enableControls);
    m_ui.aoSizeSpinBox->setEnabled(enableControls);
    m_ui.aoIntensitySlider->setEnabled(enableControls);
    m_ui.aoIntensitySpinBox->setEnabled(enableControls);
}

void ToolBarRenderings::blockAllSignals(bool block)
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

    m_ui.brightnessLuminanceSpinBox->blockSignals(block);
    m_ui.brightnessLuminanceSlider->blockSignals(block);
    m_ui.contrastSaturationSpinBox->blockSignals(block);
    m_ui.contrastSaturationSlider->blockSignals(block);
    m_ui.comboBox_renderMode->blockSignals(block);
    m_ui.falseColorSpinBox->blockSignals(block);
    m_ui.falseColorSlider->blockSignals(block);
    m_ui.spinBox_pointSize->blockSignals(block);
    m_ui.alphaObjectsSpinBox->blockSignals(block);
    m_ui.alphaObjectsSlider->blockSignals(block);
    m_ui.lineEdit_rampMax->blockSignals(block);
    m_ui.lineEdit_rampMin->blockSignals(block);
    m_ui.spinBox_rampStep->blockSignals(block);

    m_ui.normals_checkBox->blockSignals(block);
    m_ui.slider_normalStrength->blockSignals(block);
    m_ui.spinBox_normalStrength->blockSignals(block);
    m_ui.doubleSpinBox_sharpness->blockSignals(block);
    m_ui.checkBox_blendColor->blockSignals(block);
    m_ui.checkBox_ao->blockSignals(block);
    m_ui.aoSizeSlider->blockSignals(block);
    m_ui.aoSizeSpinBox->blockSignals(block);
    m_ui.aoIntensitySlider->blockSignals(block);
    m_ui.aoIntensitySpinBox->blockSignals(block);

    m_ui.checkBox_edgeAwareBlur->blockSignals(block);
    m_ui.slider_edgeAwareRadius->blockSignals(block);
    m_ui.spinBox_edgeAwareRadius->blockSignals(block);
    m_ui.slider_edgeAwareDepth->blockSignals(block);
    m_ui.spinBox_edgeAwareDepth->blockSignals(block);
    m_ui.slider_edgeAwareBlend->blockSignals(block);
    m_ui.spinBox_edgeAwareBlend->blockSignals(block);
    m_ui.comboBox_edgeAwareResolution->blockSignals(block);
    m_ui.checkBox_depthLining->blockSignals(block);
    m_ui.slider_depthLiningStrength->blockSignals(block);
    m_ui.spinBox_depthLiningStrength->blockSignals(block);
    m_ui.slider_depthLiningSensitivity->blockSignals(block);
    m_ui.spinBox_depthLiningSensitivity->blockSignals(block);
    m_ui.checkBox_depthLiningStrongMode->blockSignals(block);
}

void ToolBarRenderings::populateEdgeAwareResolutionCombo()
{
    m_ui.comboBox_edgeAwareResolution->clear();
    m_ui.comboBox_edgeAwareResolution->addItem(tr("Full res"));
    m_ui.comboBox_edgeAwareResolution->addItem(tr("Half res"));
    m_ui.comboBox_edgeAwareResolution->addItem(tr("Quarter res"));
}

void ToolBarRenderings::updateEdgeAwareBlurUi(bool enabled)
{
    m_ui.slider_edgeAwareRadius->setEnabled(enabled);
    m_ui.spinBox_edgeAwareRadius->setEnabled(enabled);
    m_ui.slider_edgeAwareDepth->setEnabled(enabled);
    m_ui.spinBox_edgeAwareDepth->setEnabled(enabled);
    m_ui.slider_edgeAwareBlend->setEnabled(enabled);
    m_ui.spinBox_edgeAwareBlend->setEnabled(enabled);
    m_ui.comboBox_edgeAwareResolution->setEnabled(enabled);
}

EdgeAwareBlur ToolBarRenderings::getEdgeAwareBlurFromUi() const
{
    EdgeAwareBlur settings = {};
    settings.enabled = m_ui.checkBox_edgeAwareBlur->isChecked();
    settings.radius = static_cast<float>(m_ui.spinBox_edgeAwareRadius->value());
    settings.depthThreshold = m_ui.spinBox_edgeAwareDepth->value() / 100.f;
    settings.blendStrength = m_ui.spinBox_edgeAwareBlend->value() / 100.f;
    switch (m_ui.comboBox_edgeAwareResolution->currentIndex())
    {
    case 1:
        settings.resolutionScale = 0.5f;
        break;
    case 2:
        settings.resolutionScale = 0.25f;
        break;
    default:
        settings.resolutionScale = 1.0f;
        break;
    }

    return settings;
}

void ToolBarRenderings::updateDepthLiningUi(bool enabled)
{
    m_ui.slider_depthLiningStrength->setEnabled(enabled);
    m_ui.spinBox_depthLiningStrength->setEnabled(enabled);
    m_ui.slider_depthLiningSensitivity->setEnabled(enabled);
    m_ui.spinBox_depthLiningSensitivity->setEnabled(enabled);
    m_ui.checkBox_depthLiningStrongMode->setEnabled(enabled);
}

DepthLining ToolBarRenderings::getDepthLiningFromUi() const
{
    DepthLining settings = {};
    settings.enabled = m_ui.checkBox_depthLining->isChecked();
    const float strengthPct = m_ui.spinBox_depthLiningStrength->value() / 100.f;
    const float sensitivityPct = m_ui.spinBox_depthLiningSensitivity->value() / 100.f;
    settings.strength = strengthPct * DEPTH_LINING_STRENGTH_UI_SCALE;
    settings.sensitivity = sensitivityPct * DEPTH_LINING_SENSITIVITY_UI_SCALE;
    settings.threshold = std::lerp(0.012f, 0.001f, sensitivityPct);
    settings.strongMode = m_ui.checkBox_depthLiningStrongMode->isChecked();
    return settings;
}

void ToolBarRenderings::switchRenderMode(const int& mode)
{
    m_currentRenderMode = UiRenderMode(mode);
    switch (m_currentRenderMode)
    {
    case UiRenderMode::Flat:
    {
        m_ui.adjust_options->hide();
        m_ui.ramp_options->setVisible(false);
        m_ui.pushButton_color->show();
        enableFalseColor(false);
    }
    break;
    case UiRenderMode::Flat_Distance_Ramp:
    case UiRenderMode::Distance_Ramp:
    {
        showContrastBrightness();
        m_ui.ramp_options->setVisible(true);
        m_ui.pushButton_color->hide();
        enableFalseColor(false);
    }
    break;
    case UiRenderMode::Grey_Colored:
    {
        showContrastBrightness();
        m_ui.ramp_options->setVisible(false);
        m_ui.pushButton_color->show();
        enableFalseColor(false);
    }
    break;
    case UiRenderMode::Scans_Color:
    case UiRenderMode::Clusters_Color:
    {
        showContrastBrightness();
        m_ui.ramp_options->setVisible(false);
        m_ui.pushButton_color->hide();
        enableFalseColor(false);
    }
    break;
    case UiRenderMode::Intensity:
    {
        showContrastBrightness();
        m_ui.ramp_options->setVisible(false);
        m_ui.pushButton_color->hide();
        enableFalseColor(false);
    }
    break;
    case UiRenderMode::RGB:
    {
        showSaturationLuminance();
        m_ui.ramp_options->setVisible(false);
        m_ui.pushButton_color->hide();
        enableFalseColor(false);
    }
    break;
    case UiRenderMode::IntensityRGB_Combined:
    case UiRenderMode::Fake_Color:
    {
        showSaturationLuminance();
        m_ui.ramp_options->setVisible(false);
        m_ui.pushButton_color->hide();
        enableFalseColor(true);
    }
    break;
    }
}

void ToolBarRenderings::showContrastBrightness()
{
    if (!m_intensityActive)
    {
        m_saturation = m_ui.contrastSaturationSpinBox->value();
        m_lumiance = m_ui.brightnessLuminanceSpinBox->value();
        m_intensityActive = true;
        m_ui.contrastSaturationSpinBox->setValue(m_contrast);
        m_ui.brightnessLuminanceSpinBox->setValue(m_brightness);
    }

    m_ui.adjust_options->show();
    m_ui.contrastSaturationLabel->setText(TEXT_RENDER_CONTRAST);
    m_ui.brightnessLuminanceLabel->setText(TEXT_RENDER_BRIGHTNESS);
}

void ToolBarRenderings::showSaturationLuminance()
{
    if (m_intensityActive)
    {
        m_contrast = m_ui.contrastSaturationSpinBox->value();
        m_brightness = m_ui.brightnessLuminanceSpinBox->value();
        m_intensityActive = false;
        m_ui.contrastSaturationSpinBox->setValue(m_saturation);
        m_ui.brightnessLuminanceSpinBox->setValue(m_lumiance);
    }

    m_ui.adjust_options->show();
    m_ui.contrastSaturationLabel->setText(TEXT_RENDER_SATURATION);
    m_ui.brightnessLuminanceLabel->setText(TEXT_RENDER_LUMINANCE);
}

void ToolBarRenderings::enableFalseColor(bool enable)
{
    m_ui.falseColorSlider->setEnabled(enable);
    m_ui.falseColorSpinBox->setEnabled(enable);
}

void ToolBarRenderings::sendTransparency()
{
    int uiTransparency = m_ui.slider_transparency->value();
    float t = ui::transparency::uiValue_to_trueValue(uiTransparency);
    BlendMode blend = m_ui.checkBox_transparency->isChecked() ? BlendMode::Transparent : BlendMode::Opaque;
    m_dataDispatcher.updateInformation(new GuiDataRenderTransparency(blend, t, m_focusCamera), this);
}

void ToolBarRenderings::sendTransparencyOptions()
{
    bool advanced = m_ui.advEnhanceContrastRadioButton->isChecked();
    float flashControl = static_cast<float>(m_ui.slider_flashControl->value());
    m_dataDispatcher.updateInformation(new GuiDataRenderTransparencyOptions(m_ui.checkBox_negativeEffect->isChecked(), m_ui.checkBox_enhanceContrast->isChecked(), advanced, flashControl, 0.f, m_focusCamera));
}

void ToolBarRenderings::slotBrightnessLuminanceValueChanged(int value)
{
    if (m_intensityActive)
        m_dataDispatcher.updateInformation(new GuiDataRenderBrightness(value, m_focusCamera), this);
    else
        m_dataDispatcher.updateInformation(new GuiDataRenderLuminance(value, m_focusCamera), this);
}

void ToolBarRenderings::slotContrastSaturationValueChanged(int value)
{
    if (m_intensityActive)
        m_dataDispatcher.updateInformation(new GuiDataRenderContrast(value, m_focusCamera), this);
    else
        m_dataDispatcher.updateInformation(new GuiDataRenderSaturation(value, m_focusCamera), this);
}

void ToolBarRenderings::slotFakeColorValueChanged(int value)
{
    m_dataDispatcher.updateInformation(new GuiDataRenderBlending(value, m_focusCamera), this);
}

void ToolBarRenderings::slotTranparencyActivationChanged(int value)
{
    m_ui.spinBox_transparency->setEnabled(value > 0);
    m_ui.slider_transparency->setEnabled(value > 0);
    updateFlashControlState();
    sendTransparency();
}

void ToolBarRenderings::slotTransparencyValueChanged(int value)
{
    sendTransparency();
}

void ToolBarRenderings::slotTransparencyOptionsChanged(int value)
{
    updateFlashControlState();
    sendTransparencyOptions();
}

void ToolBarRenderings::slotSetPointSize(int pointSize)
{
    m_dataDispatcher.sendControl(new control::application::SetRenderPointSize(pointSize, m_focusCamera));
}

void ToolBarRenderings::slotSetRenderMode(int mode)
{
    switchRenderMode(m_ui.comboBox_renderMode->currentData().toInt());

    m_dataDispatcher.sendControl(new control::application::RenderModeUpdate(UiRenderMode(m_ui.comboBox_renderMode->currentData().toInt()), m_focusCamera));
}

void ToolBarRenderings::slotColorPicking()
{
    QColor newColor = QColorDialog::getColor(m_selectedColor, this);
    if (newColor.isValid())
    {
        m_selectedColor = newColor;
        m_ui.pushButton_color->setPalette(QPalette(m_selectedColor));
        m_dataDispatcher.updateInformation(new GuiDataRenderFlatColor(m_selectedColor.redF(), m_selectedColor.greenF(), m_selectedColor.blueF(), m_focusCamera), this);
    }
}

bool ToolBarRenderings::rampValidValue(float& min, float& max, int& step)
{
    min = m_ui.lineEdit_rampMin->getValue();
    max = m_ui.lineEdit_rampMax->getValue();

    step = m_ui.spinBox_rampStep->value();
    if (!step)
        step = 0xFFFF;
    return true;
}

void ToolBarRenderings::slotRampValues()
{
    float minV, maxV;
    int step;
    if (rampValidValue(minV, maxV, step))
    {
        m_dataDispatcher.updateInformation(new GuiDataRenderDistanceRampValues(minV, maxV, step, m_focusCamera), this);
    }
}

void ToolBarRenderings::slotNormalsChanged()
{
    PostRenderingNormals lighting = {};
    bool enable = m_ui.normals_checkBox->isChecked();
    m_ui.slider_normalStrength->setEnabled(enable);
    m_ui.spinBox_normalStrength->setEnabled(enable);
    m_ui.doubleSpinBox_sharpness->setEnabled(enable);
    m_ui.checkBox_blendColor->setEnabled(enable);

    lighting.show = enable;
    lighting.inverseTone = (m_ui.normals_checkBox->checkState() == Qt::CheckState::PartiallyChecked);
    lighting.normalStrength = m_ui.spinBox_normalStrength->value() / 100.f;
    lighting.gloss = m_ui.doubleSpinBox_sharpness->value();
    lighting.blendColor = m_ui.checkBox_blendColor->isChecked();

    m_dataDispatcher.updateInformation(new GuiDataPostRenderingNormals(lighting, false, m_focusCamera), this);
}

void ToolBarRenderings::slotSharpnessChanged(double value)
{
    slotNormalsChanged();
}

void ToolBarRenderings::slotAmbientOcclusionChanged()
{
    updateAmbientOcclusionUi(m_ui.checkBox_ao->isChecked());

    QObject* source = sender();
    const double sizeFromSlider = m_ui.aoSizeSlider->value() / static_cast<double>(kAoSliderMax);
    const double intensityFromSlider = m_ui.aoIntensitySlider->value() / static_cast<double>(kAoSliderMax);
    const double sizeNormalized = (source == m_ui.aoSizeSlider) ? sizeFromSlider : m_ui.aoSizeSpinBox->value();
    const double intensityNormalized = (source == m_ui.aoIntensitySlider) ? intensityFromSlider : m_ui.aoIntensitySpinBox->value();

    blockAllSignals(true);
    m_ui.aoSizeSpinBox->setValue(sizeNormalized);
    m_ui.aoSizeSlider->setValue(static_cast<int>(std::round(sizeNormalized * kAoSliderMax)));
    m_ui.aoIntensitySpinBox->setValue(intensityNormalized);
    m_ui.aoIntensitySlider->setValue(static_cast<int>(std::round(intensityNormalized * kAoSliderMax)));
    blockAllSignals(false);

    PostRenderingAmbientOcclusion aoSettings = {};
    aoSettings.enabled = m_ui.checkBox_ao->isChecked();
    aoSettings.radius = fromNormalizedAoSize(static_cast<float>(sizeNormalized));
    aoSettings.intensity = static_cast<float>(std::clamp(intensityNormalized, 0.0, 1.0));

    m_dataDispatcher.updateInformation(new GuiDataRenderAmbientOcclusion(aoSettings, m_focusCamera), this);
}

void ToolBarRenderings::slotEdgeAwareBlurToggled(int state)
{
    updateEdgeAwareBlurUi(state == Qt::Checked);
    m_dataDispatcher.updateInformation(new GuiDataEdgeAwareBlur(getEdgeAwareBlurFromUi(), m_focusCamera), this);
}

void ToolBarRenderings::slotEdgeAwareBlurValueChanged(int value)
{
    (void)value;
    if (!m_ui.checkBox_edgeAwareBlur->isChecked())
        return;

    m_dataDispatcher.updateInformation(new GuiDataEdgeAwareBlur(getEdgeAwareBlurFromUi(), m_focusCamera), this);
}

void ToolBarRenderings::slotEdgeAwareBlurResolutionChanged(int index)
{
    (void)index;
    if (!m_ui.checkBox_edgeAwareBlur->isChecked())
        return;

    m_dataDispatcher.updateInformation(new GuiDataEdgeAwareBlur(getEdgeAwareBlurFromUi(), m_focusCamera), this);
}

void ToolBarRenderings::slotDepthLiningToggled(int state)
{
    updateDepthLiningUi(state == Qt::Checked);
    m_dataDispatcher.updateInformation(new GuiDataDepthLining(getDepthLiningFromUi(), m_focusCamera), this);
}

void ToolBarRenderings::slotDepthLiningValueChanged(int value)
{
    (void)value;
    if (!m_ui.checkBox_depthLining->isChecked())
        return;

    m_dataDispatcher.updateInformation(new GuiDataDepthLining(getDepthLiningFromUi(), m_focusCamera), this);
}

void ToolBarRenderings::slotDepthLiningSensitivityChanged(int value)
{
    (void)value;
    if (!m_ui.checkBox_depthLining->isChecked())
        return;

    m_dataDispatcher.updateInformation(new GuiDataDepthLining(getDepthLiningFromUi(), m_focusCamera), this);
}

void ToolBarRenderings::slotDepthLiningStrongModeToggled(int state)
{
    (void)state;
    if (!m_ui.checkBox_depthLining->isChecked())
    {
        updateDepthLiningUi(false);
        return;
    }

    m_dataDispatcher.updateInformation(new GuiDataDepthLining(getDepthLiningFromUi(), m_focusCamera), this);
}

void ToolBarRenderings::slotAlphaBoxesValueChanged(int value)
{
    m_dataDispatcher.updateInformation(new GuiDataAlphaObjectsRendering(1.0f - (value / 100.0f), m_focusCamera), this);
}

void ToolBarRenderings::updateFlashControlState()
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
