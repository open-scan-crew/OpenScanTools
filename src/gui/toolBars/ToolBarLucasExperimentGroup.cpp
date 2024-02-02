#include "gui/toolBars/ToolBarLucasExperimentGroup.h"
#include "controller/controls/ControlMeasure.h"
#include <QtWidgets/qpushbutton.h>

ToolBarLucasExperimentGroup::ToolBarLucasExperimentGroup(IDataDispatcher &dataDispatcher, QWidget *parent, float guiScale)
	: QWidget(parent)
	, ui(new Ui::toolbar_lucasExperiment)
	, m_dataDispatcher(dataDispatcher)
	, m_dialogSetOfPoints(dataDispatcher, parent)
{
	ui->setupUi(this);
	//setEnabled(false);

	QObject::connect(ui->localPlane, &QPushButton::released, this, &ToolBarLucasExperimentGroup::initLocalPlane);
	QObject::connect(ui->planeConnexion, &QPushButton::released, this, &ToolBarLucasExperimentGroup::initPlaneConnexion);

	QObject::connect(ui->horizontalPlane, &QPushButton::pressed, this, &ToolBarLucasExperimentGroup::initHorizontalPlane);
	QObject::connect(ui->verticalPlane, &QPushButton::pressed, this, &ToolBarLucasExperimentGroup::initVerticalPlane);
	QObject::connect(ui->autoExtendPlane, &QPushButton::pressed, this, &ToolBarLucasExperimentGroup::initAutoExtendPlane);	
	QObject::connect(ui->multipleSeeds, &QPushButton::pressed, this, &ToolBarLucasExperimentGroup::initMultipleSeedsPlane);   
	QObject::connect(ui->beamBendingManual, &QPushButton::pressed, this, &ToolBarLucasExperimentGroup::initBeamBendingManual);
	QObject::connect(ui->trajetoryButton, &QPushButton::pressed, this, &ToolBarLucasExperimentGroup::initTrajectory);

	ui->localPlane->setVisible(false);
	ui->planeConnexion->setVisible(false);
	ui->horizontalPlane->setVisible(false);
	ui->verticalPlane->setVisible(false);
	ui->autoExtendPlane->setVisible(false);
	ui->multipleSeeds->setVisible(false);
	ui->beamBendingManual->setVisible(false);

	

}

void ToolBarLucasExperimentGroup::informData(IGuiData *data)
{
}

ToolBarLucasExperimentGroup::~ToolBarLucasExperimentGroup()
{
	m_dataDispatcher.unregisterObserver(this);
}

void ToolBarLucasExperimentGroup::initBigCylinderFit()
{
	m_dataDispatcher.sendControl(new control::measure::ActivateBigCylinderFit());
}

void ToolBarLucasExperimentGroup::initCylinderToCylinderMeasure()
{
	m_dataDispatcher.sendControl(new control::measure::ActivateCylinderToCylinderMeasure());
}

void ToolBarLucasExperimentGroup::initCylinderToPlaneMeasure()
{
	m_dataDispatcher.sendControl(new control::measure::ActivateCylinderToPlaneMeasure());
}

void ToolBarLucasExperimentGroup::initPointToPlaneMeasure()
{
	m_dataDispatcher.sendControl(new control::measure::ActivatePointToPlaneMeasure());
}

void ToolBarLucasExperimentGroup::init4ClicsSphere()
{
	m_dataDispatcher.sendControl(new control::measure::Activate4ClicsSphere());
}

void ToolBarLucasExperimentGroup::initSphere()
{
	m_dataDispatcher.sendControl(new control::measure::ActivateSphere());
}

void ToolBarLucasExperimentGroup::initPointToCylinderMeasure()
{
	m_dataDispatcher.sendControl(new control::measure::ActivatePointToCylinderMeasure());
}

void ToolBarLucasExperimentGroup::initMultipleCylindersMeasure()
{
	m_dataDispatcher.sendControl(new control::measure::ActivateMultipleCylindersMeasure());
}

void ToolBarLucasExperimentGroup::initExtendCylinder()
{
	m_dataDispatcher.sendControl(new control::measure::ActivateExtendCylinder());
}

void ToolBarLucasExperimentGroup::initBeamDetection()
{
	m_dataDispatcher.sendControl(new control::measure::ActivateDetectBeam());
}

void ToolBarLucasExperimentGroup::initSlab()
{
	m_dataDispatcher.sendControl(new control::measure::ActivateSlabDetection());
}

void ToolBarLucasExperimentGroup::initPlaneConnexion()
{
	m_dataDispatcher.sendControl(new control::measure::ActivatePlaneConnexion());
}

void ToolBarLucasExperimentGroup::initLocalPlane()
{
	PlaneDetectionOptions options = PlaneDetectionOptions::localPlane;
	m_dataDispatcher.sendControl(new control::measure::ActivatePlaneDetection(options));
}

void ToolBarLucasExperimentGroup::initVerticalPlane()
{
	PlaneDetectionOptions options = PlaneDetectionOptions::vertical;
	m_dataDispatcher.sendControl(new control::measure::ActivatePlaneDetection(options));
}

void ToolBarLucasExperimentGroup::initHorizontalPlane()
{
	PlaneDetectionOptions options = PlaneDetectionOptions::horizontal;
	m_dataDispatcher.sendControl(new control::measure::ActivatePlaneDetection(options));
}

void ToolBarLucasExperimentGroup::initAutoExtendPlane()
{
	PlaneDetectionOptions options = PlaneDetectionOptions::autoExtend;
	m_dataDispatcher.sendControl(new control::measure::ActivatePlaneDetection(options));
}

void ToolBarLucasExperimentGroup::initMultipleSeedsPlane()
{
	PlaneDetectionOptions options = PlaneDetectionOptions::multipleSeeds;
	m_dataDispatcher.sendControl(new control::measure::ActivatePlaneDetection(options));
}

void ToolBarLucasExperimentGroup::initBeamBendingManual()
{
	BeamBendingOptions options = BeamBendingOptions::manual;
	m_dataDispatcher.sendControl(new control::measure::ActivateBeamBending(options));
}

void ToolBarLucasExperimentGroup::init1FromTop()
{
	SetOfPointsOptions options = SetOfPointsOptions::case1;
	double step(0.05), threshold(0.1);
	bool userAxes(false), createMeasures(true), fromTop(true), horizontal(true);
	m_dataDispatcher.sendControl(new control::measure::ActivateSetOfPoints(step, threshold, options, userAxes, createMeasures, fromTop, horizontal));
}


void ToolBarLucasExperimentGroup::init1FromBot()
{
	SetOfPointsOptions options = SetOfPointsOptions::case1;
	double step(0.05), threshold(0.1);
	bool userAxes(false), createMeasures(true), fromTop(false), horizontal(true);
	m_dataDispatcher.sendControl(new control::measure::ActivateSetOfPoints(step, threshold, options, userAxes, createMeasures, fromTop, horizontal));
}

void ToolBarLucasExperimentGroup::init2FromTop()
{
	SetOfPointsOptions options = SetOfPointsOptions::case2;
	double step(0.05), threshold(0.1);
	bool userAxes(false), createMeasures(true), fromTop(true), horizontal(true);
	m_dataDispatcher.sendControl(new control::measure::ActivateSetOfPoints(step, threshold, options, userAxes, createMeasures, fromTop, horizontal));
}

void ToolBarLucasExperimentGroup::init2FromBot()
{
	SetOfPointsOptions options = SetOfPointsOptions::case2;
	double step(0.05), threshold(0.1);
	bool userAxes(false), createMeasures(true), fromTop(false), horizontal(true);
	m_dataDispatcher.sendControl(new control::measure::ActivateSetOfPoints(step, threshold, options, userAxes, createMeasures, fromTop, horizontal));
}

void ToolBarLucasExperimentGroup::init3Axis()
{
	SetOfPointsOptions options = SetOfPointsOptions::case3;
	double step(0.05), threshold(0.1);
	bool userAxes(true), createMeasures(true), fromTop(true), horizontal(true);
	m_dataDispatcher.sendControl(new control::measure::ActivateSetOfPoints(step, threshold, options, userAxes, createMeasures, fromTop, horizontal));
}

void ToolBarLucasExperimentGroup::init3Plane()
{
	SetOfPointsOptions options = SetOfPointsOptions::case3;
	double step(0.05), threshold(0.1);
	bool userAxes(false), createMeasures(true), fromTop(true), horizontal(true);
	m_dataDispatcher.sendControl(new control::measure::ActivateSetOfPoints(step, threshold, options, userAxes, createMeasures, fromTop, horizontal));
}

void ToolBarLucasExperimentGroup::init4Horiz()
{
	SetOfPointsOptions options = SetOfPointsOptions::case4;
	double step(0.05), threshold(0.1);
	bool userAxes(false), createMeasures(true), fromTop(true), horizontal(true);
	m_dataDispatcher.sendControl(new control::measure::ActivateSetOfPoints(step, threshold, options, userAxes, createMeasures, fromTop, horizontal));
}

void ToolBarLucasExperimentGroup::init5Slope()
{
	SetOfPointsOptions options = SetOfPointsOptions::case5;
	double step(0.05), threshold(0.1);
	bool userAxes(false), createMeasures(true), fromTop(true), horizontal(true);
	m_dataDispatcher.sendControl(new control::measure::ActivateSetOfPoints(step, threshold, options, userAxes, createMeasures, fromTop, horizontal));
}

void ToolBarLucasExperimentGroup::init6Vert()
{
	SetOfPointsOptions options = SetOfPointsOptions::case6;
	double step(0.05), threshold(0.1);
	bool userAxes(false), createMeasures(true), fromTop(true), horizontal(true);
	m_dataDispatcher.sendControl(new control::measure::ActivateSetOfPoints(step, threshold, options, userAxes, createMeasures, fromTop, horizontal));
}

void ToolBarLucasExperimentGroup::init7Slope()
{
	SetOfPointsOptions options = SetOfPointsOptions::case7;
	double step(0.05), threshold(0.1);
	bool userAxes(false), createMeasures(true), fromTop(true), horizontal(true);
	m_dataDispatcher.sendControl(new control::measure::ActivateSetOfPoints(step, threshold, options, userAxes, createMeasures, fromTop, horizontal));
}

void ToolBarLucasExperimentGroup::init8Horiz()
{
	SetOfPointsOptions options = SetOfPointsOptions::case8;
	double step(0.05), threshold(0.1);
	bool userAxes(false), createMeasures(true), fromTop(true), horizontal(true);
	m_dataDispatcher.sendControl(new control::measure::ActivateSetOfPoints(step, threshold, options, userAxes, createMeasures, fromTop, horizontal));
}

void ToolBarLucasExperimentGroup::init8Plane()
{
	SetOfPointsOptions options = SetOfPointsOptions::case8;
	double step(0.05), threshold(0.1);
	bool userAxes(false), createMeasures(true), fromTop(true), horizontal(false);
	m_dataDispatcher.sendControl(new control::measure::ActivateSetOfPoints(step, threshold, options, userAxes, createMeasures, fromTop, horizontal));
}
void ToolBarLucasExperimentGroup::initTorus()
{
	m_dataDispatcher.sendControl(new control::measure::ActivateTorus());
}

void ToolBarLucasExperimentGroup::initTrajectory()
{
	m_dataDispatcher.sendControl(new control::measure::ActivateTrajectory());
}