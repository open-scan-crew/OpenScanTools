#include "gui/toolBars/ToolBarRenderEnhance.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"

#include "models/graph/CameraNode.h"

#include <algorithm>
#include <cmath>

namespace
{
	constexpr float DEPTH_LINING_STRENGTH_UI_SCALE = 1.5f;
	constexpr float DEPTH_LINING_SENSITIVITY_UI_SCALE = 3.0f;
}

ToolBarRenderEnhance::ToolBarRenderEnhance(IDataDispatcher& dataDispatcher, QWidget* parent, float guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
	, m_focusCamera()
{
	m_ui.setupUi(this);
	setEnabled(false);
	(void)guiScale;

	populateEdgeAwareResolutionCombo();

	connect(m_ui.checkBox_edgeAwareBlur, &QCheckBox::stateChanged, this, &ToolBarRenderEnhance::slotEdgeAwareBlurToggled);
	connect(m_ui.slider_edgeAwareRadius, &QSlider::valueChanged, m_ui.spinBox_edgeAwareRadius, &QSpinBox::setValue);
	connect(m_ui.spinBox_edgeAwareRadius, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.slider_edgeAwareRadius, &QSlider::setValue);
	connect(m_ui.slider_edgeAwareRadius, &QSlider::valueChanged, this, &ToolBarRenderEnhance::slotEdgeAwareBlurValueChanged);

	connect(m_ui.slider_edgeAwareDepth, &QSlider::valueChanged, m_ui.spinBox_edgeAwareDepth, &QSpinBox::setValue);
	connect(m_ui.spinBox_edgeAwareDepth, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.slider_edgeAwareDepth, &QSlider::setValue);
	connect(m_ui.slider_edgeAwareDepth, &QSlider::valueChanged, this, &ToolBarRenderEnhance::slotEdgeAwareBlurValueChanged);

	connect(m_ui.slider_edgeAwareBlend, &QSlider::valueChanged, m_ui.spinBox_edgeAwareBlend, &QSpinBox::setValue);
	connect(m_ui.spinBox_edgeAwareBlend, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.slider_edgeAwareBlend, &QSlider::setValue);
	connect(m_ui.slider_edgeAwareBlend, &QSlider::valueChanged, this, &ToolBarRenderEnhance::slotEdgeAwareBlurValueChanged);

	connect(m_ui.comboBox_edgeAwareResolution, qOverload<int>(&QComboBox::currentIndexChanged), this, &ToolBarRenderEnhance::slotEdgeAwareBlurResolutionChanged);

	connect(m_ui.checkBox_depthLining, &QCheckBox::stateChanged, this, &ToolBarRenderEnhance::slotDepthLiningToggled);
	connect(m_ui.slider_depthLiningStrength, &QSlider::valueChanged, m_ui.spinBox_depthLiningStrength, &QSpinBox::setValue);
	connect(m_ui.spinBox_depthLiningStrength, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.slider_depthLiningStrength, &QSlider::setValue);
	connect(m_ui.slider_depthLiningStrength, &QSlider::valueChanged, this, &ToolBarRenderEnhance::slotDepthLiningValueChanged);
	connect(m_ui.slider_depthLiningSensitivity, &QSlider::valueChanged, m_ui.spinBox_depthLiningSensitivity, &QSpinBox::setValue);
	connect(m_ui.spinBox_depthLiningSensitivity, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.slider_depthLiningSensitivity, &QSlider::setValue);
	connect(m_ui.slider_depthLiningSensitivity, &QSlider::valueChanged, this, &ToolBarRenderEnhance::slotDepthLiningSensitivityChanged);
	connect(m_ui.checkBox_depthLiningStrongMode, &QCheckBox::stateChanged, this, &ToolBarRenderEnhance::slotDepthLiningStrongModeToggled);

	updateEdgeAwareBlurUi(m_ui.checkBox_edgeAwareBlur->isChecked());
	updateDepthLiningUi(m_ui.checkBox_depthLining->isChecked());

	registerGuiDataFunction(guiDType::projectLoaded, &ToolBarRenderEnhance::onProjectLoad);
	registerGuiDataFunction(guiDType::renderActiveCamera, &ToolBarRenderEnhance::onActiveCamera);
	registerGuiDataFunction(guiDType::focusViewport, &ToolBarRenderEnhance::onFocusViewport);
}

void ToolBarRenderEnhance::informData(IGuiData* data)
{
	auto it = m_methods.find(data->getType());
	if (it != m_methods.end())
	{
		GuiDataFunction method = it->second;
		(this->*method)(data);
	}
}

void ToolBarRenderEnhance::onProjectLoad(IGuiData* data)
{
	GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(plData->m_isProjectLoad);
}

void ToolBarRenderEnhance::onActiveCamera(IGuiData* data)
{
	auto infos = static_cast<GuiDataCameraInfo*>(data);
	if (infos->m_camera && m_focusCamera != infos->m_camera)
		return;

	ReadPtr<CameraNode> rCam = m_focusCamera.cget();
	if (!rCam)
		return;
	const DisplayParameters& displayParameters = rCam->getDisplayParameters();

	blockAllSignals(true);

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

void ToolBarRenderEnhance::onFocusViewport(IGuiData* data)
{
	GuiDataFocusViewport* castData = static_cast<GuiDataFocusViewport*>(data);
	if (castData->m_forceFocus && castData->m_camera)
		m_focusCamera = castData->m_camera;
}

void ToolBarRenderEnhance::blockAllSignals(bool block)
{
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

void ToolBarRenderEnhance::populateEdgeAwareResolutionCombo()
{
	m_ui.comboBox_edgeAwareResolution->clear();
	m_ui.comboBox_edgeAwareResolution->addItem(tr("Full res"));
	m_ui.comboBox_edgeAwareResolution->addItem(tr("Half res"));
	m_ui.comboBox_edgeAwareResolution->addItem(tr("Quarter res"));
}

void ToolBarRenderEnhance::updateEdgeAwareBlurUi(bool enabled)
{
	m_ui.slider_edgeAwareRadius->setEnabled(enabled);
	m_ui.spinBox_edgeAwareRadius->setEnabled(enabled);
	m_ui.slider_edgeAwareDepth->setEnabled(enabled);
	m_ui.spinBox_edgeAwareDepth->setEnabled(enabled);
	m_ui.slider_edgeAwareBlend->setEnabled(enabled);
	m_ui.spinBox_edgeAwareBlend->setEnabled(enabled);
	m_ui.comboBox_edgeAwareResolution->setEnabled(enabled);
}

EdgeAwareBlur ToolBarRenderEnhance::getEdgeAwareBlurFromUi() const
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

void ToolBarRenderEnhance::updateDepthLiningUi(bool enabled)
{
	m_ui.slider_depthLiningStrength->setEnabled(enabled);
	m_ui.spinBox_depthLiningStrength->setEnabled(enabled);
	m_ui.slider_depthLiningSensitivity->setEnabled(enabled);
	m_ui.spinBox_depthLiningSensitivity->setEnabled(enabled);
	m_ui.checkBox_depthLiningStrongMode->setEnabled(enabled);
}

DepthLining ToolBarRenderEnhance::getDepthLiningFromUi() const
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

void ToolBarRenderEnhance::slotEdgeAwareBlurToggled(int state)
{
	updateEdgeAwareBlurUi(state == Qt::Checked);
	m_dataDispatcher.updateInformation(new GuiDataEdgeAwareBlur(getEdgeAwareBlurFromUi(), m_focusCamera), this);
}

void ToolBarRenderEnhance::slotEdgeAwareBlurValueChanged(int value)
{
	(void)value;
	if (!m_ui.checkBox_edgeAwareBlur->isChecked())
		return;

	m_dataDispatcher.updateInformation(new GuiDataEdgeAwareBlur(getEdgeAwareBlurFromUi(), m_focusCamera), this);
}

void ToolBarRenderEnhance::slotEdgeAwareBlurResolutionChanged(int index)
{
	(void)index;
	if (!m_ui.checkBox_edgeAwareBlur->isChecked())
		return;

	m_dataDispatcher.updateInformation(new GuiDataEdgeAwareBlur(getEdgeAwareBlurFromUi(), m_focusCamera), this);
}

void ToolBarRenderEnhance::slotDepthLiningToggled(int state)
{
	updateDepthLiningUi(state == Qt::Checked);
	m_dataDispatcher.updateInformation(new GuiDataDepthLining(getDepthLiningFromUi(), m_focusCamera), this);
}

void ToolBarRenderEnhance::slotDepthLiningValueChanged(int value)
{
	(void)value;
	if (!m_ui.checkBox_depthLining->isChecked())
		return;

	m_dataDispatcher.updateInformation(new GuiDataDepthLining(getDepthLiningFromUi(), m_focusCamera), this);
}

void ToolBarRenderEnhance::slotDepthLiningSensitivityChanged(int value)
{
	(void)value;
	if (!m_ui.checkBox_depthLining->isChecked())
		return;

	m_dataDispatcher.updateInformation(new GuiDataDepthLining(getDepthLiningFromUi(), m_focusCamera), this);
}

void ToolBarRenderEnhance::slotDepthLiningStrongModeToggled(int state)
{
	(void)state;
	if (!m_ui.checkBox_depthLining->isChecked())
	{
		updateDepthLiningUi(false);
		return;
	}

	m_dataDispatcher.updateInformation(new GuiDataDepthLining(getDepthLiningFromUi(), m_focusCamera), this);
}
