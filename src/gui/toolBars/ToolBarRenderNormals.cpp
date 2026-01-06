#include "gui/toolBars/ToolBarRenderNormals.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataGeneralProject.h"

#include "models/graph/CameraNode.h"
#include <cmath>

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
	m_ui.checkBox_ao->setEnabled(true);
	m_ui.slider_aoSize->setEnabled(false);
	m_ui.doubleSpinBox_aoSize->setEnabled(false);
	m_ui.slider_aoIntensity->setEnabled(false);
	m_ui.doubleSpinBox_aoIntensity->setEnabled(false);

	// Connect widgets
	connect(m_ui.normals_checkBox, &QCheckBox::stateChanged, this, &ToolBarRenderNormals::slotNormalsChanged);
	connect(m_ui.checkBox_blendColor, &QCheckBox::stateChanged, this, &ToolBarRenderNormals::slotNormalsChanged);
	connect(m_ui.checkBox_ao, &QCheckBox::stateChanged, this, &ToolBarRenderNormals::slotAOChanged);

	connect(m_ui.slider_normalStrength, &QSlider::valueChanged, m_ui.spinBox_normalStrength, &QSpinBox::setValue);
	connect(m_ui.slider_normalStrength, &QSlider::valueChanged, this, &ToolBarRenderNormals::slotNormalsChanged);
	connect(m_ui.spinBox_normalStrength, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.slider_normalStrength, &QSlider::setValue);

	connect(m_ui.doubleSpinBox_sharpness, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolBarRenderNormals::slotSharpnessChanged);

	connect(m_ui.slider_aoSize, &QSlider::valueChanged, this, [this](int value) {
		m_ui.doubleSpinBox_aoSize->setValue(value / 100.0);
		slotAOChanged();
	});
	connect(m_ui.doubleSpinBox_aoSize, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
		m_ui.slider_aoSize->setValue(static_cast<int>(std::round(value * 100.0)));
		slotAOChanged();
	});

	connect(m_ui.slider_aoIntensity, &QSlider::valueChanged, this, [this](int value) {
		m_ui.doubleSpinBox_aoIntensity->setValue(value / 100.0);
		slotAOChanged();
	});
	connect(m_ui.doubleSpinBox_aoIntensity, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
		m_ui.slider_aoIntensity->setValue(static_cast<int>(std::round(value * 100.0)));
		slotAOChanged();
	});

	m_ui.spinBox_normalStrength->setValue(50);
	m_ui.doubleSpinBox_sharpness->setValue(1.0);
	m_ui.slider_aoSize->setValue(80);
	m_ui.doubleSpinBox_aoSize->setValue(0.8);
	m_ui.slider_aoIntensity->setValue(120);
	m_ui.doubleSpinBox_aoIntensity->setValue(1.2);

	// GuiData link
	registerGuiDataFunction(guiDType::projectLoaded, &ToolBarRenderNormals::onProjectLoad);
	registerGuiDataFunction(guiDType::renderActiveCamera, &ToolBarRenderNormals::onActiveCamera);
	registerGuiDataFunction(guiDType::focusViewport, &ToolBarRenderNormals::onFocusViewport);
	registerGuiDataFunction(guiDType::renderTransparency, &ToolBarRenderNormals::onTransparencyChanged);
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
	applyTransparencyLock(displayParameters.m_blendMode == BlendMode::Transparent);
	updateNormals(displayParameters.m_postRenderingNormals);
	updateAmbientOcclusion(displayParameters.m_ambientOcclusion);
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
	applyAOEnabledState();
}

void ToolBarRenderNormals::updateAmbientOcclusion(const AmbientOcclusion& aoParams)
{
	blockAllSignals(true);
	m_ui.checkBox_ao->setChecked(aoParams.enabled && !m_transparencyActive);
	m_ui.slider_aoSize->setValue(static_cast<int>(std::round(aoParams.radius * 100.f)));
	m_ui.doubleSpinBox_aoSize->setValue(aoParams.radius);
	m_ui.slider_aoIntensity->setValue(static_cast<int>(std::round(aoParams.intensity * 100.f)));
	m_ui.doubleSpinBox_aoIntensity->setValue(aoParams.intensity);
	blockAllSignals(false);
	applyAOEnabledState();
}

void ToolBarRenderNormals::blockAllSignals(bool block)
{
	m_ui.normals_checkBox->blockSignals(block);
	m_ui.slider_normalStrength->blockSignals(block);
	m_ui.spinBox_normalStrength->blockSignals(block);
	m_ui.doubleSpinBox_sharpness->blockSignals(block);
	m_ui.checkBox_blendColor->blockSignals(block);
	m_ui.checkBox_ao->blockSignals(block);
	m_ui.slider_aoSize->blockSignals(block);
	m_ui.doubleSpinBox_aoSize->blockSignals(block);
	m_ui.slider_aoIntensity->blockSignals(block);
	m_ui.doubleSpinBox_aoIntensity->blockSignals(block);
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

void ToolBarRenderNormals::slotAOChanged()
{
	AmbientOcclusion ao = {};
	ao.enabled = m_ui.checkBox_ao->isChecked() && !m_transparencyActive;
	ao.radius = static_cast<float>(m_ui.doubleSpinBox_aoSize->value());
	ao.intensity = static_cast<float>(m_ui.doubleSpinBox_aoIntensity->value());
	ao.useTopLight = true;
	ao.topLightStrength = 1.0f;

	applyAOEnabledState();
	m_dataDispatcher.updateInformation(new GuiDataAmbientOcclusion(ao, m_focusCamera), this);
}

void ToolBarRenderNormals::applyAOEnabledState()
{
	const bool aoControlsEnabled = m_ui.checkBox_ao->isChecked() && !m_transparencyActive;
	m_ui.checkBox_ao->setEnabled(!m_transparencyActive);
	m_ui.slider_aoSize->setEnabled(aoControlsEnabled);
	m_ui.doubleSpinBox_aoSize->setEnabled(aoControlsEnabled);
	m_ui.slider_aoIntensity->setEnabled(aoControlsEnabled);
	m_ui.doubleSpinBox_aoIntensity->setEnabled(aoControlsEnabled);
}

void ToolBarRenderNormals::applyTransparencyLock(bool transparent)
{
	const bool wasTransparent = m_transparencyActive;
	m_transparencyActive = transparent;

	if (transparent)
	{
		const bool wasChecked = m_ui.checkBox_ao->isChecked();
		blockAllSignals(true);
		m_ui.checkBox_ao->setChecked(false);
		blockAllSignals(false);
		applyAOEnabledState();
		if (!wasTransparent && wasChecked)
			slotAOChanged();
	}
	else
	{
		applyAOEnabledState();
	}
}

void ToolBarRenderNormals::onTransparencyChanged(IGuiData* data)
{
	auto transparencyData = static_cast<GuiDataRenderTransparency*>(data);
	if (m_focusCamera && transparencyData->m_camera && m_focusCamera != transparencyData->m_camera)
		return;

	applyTransparencyLock(transparencyData->m_mode == BlendMode::Transparent);
}
