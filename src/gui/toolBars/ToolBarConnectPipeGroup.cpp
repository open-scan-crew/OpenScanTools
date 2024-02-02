#include "gui/toolBars/ToolBarConnectPipeGroup.h"
#include "gui/Dialog/StandardListDialog.h"
#include "controller/controls/ControlMeasure.h"
#include "controller/controls/ControlFunction.h"
#include "controller/controls/ControlFunctionPiping.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataList.h"
#include "gui/GuiData/GuiDataMeasure.h"
#include <QtWidgets/qpushbutton.h>

ToolBarConnectPipeGroup::ToolBarConnectPipeGroup(IDataDispatcher& dataDispatcher, QWidget* parent, float scale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
{
	m_ui.setupUi(this);
	setEnabled(false);

	connect(m_ui.connectPipeButton, &QToolButton::released, this, &ToolBarConnectPipeGroup::initConnectPipe);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::activatedFunctions);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_methods.insert({ guiDType::activatedFunctions, &ToolBarConnectPipeGroup::onActivateFunction });
	m_methods.insert({ guiDType::projectLoaded, &ToolBarConnectPipeGroup::onProjectLoad });

}

ToolBarConnectPipeGroup::~ToolBarConnectPipeGroup()
{
	m_dataDispatcher.unregisterObserver(this);
}

void ToolBarConnectPipeGroup::informData(IGuiData* data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		ConnectPipeGroupMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarConnectPipeGroup::onProjectLoad(IGuiData* data)
{
	auto* function = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(function->m_isProjectLoad);
}

void ToolBarConnectPipeGroup::onActivateFunction(IGuiData* data)
{
	auto* function = static_cast<GuiDataActivatedFunctions*>(data);

	m_ui.connectPipeButton->blockSignals(true);

	switch (function->type)
	{
		case ContextType::pipeDetectionConnexion:
		{
			break;
		}
		default:
		{
			m_ui.connectPipeButton->setChecked(false);
		}
	}

	m_ui.connectPipeButton->blockSignals(false);
}

void ToolBarConnectPipeGroup::initConnectPipe()
{
	bool keepDiameter = m_ui.keepDiameterCheckBox->isChecked();
	
	double RonD(m_ui.ratioRonDextField->getValue());
	if (m_ui.detectConnectRadioButton->isChecked())
		m_dataDispatcher.sendControl(new control::measure::ActivatePipeDetectionConnexion(RonD > 0 ? RonD : 1, m_ui.standardAngleRadioButton->isChecked(), keepDiameter));
	else if (m_ui.postConnectionRadioButton->isChecked())
		m_dataDispatcher.sendControl(new control::measure::ActivatePipePostConnexion(RonD > 0 ? RonD : 1, m_ui.standardAngleRadioButton->isChecked(),keepDiameter));
	else
		m_dataDispatcher.sendControl(new control::function::Abort());
}

