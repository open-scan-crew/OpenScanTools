#include "gui/toolBars/ToolBarRenderRampGroup.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataGeneralProject.h"

#include "models/3d/Graph/CameraNode.h"

ToolBarRenderRampGroup::ToolBarRenderRampGroup(IDataDispatcher& dataDispatcher, QWidget* parent, float guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
{
	m_ui.setupUi(this);
	setEnabled(false);
	m_ui.checkBox_showScale->setChecked(true);

	// Connect widgets
	connect(m_ui.checkBox_showScale, &QCheckBox::stateChanged, [this]() { this->sendGuiData(); });
	connect(m_ui.checkBox_centerBoxScale, &QCheckBox::stateChanged, [this]() { this->sendGuiData(); });
	connect(m_ui.spinBox_graduation, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this](int) { this->sendGuiData(); });

	// GuiData link
	registerGuiDataFunction(guiDType::projectLoaded, &ToolBarRenderRampGroup::onProjectLoad);
	registerGuiDataFunction(guiDType::renderActiveCamera, &ToolBarRenderRampGroup::onActiveCamera);
	registerGuiDataFunction(guiDType::focusViewport, &ToolBarRenderRampGroup::onFocusViewport);
}

void ToolBarRenderRampGroup::informData(IGuiData* data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		GuiDataFunction method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarRenderRampGroup::onProjectLoad(IGuiData* data)
{
	GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(plData->m_isProjectLoad);
}

void ToolBarRenderRampGroup::onFocusViewport(IGuiData* data)
{
	GuiDataFocusViewport* castData = static_cast<GuiDataFocusViewport*>(data);
	if (castData->m_forceFocus && castData->m_camera)
		m_focusCamera = castData->m_camera;
}

void ToolBarRenderRampGroup::onActiveCamera(IGuiData* data)
{
	auto infos = static_cast<GuiDataCameraInfo*>(data);
	if (infos->m_camera && m_focusCamera != infos->m_camera)
		return;

	ReadPtr<CameraNode> rCam = m_focusCamera.cget();
	if (!rCam)
		return;
	const DisplayParameters& displayParameters = rCam->getDisplayParameters();
	updateRamp(displayParameters.m_rampScale);
}

void ToolBarRenderRampGroup::updateRamp(const RampScale& rampScaleParams)
{
	blockAllSignals(true);

	m_ui.checkBox_showScale->setChecked(rampScaleParams.showScale);
	m_ui.checkBox_centerBoxScale->setChecked(rampScaleParams.centerBoxScale);
	m_ui.spinBox_graduation->setValue(rampScaleParams.graduationCount);

	blockAllSignals(false);
}

void ToolBarRenderRampGroup::blockAllSignals(bool block)
{
	m_ui.checkBox_showScale->blockSignals(block);
	m_ui.checkBox_centerBoxScale->blockSignals(block);
	m_ui.spinBox_graduation->blockSignals(block);
}

void ToolBarRenderRampGroup::sendGuiData()
{
    RampScale rs = { m_ui.checkBox_showScale->isChecked(),
                     m_ui.checkBox_centerBoxScale->isChecked(),
                     m_ui.spinBox_graduation->value() };
    m_dataDispatcher.updateInformation(new GuiDataRampScale(rs, m_focusCamera), this);
}
