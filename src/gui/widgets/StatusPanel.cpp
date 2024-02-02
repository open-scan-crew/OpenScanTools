#include "gui/widgets/StatusPanel.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"
#include <QtWidgets/QGridLayout>
#include <magic_enum/magic_enum.hpp>
#include "gui/Texts.hpp"
#include "utils/Utils.h"

StatusPanel::StatusPanel(IDataDispatcher &dataDispatcher)
	: m_dataDispatcher(dataDispatcher)
{
    m_cameraInfo = new QLabel(TEXT_CAMERA + " : --, --, -- ", this);
	m_scanInfo = new QLabel(TEXT_MOUSE + " : --, --, -- |", this);
	m_functionMode = new QLabel("", this);
	m_tmpMessageTimeout = 0;

	addPermanentWidget(m_functionMode);
	addPermanentWidget(m_scanInfo);
    addPermanentWidget(m_cameraInfo);

	methods.insert(std::pair<guiDType, statusPanelMethod>(guiDType::cameraData, &StatusPanel::onCameraData));
	methods.insert(std::pair<guiDType, statusPanelMethod>(guiDType::currentScanData, &StatusPanel::onCurrentScanData));
	methods.insert(std::pair<guiDType, statusPanelMethod>(guiDType::tmpMsgData, &StatusPanel::onTmpMsgData));
	methods.insert(std::pair<guiDType, statusPanelMethod>(guiDType::activatedFunctions, &StatusPanel::onFunctionUpdate));
	methods.insert(std::pair<guiDType, statusPanelMethod>(guiDType::renderValueDisplay, &StatusPanel::onRenderUnitUsage));

	m_dataDispatcher.registerObserverOnKey(this, guiDType::cameraData);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::pointCountData);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::tmpMsgData);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::activatedFunctions);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::renderValueDisplay);
}

void StatusPanel::informData(IGuiData *data)
{
	if (methods.find(data->getType()) != methods.end())
	{
		statusPanelMethod method = methods.at(data->getType());
		(this->*method)(data);
	}
}

void StatusPanel::onCameraData(IGuiData* idata)
{
	GuiDataCameraPos* data = static_cast<GuiDataCameraPos*>(idata);
	onCameraPos(data->pos.x, data->pos.y, data->pos.z);
}

void StatusPanel::onRenderUnitUsage(IGuiData * data)
{
	GuiDataRenderUnitUsage* parameters = static_cast<GuiDataRenderUnitUsage*>(data);
	m_valueDisplayParameters = parameters->m_valueParameters;
}

void StatusPanel::onCameraPos(double x, double y, double z)
{
	m_camPos = glm::dvec3(x, y, z);
	auto valueDisplay = [&](double t) { return Utils::roundFloat(unit_converter::meterToX(t, m_valueDisplayParameters.distanceUnit), m_valueDisplayParameters.displayedDigits); };
	std::string unitText = unit_converter::getUnitText(m_valueDisplayParameters.distanceUnit).toStdString();
	std::string text = valueDisplay(x) + unitText + " " + valueDisplay(y) + unitText + " " + valueDisplay(z) + unitText;
	m_cameraInfo->setText(TEXT_CAMERA + " " + text.c_str() + " |");
	if (m_cameraInfo->text().size() > 350)
		m_cameraInfo->setText(TEXT_CAMERA + QString(" : --, --, --") + " |");
}

void StatusPanel::onCurrentScanData(IGuiData * data)
{
	GuiDataScanCurrent *currentScanata = static_cast<GuiDataScanCurrent*>(data);
	auto valueDisplay = [&](double t) { return Utils::roundFloat(unit_converter::meterToX(t, m_valueDisplayParameters.distanceUnit), m_valueDisplayParameters.displayedDigits); };
	std::string unitText = unit_converter::getUnitText(m_valueDisplayParameters.distanceUnit).toStdString();
	std::string text = valueDisplay(currentScanata->pos.x) + unitText + " " + valueDisplay(currentScanata->pos.y) + unitText + " " + valueDisplay(currentScanata->pos.z) + unitText;
	if (currentScanata->exists == true)
		m_scanInfo->setText(TEXT_SCAN + " " + text.c_str() + " |");
	else
		m_scanInfo->setText("");
}

void StatusPanel::onTmpMsgData(IGuiData * data)
{
	GuiDataTmpMessage *tmpmsgData = static_cast<GuiDataTmpMessage*>(data);
	if (tmpmsgData->m_message.isEmpty() && m_tmpMessageTimeout != 0)
		return;
	showMessage(tmpmsgData->m_message, tmpmsgData->m_timeout);
	m_tmpMessageTimeout = tmpmsgData->m_timeout;
}

void StatusPanel::onPicking(double x, double y, double z)
{
	float len = glm::length(glm::dvec3(x, y, z) - m_camPos);
	if (std::isnan(x) || std::isnan(y) || std::isnan(z))
	{
		m_scanInfo->setText(TEXT_MOUSE + QString(" : --, --, --") + " | ");
	}
	else
	{
		auto valueDisplay = [&](double t) { return Utils::roundFloat(unit_converter::meterToX(t, m_valueDisplayParameters.distanceUnit), m_valueDisplayParameters.displayedDigits); };
		std::string unitText = unit_converter::getUnitText(m_valueDisplayParameters.distanceUnit).toStdString();
		std::string text = valueDisplay(x) + unitText + " " + valueDisplay(y) + unitText + " " + valueDisplay(z) + unitText;
		m_scanInfo->setText(TEXT_MOUSE + " " + text.c_str() + " | " + TEXT_DISTANCE + " " + valueDisplay(len).c_str() + unitText.c_str() + " |");
		if(m_scanInfo->text().size() > 350)
			m_scanInfo->setText(TEXT_MOUSE + QString(" : --, --, --") + " |");
	}
}

StatusPanel::~StatusPanel()
{
    m_dataDispatcher.unregisterObserver(this);
}

void StatusPanel::onFunctionUpdate(IGuiData *data)
{
	GuiDataActivatedFunctions *pcData = static_cast<GuiDataActivatedFunctions*>(data);

	if (pcData->type == ContextType::none)
		m_functionMode->setText("");
	else if (pcData->type == ContextType::autosaveProject)
		return;
	else
	{
		std::string str(magic_enum::enum_name(pcData->type));
		m_functionMode->setTextFormat(Qt::RichText);
		m_functionMode->setText("<b><span style='color:#A05000'>"+ TEXT_CURRENT_FUNCTION+": " +QString::fromStdString(str) + "</span></b> |");
	}
}