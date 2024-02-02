#include "gui/toolBars/ToolBarMeasureGroup.h"
#include "controller/controls/ControlMeasure.h"
#include "controller/controls/ControlFunction.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include <QtWidgets/qpushbutton.h>

ToolBarMeasureGroup::ToolBarMeasureGroup(IDataDispatcher &dataDispatcher, QWidget *parent, float guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
{
	m_ui.setupUi(this);

	setEnabled(false);
	m_ui.pointPlaneButton->setIconSize(QSize(20, 20) * guiScale);
	m_ui.pipePipeButton->setIconSize(QSize(20, 20) * guiScale);
	m_ui.pointPipeButton->setIconSize(QSize(20, 20) * guiScale);
	m_ui.pipePlaneButton->setIconSize(QSize(20, 20) * guiScale);
	m_ui.pointMeshButton->setIconSize(QSize(20, 20) * guiScale);
	//m_ui.threePointsPlaneCheckBox->setChecked(false);

	m_ui.pointMeshButton->hide();

	connect(m_ui.pointPlaneButton, &QPushButton::released, this, &ToolBarMeasureGroup::initPointPlaneMeasure);
	connect(m_ui.pipePipeButton, &QPushButton::released, this, &ToolBarMeasureGroup::initPipePipeMeasure);
	connect(m_ui.pointPipeButton, &QPushButton::released, this, &ToolBarMeasureGroup::initPointPipeMeasure);
	connect(m_ui.pipePlaneButton, &QPushButton::released, this, &ToolBarMeasureGroup::initPipePlaneMeasure);
	connect(m_ui.threePointsPlaneCheckBox, &QCheckBox::stateChanged, this, &ToolBarMeasureGroup::switchTo3Plan);
	connect(m_ui.pointMeshButton, &QPushButton::released, this, &ToolBarMeasureGroup::initPointMeshMeasure);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::activatedFunctions);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_methods.insert({ guiDType::projectLoaded, &ToolBarMeasureGroup::onProjectLoad });
	m_methods.insert({ guiDType::activatedFunctions, &ToolBarMeasureGroup::onActivateFunction });
}

void ToolBarMeasureGroup::informData(IGuiData* data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		MeasureGroupMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarMeasureGroup::onProjectLoad(IGuiData* data)
{
	GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(plData->m_isProjectLoad);
	m_ui.pointPlaneButton->setEnabled(true);
	m_ui.pipePipeButton->setEnabled(true);
	m_ui.pointPipeButton->setEnabled(true);
	m_ui.pipePlaneButton->setEnabled(true);
	m_ui.pointMeshButton->setEnabled(true);
}

void ToolBarMeasureGroup::onActivateFunction(IGuiData *data)
{
	auto* function = static_cast<GuiDataActivatedFunctions*>(data);

	m_ui.pointPlaneButton->blockSignals(true);
	m_ui.pipePipeButton->blockSignals(true);
	m_ui.pointPipeButton->blockSignals(true);
	m_ui.pipePlaneButton->blockSignals(true);
	m_ui.pointMeshButton->blockSignals(true);


	m_ui.pointPlaneButton->setChecked(false);
	m_ui.pipePipeButton->setChecked(false);
	m_ui.pointPipeButton->setChecked(false);
	m_ui.pipePlaneButton->setChecked(false);
	m_ui.pointMeshButton->setChecked(false);

	switch (function->type)
	{
	case ContextType::pointToPlane3:
	case ContextType::pointToPlane:
        m_ui.pointPlaneButton->setChecked(true);
		break;

	case ContextType::cylinderToCylinder:
		m_ui.pipePipeButton->setChecked(true);
		break;

	case ContextType::pointToCylinder:
		m_ui.pointPipeButton->setChecked(true);
		break;

	case ContextType::cylinderToPlane3:
	case ContextType::cylinderToPlane:
		m_ui.pipePlaneButton->setChecked(true);
		break;
	case ContextType::meshDistance:
		m_ui.pointMeshButton->setChecked(true);
		break;
	default:
		break;
	}

	m_ui.pointPlaneButton->blockSignals(false);
	m_ui.pipePipeButton->blockSignals(false);
	m_ui.pointPipeButton->blockSignals(false);
	m_ui.pipePlaneButton->blockSignals(false);
	m_ui.pointMeshButton->blockSignals(false);
}

ToolBarMeasureGroup::~ToolBarMeasureGroup()
{
    m_dataDispatcher.unregisterObserver(this);
}

void ToolBarMeasureGroup::initPointPlaneMeasure() 
{
	if (m_ui.pointPlaneButton->isChecked())
	{
		if (m_ui.threePointsPlaneCheckBox->isChecked())
			m_dataDispatcher.sendControl(new control::measure::ActivatePointToPlane3Measure());
		else	
			m_dataDispatcher.sendControl(new control::measure::ActivatePointToPlaneMeasure());
	}
	else
		m_dataDispatcher.sendControl(new control::function::Abort());
}

void ToolBarMeasureGroup::initPipePipeMeasure()
{
	if (m_ui.pipePipeButton->isChecked())
        m_dataDispatcher.sendControl(new control::measure::ActivateCylinderToCylinderMeasure());
    else
        m_dataDispatcher.sendControl(new control::function::Abort());
}

void ToolBarMeasureGroup::initPointPipeMeasure() 
{
    if (m_ui.pointPipeButton->isChecked())
        m_dataDispatcher.sendControl(new control::measure::ActivatePointToCylinderMeasure());
    else
        m_dataDispatcher.sendControl(new control::function::Abort());
}

void ToolBarMeasureGroup::initPipePlaneMeasure() 
{
	if (m_ui.pipePlaneButton->isChecked())
	{
		if (m_ui.threePointsPlaneCheckBox->isChecked())
		{
			m_dataDispatcher.sendControl(new control::measure::ActivateCylinderToPlane3Measure());
		}
		else
			m_dataDispatcher.sendControl(new control::measure::ActivateCylinderToPlaneMeasure());
	}
	else
		m_dataDispatcher.sendControl(new control::function::Abort());
}

void ToolBarMeasureGroup::initPointMeshMeasure()
{
	if (m_ui.pointMeshButton->isChecked())
		m_dataDispatcher.sendControl(new control::measure::ActivatePointToMeshMeasure());
	else
		m_dataDispatcher.sendControl(new control::function::Abort());
}

void ToolBarMeasureGroup::switchTo3Plan(bool threeplancheck)
{
	m_dataDispatcher.sendControl(new control::measure::Switch3PlanMeasure(threeplancheck));
}
