#include "gui/toolBars/ToolBarRenderingAdvanced.h"

#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataGeneralProject.h"

#include "models/graph/CameraNode.h"

#include <cmath>

ToolBarRenderingAdvanced::ToolBarRenderingAdvanced(IDataDispatcher& dataDispatcher, QWidget* parent, float guiScale)
    : QWidget(parent)
    , m_dataDispatcher(dataDispatcher)
    , m_focusCamera()
{
    (void)guiScale;
    m_ui.setupUi(this);
    setEnabled(false);

    // Defaults
    m_ui.checkBox_billboardEnable->setChecked(false);
    m_ui.slider_billboardFeather->setValue(15);
    m_ui.doubleSpinBox_billboardFeather->setValue(15.0);

    m_ui.checkBox_edlEnable->setChecked(false);
    m_ui.slider_edlStrength->setValue(100);
    m_ui.doubleSpinBox_edlStrength->setValue(1.0);
    m_ui.slider_edlRadius->setValue(15);
    m_ui.doubleSpinBox_edlRadius->setValue(1.5);
    m_ui.slider_edlBias->setValue(35);
    m_ui.doubleSpinBox_edlBias->setValue(0.35);

    m_ui.slider_billboardFeather->setEnabled(false);
    m_ui.doubleSpinBox_billboardFeather->setEnabled(false);
    m_ui.slider_edlStrength->setEnabled(false);
    m_ui.doubleSpinBox_edlStrength->setEnabled(false);
    m_ui.slider_edlRadius->setEnabled(false);
    m_ui.doubleSpinBox_edlRadius->setEnabled(false);
    m_ui.slider_edlBias->setEnabled(false);
    m_ui.doubleSpinBox_edlBias->setEnabled(false);

    // Connections - billboard
    connect(m_ui.checkBox_billboardEnable, &QCheckBox::stateChanged, this, &ToolBarRenderingAdvanced::slotBillboardToggled);
    connect(m_ui.slider_billboardFeather, &QSlider::valueChanged, this, &ToolBarRenderingAdvanced::slotBillboardFeatherSlider);
    connect(m_ui.doubleSpinBox_billboardFeather, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolBarRenderingAdvanced::slotBillboardFeatherSpin);

    // Connections - EDL
    connect(m_ui.checkBox_edlEnable, &QCheckBox::stateChanged, this, &ToolBarRenderingAdvanced::slotEdlToggled);
    connect(m_ui.slider_edlStrength, &QSlider::valueChanged, this, &ToolBarRenderingAdvanced::slotEdlStrengthSlider);
    connect(m_ui.doubleSpinBox_edlStrength, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolBarRenderingAdvanced::slotEdlStrengthSpin);
    connect(m_ui.slider_edlRadius, &QSlider::valueChanged, this, &ToolBarRenderingAdvanced::slotEdlRadiusSlider);
    connect(m_ui.doubleSpinBox_edlRadius, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolBarRenderingAdvanced::slotEdlRadiusSpin);
    connect(m_ui.slider_edlBias, &QSlider::valueChanged, this, &ToolBarRenderingAdvanced::slotEdlBiasSlider);
    connect(m_ui.doubleSpinBox_edlBias, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolBarRenderingAdvanced::slotEdlBiasSpin);

    registerGuiDataFunction(guiDType::projectLoaded, &ToolBarRenderingAdvanced::onProjectLoad);
    registerGuiDataFunction(guiDType::renderActiveCamera, &ToolBarRenderingAdvanced::onActiveCamera);
    registerGuiDataFunction(guiDType::focusViewport, &ToolBarRenderingAdvanced::onFocusViewport);
}

void ToolBarRenderingAdvanced::informData(IGuiData* data)
{
    if (m_methods.find(data->getType()) != m_methods.end())
    {
        GuiDataFunction method = m_methods.at(data->getType());
        (this->*method)(data);
    }
}

void ToolBarRenderingAdvanced::onProjectLoad(IGuiData* data)
{
    GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
    setEnabled(plData->m_isProjectLoad);
}

void ToolBarRenderingAdvanced::onFocusViewport(IGuiData* data)
{
    GuiDataFocusViewport* castData = static_cast<GuiDataFocusViewport*>(data);
    if (castData->m_forceFocus && castData->m_camera)
        m_focusCamera = castData->m_camera;
}

void ToolBarRenderingAdvanced::onActiveCamera(IGuiData* data)
{
    auto infos = static_cast<GuiDataCameraInfo*>(data);
    if (infos->m_camera && m_focusCamera != infos->m_camera)
        return;

    ReadPtr<CameraNode> rCam = m_focusCamera.cget();
    if (!rCam)
        return;

    const DisplayParameters& displayParameters = rCam->getDisplayParameters();

    blockAllSignals(true);
    updateBillboardUi(displayParameters.m_billboard);
    updateEyeDomeUi(displayParameters.m_eyeDomeLighting);
    blockAllSignals(false);
}

void ToolBarRenderingAdvanced::updateBillboardUi(const BillboardRendering& billboardSettings)
{
    m_ui.checkBox_billboardEnable->setChecked(billboardSettings.enabled);
    double featherPercent = billboardSettings.feather * 100.0;
    m_ui.slider_billboardFeather->setValue(static_cast<int>(std::round(featherPercent)));
    m_ui.doubleSpinBox_billboardFeather->setValue(featherPercent);

    bool enableControls = billboardSettings.enabled;
    m_ui.slider_billboardFeather->setEnabled(enableControls);
    m_ui.doubleSpinBox_billboardFeather->setEnabled(enableControls);
}

void ToolBarRenderingAdvanced::updateEyeDomeUi(const EyeDomeLighting& edlSettings)
{
    m_ui.checkBox_edlEnable->setChecked(edlSettings.enabled);
    m_ui.slider_edlStrength->setValue(static_cast<int>(std::round(edlSettings.strength * 100.0f)));
    m_ui.doubleSpinBox_edlStrength->setValue(edlSettings.strength);

    m_ui.slider_edlRadius->setValue(static_cast<int>(std::round(edlSettings.radius * 10.0f)));
    m_ui.doubleSpinBox_edlRadius->setValue(edlSettings.radius);

    m_ui.slider_edlBias->setValue(static_cast<int>(std::round(edlSettings.bias * 100.0f)));
    m_ui.doubleSpinBox_edlBias->setValue(edlSettings.bias);

    bool enableControls = edlSettings.enabled;
    m_ui.slider_edlStrength->setEnabled(enableControls);
    m_ui.doubleSpinBox_edlStrength->setEnabled(enableControls);
    m_ui.slider_edlRadius->setEnabled(enableControls);
    m_ui.doubleSpinBox_edlRadius->setEnabled(enableControls);
    m_ui.slider_edlBias->setEnabled(enableControls);
    m_ui.doubleSpinBox_edlBias->setEnabled(enableControls);
}

void ToolBarRenderingAdvanced::blockAllSignals(bool block)
{
    m_ui.checkBox_billboardEnable->blockSignals(block);
    m_ui.slider_billboardFeather->blockSignals(block);
    m_ui.doubleSpinBox_billboardFeather->blockSignals(block);

    m_ui.checkBox_edlEnable->blockSignals(block);
    m_ui.slider_edlStrength->blockSignals(block);
    m_ui.doubleSpinBox_edlStrength->blockSignals(block);
    m_ui.slider_edlRadius->blockSignals(block);
    m_ui.doubleSpinBox_edlRadius->blockSignals(block);
    m_ui.slider_edlBias->blockSignals(block);
    m_ui.doubleSpinBox_edlBias->blockSignals(block);
}

BillboardRendering ToolBarRenderingAdvanced::getBillboardFromUi() const
{
    BillboardRendering settings = {};
    settings.enabled = m_ui.checkBox_billboardEnable->isChecked();
    settings.feather = static_cast<float>(m_ui.doubleSpinBox_billboardFeather->value() / 100.0);
    return settings;
}

EyeDomeLighting ToolBarRenderingAdvanced::getEdlFromUi() const
{
    EyeDomeLighting settings = {};
    settings.enabled = m_ui.checkBox_edlEnable->isChecked();
    settings.strength = static_cast<float>(m_ui.doubleSpinBox_edlStrength->value());
    settings.radius = static_cast<float>(m_ui.doubleSpinBox_edlRadius->value());
    settings.bias = static_cast<float>(m_ui.doubleSpinBox_edlBias->value());
    return settings;
}

void ToolBarRenderingAdvanced::sendBillboard()
{
    m_dataDispatcher.updateInformation(new GuiDataBillboardRendering(getBillboardFromUi(), m_focusCamera), this);
}

void ToolBarRenderingAdvanced::sendEdl()
{
    m_dataDispatcher.updateInformation(new GuiDataEyeDomeLighting(getEdlFromUi(), m_focusCamera), this);
}

void ToolBarRenderingAdvanced::slotBillboardToggled(int state)
{
    (void)state;
    bool enabled = m_ui.checkBox_billboardEnable->isChecked();
    m_ui.slider_billboardFeather->setEnabled(enabled);
    m_ui.doubleSpinBox_billboardFeather->setEnabled(enabled);
    sendBillboard();
}

void ToolBarRenderingAdvanced::slotBillboardFeatherSlider(int value)
{
    m_ui.doubleSpinBox_billboardFeather->setValue(value);
    if (m_ui.checkBox_billboardEnable->isChecked())
        sendBillboard();
}

void ToolBarRenderingAdvanced::slotBillboardFeatherSpin(double value)
{
    m_ui.slider_billboardFeather->setValue(static_cast<int>(std::round(value)));
    if (m_ui.checkBox_billboardEnable->isChecked())
        sendBillboard();
}

void ToolBarRenderingAdvanced::slotEdlToggled(int state)
{
    (void)state;
    bool enabled = m_ui.checkBox_edlEnable->isChecked();
    m_ui.slider_edlStrength->setEnabled(enabled);
    m_ui.doubleSpinBox_edlStrength->setEnabled(enabled);
    m_ui.slider_edlRadius->setEnabled(enabled);
    m_ui.doubleSpinBox_edlRadius->setEnabled(enabled);
    m_ui.slider_edlBias->setEnabled(enabled);
    m_ui.doubleSpinBox_edlBias->setEnabled(enabled);
    sendEdl();
}

void ToolBarRenderingAdvanced::slotEdlStrengthSlider(int value)
{
    m_ui.doubleSpinBox_edlStrength->setValue(value / 100.0);
    if (m_ui.checkBox_edlEnable->isChecked())
        sendEdl();
}

void ToolBarRenderingAdvanced::slotEdlStrengthSpin(double value)
{
    m_ui.slider_edlStrength->setValue(static_cast<int>(std::round(value * 100.0)));
    if (m_ui.checkBox_edlEnable->isChecked())
        sendEdl();
}

void ToolBarRenderingAdvanced::slotEdlRadiusSlider(int value)
{
    m_ui.doubleSpinBox_edlRadius->setValue(value / 10.0);
    if (m_ui.checkBox_edlEnable->isChecked())
        sendEdl();
}

void ToolBarRenderingAdvanced::slotEdlRadiusSpin(double value)
{
    m_ui.slider_edlRadius->setValue(static_cast<int>(std::round(value * 10.0)));
    if (m_ui.checkBox_edlEnable->isChecked())
        sendEdl();
}

void ToolBarRenderingAdvanced::slotEdlBiasSlider(int value)
{
    m_ui.doubleSpinBox_edlBias->setValue(value / 100.0);
    if (m_ui.checkBox_edlEnable->isChecked())
        sendEdl();
}

void ToolBarRenderingAdvanced::slotEdlBiasSpin(double value)
{
    m_ui.slider_edlBias->setValue(static_cast<int>(std::round(value * 100.0)));
    if (m_ui.checkBox_edlEnable->isChecked())
        sendEdl();
}
