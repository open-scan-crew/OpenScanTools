#include "gui/toolBars/ToolBarRenderNormals.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataGeneralProject.h"

#include "models/graph/CameraNode.h"

#include <algorithm>
#include <cmath>

namespace
{
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

ToolBarRenderNormals::ToolBarRenderNormals(IDataDispatcher& dataDispatcher, QWidget* parent, float guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
	, m_focusCamera()
{
	m_ui.setupUi(this);
	setEnabled(false);
	m_ui.normals_checkBox->setChecked(true);
	m_ui.spinBox_normalStrength->setEnabled(false);
	m_ui.slider_normalStrength->setEnabled(false);
	m_ui.doubleSpinBox_sharpness->setEnabled(false);
	m_ui.checkBox_blendColor->setEnabled(true);
	m_ui.aoSizeSlider->setRange(0, kAoSliderMax);
	m_ui.aoIntensitySlider->setRange(0, kAoSliderMax);

	// Connect widgets
	connect(m_ui.normals_checkBox, &QCheckBox::stateChanged, this, &ToolBarRenderNormals::slotNormalsChanged);
	connect(m_ui.checkBox_blendColor, &QCheckBox::stateChanged, this, &ToolBarRenderNormals::slotNormalsChanged);
	connect(m_ui.checkBox_ao, &QCheckBox::stateChanged, this, &ToolBarRenderNormals::slotAmbientOcclusionChanged);

	connect(m_ui.slider_normalStrength, &QSlider::valueChanged, m_ui.spinBox_normalStrength, &QSpinBox::setValue);
	connect(m_ui.slider_normalStrength, &QSlider::valueChanged, this, &ToolBarRenderNormals::slotNormalsChanged);
	connect(m_ui.spinBox_normalStrength, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.slider_normalStrength, &QSlider::setValue);

	connect(m_ui.doubleSpinBox_sharpness, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolBarRenderNormals::slotSharpnessChanged);

	connect(m_ui.aoSizeSlider, &QSlider::valueChanged, this, &ToolBarRenderNormals::slotAmbientOcclusionChanged);
	connect(m_ui.aoIntensitySlider, &QSlider::valueChanged, this, &ToolBarRenderNormals::slotAmbientOcclusionChanged);
	connect(m_ui.aoSizeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolBarRenderNormals::slotAmbientOcclusionChanged);
	connect(m_ui.aoIntensitySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolBarRenderNormals::slotAmbientOcclusionChanged);

	m_ui.spinBox_normalStrength->setValue(50);
	m_ui.doubleSpinBox_sharpness->setValue(1.0);
	m_ui.aoSizeSpinBox->setValue(toNormalizedAoSize(16.0f));
	m_ui.aoIntensitySpinBox->setValue(0.5);
	m_ui.aoSizeSlider->setValue(static_cast<int>(std::round(m_ui.aoSizeSpinBox->value() * kAoSliderMax)));
	m_ui.aoIntensitySlider->setValue(static_cast<int>(std::round(m_ui.aoIntensitySpinBox->value() * kAoSliderMax)));
	updateAmbientOcclusionUi(m_ui.checkBox_ao->isChecked());

	// GuiData link
	registerGuiDataFunction(guiDType::projectLoaded, &ToolBarRenderNormals::onProjectLoad);
	registerGuiDataFunction(guiDType::renderActiveCamera, &ToolBarRenderNormals::onActiveCamera);
	registerGuiDataFunction(guiDType::focusViewport, &ToolBarRenderNormals::onFocusViewport);
	registerGuiDataFunction(guiDType::renderTransparency, &ToolBarRenderNormals::onRenderTransparency);
	//registerGuiDataFunction(guiDType::renderPostRenderingNormals, &ToolBarRenderNormals::onRenderNormals);
}

void ToolBarRenderNormals::informData(IGuiData* data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		GuiDataFunction method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarRenderNormals::onProjectLoad(IGuiData* data)
{
	GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(plData->m_isProjectLoad);
}

void ToolBarRenderNormals::onFocusViewport(IGuiData* data)
{
	GuiDataFocusViewport* castData = static_cast<GuiDataFocusViewport*>(data);
	if (castData->m_forceFocus && castData->m_camera)
		m_focusCamera = castData->m_camera;
}

void ToolBarRenderNormals::onActiveCamera(IGuiData* data)
{
	auto infos = static_cast<GuiDataCameraInfo*>(data);
	if (infos->m_camera && m_focusCamera != infos->m_camera)
		return;

	ReadPtr<CameraNode> rCam = m_focusCamera.cget();
	if (!rCam)
		return;
	const DisplayParameters& displayParameters = rCam->getDisplayParameters();
	updateNormals(displayParameters.m_postRenderingNormals);
	updateAmbientOcclusion(displayParameters.m_postRenderingAmbientOcclusion);
	m_transparencyActive = (displayParameters.m_blendMode == BlendMode::Transparent);
	updateAmbientOcclusionUi(m_ui.checkBox_ao->isChecked());
}

void ToolBarRenderNormals::updateNormals(const PostRenderingNormals& normalsParams)
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

	// ambiant value
	int newNormalValue = (int)std::round(normalsParams.normalStrength * 100.f);

	m_ui.slider_normalStrength->setEnabled(normalState != Qt::CheckState::Unchecked);
	m_ui.slider_normalStrength->setValue(newNormalValue);
	m_ui.spinBox_normalStrength->setEnabled(normalState != Qt::CheckState::Unchecked);
	m_ui.spinBox_normalStrength->setValue(newNormalValue);

	m_ui.doubleSpinBox_sharpness->setEnabled(normalState != Qt::CheckState::Unchecked);
	m_ui.checkBox_blendColor->setEnabled(normalState != Qt::CheckState::Unchecked);

	// gloss
	m_ui.doubleSpinBox_sharpness->setValue(normalsParams.gloss);

	// blend
	m_ui.checkBox_blendColor->setChecked(normalsParams.blendColor);

	blockAllSignals(false);
}

void ToolBarRenderNormals::updateAmbientOcclusion(const PostRenderingAmbientOcclusion& aoParams)
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

void ToolBarRenderNormals::updateAmbientOcclusionUi(bool aoEnabled)
{
	const bool enableControls = aoEnabled && !m_transparencyActive;
	m_ui.checkBox_ao->setEnabled(!m_transparencyActive);
	m_ui.aoSizeSlider->setEnabled(enableControls);
	m_ui.aoSizeSpinBox->setEnabled(enableControls);
	m_ui.aoIntensitySlider->setEnabled(enableControls);
	m_ui.aoIntensitySpinBox->setEnabled(enableControls);
}

void ToolBarRenderNormals::blockAllSignals(bool block)
{
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
}

void ToolBarRenderNormals::slotNormalsChanged()
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

void ToolBarRenderNormals::slotSharpnessChanged(double value)
{
	slotNormalsChanged();
}

void ToolBarRenderNormals::slotAmbientOcclusionChanged()
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

void ToolBarRenderNormals::onRenderTransparency(IGuiData* data)
{
	auto transparencyData = static_cast<GuiDataRenderTransparency*>(data);
	m_transparencyActive = (transparencyData->m_mode == BlendMode::Transparent);
	updateAmbientOcclusionUi(m_ui.checkBox_ao->isChecked());
}
