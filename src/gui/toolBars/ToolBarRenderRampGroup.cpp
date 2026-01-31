#include "gui/toolBars/ToolBarRenderRampGroup.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "controller/controls/ControlTemperatureScale.h"
#include "controller/controls/ControlPicking.h"

#include <QFileDialog>

#include "models/graph/CameraNode.h"

ToolBarRenderRampGroup::ToolBarRenderRampGroup(IDataDispatcher& dataDispatcher, QWidget* parent, float guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
{
	m_ui.setupUi(this);
	setEnabled(false);
	m_ui.checkBox_showScale->setChecked(true);
	m_ui.checkBox_showTemperatureScale->setEnabled(false);
	m_ui.pushButton_pickTemperature->setEnabled(false);
	m_ui.lineEdit_temperatureScale->setPlaceholderText(tr("No file found"));

	// Connect widgets
	connect(m_ui.checkBox_showScale, &QCheckBox::stateChanged, [this]() { this->sendGuiData(); });
	connect(m_ui.checkBox_showTemperatureScale, &QCheckBox::stateChanged, [this]() { this->sendGuiData(); });
	connect(m_ui.checkBox_centerBoxScale, &QCheckBox::stateChanged, [this]() { this->sendGuiData(); });
	connect(m_ui.spinBox_graduation, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this](int) { this->sendGuiData(); });
	connect(m_ui.importTempScaleButton, &QPushButton::clicked, [this]()
		{
			QString fileName = QFileDialog::getOpenFileName(this, tr("Import Temperature Scale"), QString(), tr("Text files (*.txt);;All files (*)"));
			if (!fileName.isEmpty())
			{
				m_dataDispatcher.sendControl(new control::temperatureScale::ImportTemperatureScale(std::filesystem::path(fileName.toStdWString())));
			}
		});
	connect(m_ui.pushButton_pickTemperature, &QPushButton::clicked, [this]()
		{
			m_dataDispatcher.sendControl(new control::picking::PickTemperatureFromPick());
		});

	// GuiData link
	registerGuiDataFunction(guiDType::projectLoaded, &ToolBarRenderRampGroup::onProjectLoad);
	registerGuiDataFunction(guiDType::temperatureScaleInfo, &ToolBarRenderRampGroup::updateTemperatureScaleStatus);
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
	m_ui.checkBox_showTemperatureScale->setChecked(rampScaleParams.showTemperatureScale);

	blockAllSignals(false);
}

void ToolBarRenderRampGroup::updateTemperatureScaleStatus(IGuiData* data)
{
	GuiDataTemperatureScaleInfo* info = static_cast<GuiDataTemperatureScaleInfo*>(data);
	m_temperatureScalePath = info->m_filePath;
	m_temperatureScaleValid = info->m_isValid;
	m_temperatureScaleFileFound = info->m_fileFound;

	const bool canUseTemperatureScale = m_temperatureScaleValid && m_temperatureScaleFileFound;
	m_ui.checkBox_showTemperatureScale->setEnabled(canUseTemperatureScale);
	m_ui.pushButton_pickTemperature->setEnabled(canUseTemperatureScale);

	if (m_temperatureScalePath.empty())
	{
		m_ui.lineEdit_temperatureScale->setText(tr("No file found"));
		return;
	}

	if (!m_temperatureScaleFileFound)
	{
		m_ui.lineEdit_temperatureScale->setText(tr("No file found"));
		return;
	}

	m_ui.lineEdit_temperatureScale->setText(QString::fromStdWString(m_temperatureScalePath.filename().wstring()));
}

void ToolBarRenderRampGroup::blockAllSignals(bool block)
{
	m_ui.checkBox_showScale->blockSignals(block);
	m_ui.checkBox_showTemperatureScale->blockSignals(block);
	m_ui.checkBox_centerBoxScale->blockSignals(block);
	m_ui.spinBox_graduation->blockSignals(block);
}

void ToolBarRenderRampGroup::sendGuiData()
{
    RampScale rs = { m_ui.checkBox_showScale->isChecked(),
                     m_ui.checkBox_centerBoxScale->isChecked(),
                     m_ui.spinBox_graduation->value(),
                     m_ui.checkBox_showTemperatureScale->isChecked() };
    m_dataDispatcher.updateInformation(new GuiDataRampScale(rs, m_focusCamera), this);
}
