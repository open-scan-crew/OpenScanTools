#include "gui/toolBars/ToolBarRenderNormals.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataGeneralProject.h"

#include "models/graph/CameraNode.h"

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
	m_ui.checkBox_ao->setChecked(false);
	m_ui.aoSizeSlider->setEnabled(false);
	m_ui.aoSizeSpinBox->setEnabled(false);
	m_ui.aoIntensitySlider->setEnabled(false);
	m_ui.aoIntensitySpinBox->setEnabled(false);

	// Connect widgets
	connect(m_ui.normals_checkBox, &QCheckBox::stateChanged, this, &ToolBarRenderNormals::slotNormalsChanged);
	connect(m_ui.checkBox_blendColor, &QCheckBox::stateChanged, this, &ToolBarRenderNormals::slotNormalsChanged);
	connect(m_ui.checkBox_ao, &QCheckBox::stateChanged, this, &ToolBarRenderNormals::slotSsaoChanged);

	connect(m_ui.slider_normalStrength, &QSlider::valueChanged, m_ui.spinBox_normalStrength, &QSpinBox::setValue);
	connect(m_ui.slider_normalStrength, &QSlider::valueChanged, this, &ToolBarRenderNormals::slotNormalsChanged);
	connect(m_ui.spinBox_normalStrength, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_ui.slider_normalStrength, &QSlider::setValue);

	connect(m_ui.doubleSpinBox_sharpness, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolBarRenderNormals::slotSharpnessChanged);
	connect(m_ui.aoSizeSlider, &QSlider::valueChanged, this, &ToolBarRenderNormals::slotSsaoSizeSliderChanged);
	connect(m_ui.aoIntensitySlider, &QSlider::valueChanged, this, &ToolBarRenderNormals::slotSsaoIntensitySliderChanged);
	connect(m_ui.aoSizeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolBarRenderNormals::slotSsaoSizeSpinBoxChanged);
	connect(m_ui.aoIntensitySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ToolBarRenderNormals::slotSsaoIntensitySpinBoxChanged);

	blockAllSignals(true);
	m_ui.spinBox_normalStrength->setValue(50);
	m_ui.doubleSpinBox_sharpness->setValue(1.0);
	m_ui.aoSizeSlider->setValue(50);
	m_ui.aoSizeSpinBox->setValue(0.5);
	m_ui.aoIntensitySlider->setValue(60);
	m_ui.aoIntensitySpinBox->setValue(0.6);
	blockAllSignals(false);

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
	updateSSAO(displayParameters.m_postRenderingSSAO, displayParameters.m_blendMode);
}

void ToolBarRenderNormals::onRenderTransparency(IGuiData* data)
{
	auto transparencyData = static_cast<GuiDataRenderTransparency*>(data);
	m_blendMode = transparencyData->m_mode;
	updateSsaoUiState(m_blendMode != BlendMode::Opaque);
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

void ToolBarRenderNormals::updateSSAO(const PostRenderingSSAO& ssaoParams, BlendMode blendMode)
{
	blockAllSignals(true);
	m_ssaoParams = ssaoParams;
	m_blendMode = blendMode;

	m_ui.checkBox_ao->setChecked(ssaoParams.enabled);

	int sizeValue = (int)std::round(ssaoParams.size * 100.f);
	int intensityValue = (int)std::round(ssaoParams.intensity * 100.f);
	m_ui.aoSizeSlider->setValue(sizeValue);
	m_ui.aoSizeSpinBox->setValue(ssaoParams.size);
	m_ui.aoIntensitySlider->setValue(intensityValue);
	m_ui.aoIntensitySpinBox->setValue(ssaoParams.intensity);

	updateSsaoUiState(m_blendMode != BlendMode::Opaque);

	blockAllSignals(false);
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

void ToolBarRenderNormals::updateSsaoUiState(bool transparencyActive)
{
	bool enable = !transparencyActive;
	m_ui.checkBox_ao->setEnabled(enable);
	bool controlsEnabled = enable && m_ui.checkBox_ao->isChecked();
	m_ui.aoSizeSlider->setEnabled(controlsEnabled);
	m_ui.aoSizeSpinBox->setEnabled(controlsEnabled);
	m_ui.aoIntensitySlider->setEnabled(controlsEnabled);
	m_ui.aoIntensitySpinBox->setEnabled(controlsEnabled);
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

void ToolBarRenderNormals::slotSsaoChanged()
{
	if (!m_ui.checkBox_ao->isEnabled())
		return;

	PostRenderingSSAO ssao = m_ssaoParams;
	ssao.enabled = m_ui.checkBox_ao->isChecked();
	ssao.size = m_ui.aoSizeSpinBox->value();
	ssao.intensity = m_ui.aoIntensitySpinBox->value();
	m_ssaoParams = ssao;

	updateSsaoUiState(m_blendMode != BlendMode::Opaque);
	m_dataDispatcher.updateInformation(new GuiDataPostRenderingSSAO(ssao, m_focusCamera), this);
}

void ToolBarRenderNormals::slotSsaoSizeSliderChanged(int value)
{
	m_ui.aoSizeSpinBox->blockSignals(true);
	m_ui.aoSizeSpinBox->setValue(value / 100.0);
	m_ui.aoSizeSpinBox->blockSignals(false);
	slotSsaoChanged();
}

void ToolBarRenderNormals::slotSsaoIntensitySliderChanged(int value)
{
	m_ui.aoIntensitySpinBox->blockSignals(true);
	m_ui.aoIntensitySpinBox->setValue(value / 100.0);
	m_ui.aoIntensitySpinBox->blockSignals(false);
	slotSsaoChanged();
}

void ToolBarRenderNormals::slotSsaoSizeSpinBoxChanged(double value)
{
	m_ui.aoSizeSlider->blockSignals(true);
	m_ui.aoSizeSlider->setValue((int)std::round(value * 100.0));
	m_ui.aoSizeSlider->blockSignals(false);
	slotSsaoChanged();
}

void ToolBarRenderNormals::slotSsaoIntensitySpinBoxChanged(double value)
{
	m_ui.aoIntensitySlider->blockSignals(true);
	m_ui.aoIntensitySlider->setValue((int)std::round(value * 100.0));
	m_ui.aoIntensitySlider->blockSignals(false);
	slotSsaoChanged();
}
