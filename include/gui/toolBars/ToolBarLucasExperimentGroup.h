#ifndef TOOLBAR_LUCASEXPERIMENT_H
#define TOOLBAR_LUCASEXPERIMENT_H

#include <QtWidgets/qwidget.h>
#include "ui_toolbar_lucasExperiment.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "gui/Dialog/DialogSetOfPoints.h"


class ToolBarLucasExperimentGroup : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarLucasExperimentGroup(IDataDispatcher &dataDispatcher, QWidget *parent, float guiScale);

	void initBigCylinderFit();
	void initCylinderToCylinderMeasure();
	void initCylinderToPlaneMeasure();
	void initPointToPlaneMeasure();
	void initPointToCylinderMeasure();
	void initMultipleCylindersMeasure();
	void initExtendCylinder();
	void init4ClicsSphere();
	void initSphere();
	void initBeamDetection();
	void initSlab();
	void initPlaneConnexion();
	void initLocalPlane();
	void initHorizontalPlane();
	void initVerticalPlane();
	void initAutoExtendPlane();
	void initMultipleSeedsPlane();
	void initBeamBendingManual();
	void initTorus();
	void informData(IGuiData *data);

	void init1FromTop();
	void init1FromBot();
	void init2FromTop();
	void init2FromBot();
	void init3Axis();
	void init3Plane();
	void init4Horiz();
	void init5Slope();
	void init6Vert();
	void init7Slope();
	void init8Horiz();
	void init8Plane();

	void initTrajectory();

private:
	~ToolBarLucasExperimentGroup();


private:
	Ui::toolbar_lucasExperiment *ui;
	IDataDispatcher &m_dataDispatcher;
	DialogSetOfPoints m_dialogSetOfPoints;
};

#endif // TOOLBAR_LUCASEXPERIMENT_H

