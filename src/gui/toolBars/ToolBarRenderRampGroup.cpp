#include "gui/toolBars/ToolBarRenderRampGroup.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataGeneralProject.h"

#include "controller/controls/ControlApplication.h"
#include "controller/controls/ControlPicking.h"
#include "models/graph/CameraNode.h"
#include "utils/TemperatureScaleParser.h"

#include <QFileDialog>

ToolBarRenderRampGroup::ToolBarRenderRampGroup(IDataDispatcher& dataDispatcher, QWidget* parent, float guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
{
	m_ui.setupUi(this);
	setEnabled(false);
	m_ui.checkBox_showScale->setChecked(true);
	m_ui.checkBox_showTemperatureScale->setChecked(false);
	m_ui.checkBox_showTemperatureScale->setEnabled(false);
	m_ui.pushButton_pickTemperature->setEnabled(false);
	m_ui.lineEdit_temperatureScaleFile->clear();

	// Connect widgets
	connect(m_ui.checkBox_showScale, &QCheckBox::stateChanged, [this]() { this->sendGuiData(); });
	connect(m_ui.checkBox_showTemperatureScale, &QCheckBox::stateChanged, [this]() { this->sendGuiData(); });
	connect(m_ui.checkBox_centerBoxScale, &QCheckBox::stateChanged, [this]() { this->sendGuiData(); });
	connect(m_ui.spinBox_graduation, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this](int) { this->sendGuiData(); });
	connect(m_ui.importTempScaleButton, &QPushButton::clicked, [this]() { this->importTemperatureScale(); });
	connect(m_ui.pushButton_pickTemperature, &QPushButton::clicked, [this]() { this->pickTemperature(); });

	// GuiData link
	registerGuiDataFunction(guiDType::projectLoaded, &ToolBarRenderRampGroup::onProjectLoad);
	registerGuiDataFunction(guiDType::renderActiveCamera, &ToolBarRenderRampGroup::onActiveCamera);
	registerGuiDataFunction(guiDType::focusViewport, &ToolBarRenderRampGroup::onFocusViewport);
	registerGuiDataFunction(guiDType::renderRampScale, &ToolBarRenderRampGroup::onRampScaleUpdated);
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

void ToolBarRenderRampGroup::onRampScaleUpdated(IGuiData* data)
{
	auto rampData = static_cast<GuiDataRampScale*>(data);
	if (rampData->m_camera && m_focusCamera != rampData->m_camera)
		return;

	updateRamp(rampData->m_rampScale);
}

void ToolBarRenderRampGroup::updateRamp(const RampScale& rampScaleParams)
{
	blockAllSignals(true);

	m_ui.checkBox_showScale->setChecked(rampScaleParams.showScale);
	m_ui.checkBox_showTemperatureScale->setChecked(rampScaleParams.showTemperatureScale);
	m_ui.checkBox_centerBoxScale->setChecked(rampScaleParams.centerBoxScale);
	m_ui.spinBox_graduation->setValue(rampScaleParams.graduationCount);
	updateTemperatureScaleUi(rampScaleParams);

	blockAllSignals(false);
}

void ToolBarRenderRampGroup::updateTemperatureScaleUi(const RampScale& rampScaleParams)
{
	m_temperatureScaleFile = rampScaleParams.temperatureScaleFile;

	bool hasFile = !m_temperatureScaleFile.empty();
	bool fileExists = hasFile && std::filesystem::exists(m_temperatureScaleFile);
	bool fileValid = false;
	if (fileExists)
	{
		temperature_scale::TemperatureScaleData data;
		std::string error;
		fileValid = temperature_scale::getTemperatureScaleData(m_temperatureScaleFile, data, &error);
	}

	m_ui.checkBox_showTemperatureScale->setEnabled(fileValid);
	m_ui.pushButton_pickTemperature->setEnabled(fileValid);

	if (!hasFile)
	{
		m_ui.lineEdit_temperatureScaleFile->clear();
	}
	else if (!fileExists)
	{
		m_ui.lineEdit_temperatureScaleFile->setText(QStringLiteral("No file found"));
	}
	else
	{
		m_ui.lineEdit_temperatureScaleFile->setText(QString::fromStdWString(m_temperatureScaleFile.filename().wstring()));
	}
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
					 m_ui.checkBox_showTemperatureScale->isChecked(),
					 m_temperatureScaleFile };
    m_dataDispatcher.updateInformation(new GuiDataRampScale(rs, m_focusCamera), this);
}

void ToolBarRenderRampGroup::importTemperatureScale()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Import temperature scale"), QString(), tr("Text file (*.txt);;All files (*.*)"));
	if (fileName.isEmpty())
		return;

	m_dataDispatcher.sendControl(new control::application::ImportTemperatureScaleFile(fileName.toStdWString(), m_focusCamera));
}

void ToolBarRenderRampGroup::pickTemperature()
{
	m_dataDispatcher.sendControl(new control::picking::PickTemperatureFromPick());
}
