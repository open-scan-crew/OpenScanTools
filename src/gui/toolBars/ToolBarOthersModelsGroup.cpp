#include "gui/toolBars/ToolBarOthersModelsGroup.h"
#include "controller/controls/ControlMeasure.h"
#include "controller/controls/ControlFunction.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include <QtWidgets/qpushbutton.h>

#include "controller/controls/ControlPCObject.h"
#include "io/FileUtils.h"
#include <QtWidgets/QFileDialog>
#include <QtCore/QStandardPaths>


ToolBarOthersModelsGroup::ToolBarOthersModelsGroup(IDataDispatcher& dataDispatcher, QWidget* parent, float guiScale)
	: QWidget(parent)
	
	, m_dataDispatcher(dataDispatcher)
{
	m_ui.setupUi(this);

	setEnabled(false);
	m_ui.sphereButton->setIconSize(QSize(20, 20) * guiScale);
	m_ui.pointButton->setIconSize(QSize(20, 20) * guiScale);
	//m_ui.beamButton->setIconSize(QSize(20, 20) * guiScale);


	//fixme (Aurélien) to remove when completed
#ifndef _DEBUG
	m_ui.sphereButton->setHidden(true);
#endif

	QObject::connect(m_ui.sphereButton, &QPushButton::released, this, &ToolBarOthersModelsGroup::onSphere);
	QObject::connect(m_ui.pointButton, &QPushButton::released, this, &ToolBarOthersModelsGroup::onPoint);
	//QObject::connect(m_ui.beamButton, &QPushButton::released, this, &ToolBarOthersModelsGroup::onBeam);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::activatedFunctions);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_methods.insert({ guiDType::activatedFunctions, &ToolBarOthersModelsGroup::onActivateFunction });
	m_methods.insert({ guiDType::projectLoaded, &ToolBarOthersModelsGroup::onProjectLoad });


}

void ToolBarOthersModelsGroup::informData(IGuiData* data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		OtherModelGroupMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarOthersModelsGroup::onProjectLoad(IGuiData* data)
{
	auto* function = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(function->m_isProjectLoad);
}

void ToolBarOthersModelsGroup::onActivateFunction(IGuiData* data)
{
	auto* function = static_cast<GuiDataActivatedFunctions*>(data);

	m_ui.sphereButton->blockSignals(true);
	m_ui.pointButton->blockSignals(true);


	switch (function->type)
	{
	case ContextType::pointCreation:
		m_ui.pointButton->setChecked(true);
		m_ui.sphereButton->setChecked(false);
		//m_ui.beamButton->setChecked(false);
		break;
	case ContextType::beamDetection:
		m_ui.pointButton->setChecked(false);
		m_ui.sphereButton->setChecked(true);
		//m_ui.beamButton->setChecked(true);
		break;
	default:
		m_ui.sphereButton->setChecked(false);
		m_ui.pointButton->setChecked(false);
		//m_ui.beamButton->setChecked(false);
	}
	m_ui.sphereButton->blockSignals(false);
	m_ui.pointButton->blockSignals(false);
	//m_ui.beamButton->setChecked(false);
}

ToolBarOthersModelsGroup::~ToolBarOthersModelsGroup()
{
	m_dataDispatcher.unregisterObserver(this);
}

void ToolBarOthersModelsGroup::onSphere()
{
	if (m_ui.sphereButton->isChecked())
		m_dataDispatcher.sendControl(new control::measure::ActivateDetectBeam());
	else
		m_dataDispatcher.sendControl(new control::function::Abort());
}

void ToolBarOthersModelsGroup::onPoint()
{
	if (m_ui.pointButton->isChecked())
		m_dataDispatcher.sendControl(new control::measure::ActivatePointCreation());
	else
		m_dataDispatcher.sendControl(new control::function::Abort());
}

void ToolBarOthersModelsGroup::onBeam()
{
	/*if (m_ui.beamButton->isChecked())
		m_dataDispatcher.sendControl(new control::measure::ActivateDetectBeam());
	else
		m_dataDispatcher.sendControl(new control::function::Abort());*/
}