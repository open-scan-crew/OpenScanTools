#include "gui/toolBars/ToolBarBeamDetection.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "controller/controls/ControlMeasure.h"
#include "controller/controls/ControlFunction.h"
#include <QtWidgets/qtoolbutton.h>

ToolBarBeamDetection::ToolBarBeamDetection(IDataDispatcher& dataDispatcher, QWidget* parent, float guiScale)
	: QWidget(parent)
	
	, m_dataDispatcher(dataDispatcher)
{
	m_ui.setupUi(this);

	setEnabled(false);

	QObject::connect(m_ui.detectSimpleButton, &QToolButton::released, this, &ToolBarBeamDetection::onSimpleDetection);
	QObject::connect(m_ui.manualExtensionButton, &QToolButton::released, this, &ToolBarBeamDetection::onManualExtension);
	QObject::connect(m_ui.applyStandardCheckBox, &QCheckBox::stateChanged, this, &ToolBarBeamDetection::onApplyStandard);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::activatedFunctions);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_methods.insert({ guiDType::activatedFunctions, &ToolBarBeamDetection::onActivateFunction });
	m_methods.insert({ guiDType::projectLoaded, &ToolBarBeamDetection::onProjectLoad });


}

void ToolBarBeamDetection::informData(IGuiData* data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		BeamDetectionMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarBeamDetection::onProjectLoad(IGuiData* data)
{
	auto* function = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(function->m_isProjectLoad);

}

void ToolBarBeamDetection::onActivateFunction(IGuiData* data)
{
	auto* function = static_cast<GuiDataActivatedFunctions*>(data);

	m_ui.detectSimpleButton->blockSignals(true);
	m_ui.manualExtensionButton->blockSignals(true);

	/*
	switch (function->type)
	{
	case ContextType:::
		m_ui.detectSimpleButton->setChecked(true);
		m_ui.manualExtensionButton->setChecked(false);
		break;
	case ContextType:::
		m_ui.detectSimpleButton->setChecked(false);
		m_ui.manualExtensionButton->setChecked(true);
		break;
	default:
		m_ui.detectSimpleButton->setChecked(false);
		m_ui.manualExtensionButton->setChecked(false);
	}
	*/
	m_ui.detectSimpleButton->blockSignals(false);
	m_ui.manualExtensionButton->blockSignals(false);
}

void ToolBarBeamDetection::onSimpleDetection()
{
	//m_dataDispatcher.sendControl(new control::measure::ActivatePeopleRemover);
}

void ToolBarBeamDetection::onManualExtension()
{
	m_dataDispatcher.sendControl(new control::measure::ActivateDetectBeam);
}

void ToolBarBeamDetection::onApplyStandard(int checkState)
{

}

ToolBarBeamDetection::~ToolBarBeamDetection()
{
	m_dataDispatcher.unregisterObserver(this);
}