#ifndef TOOLBAR_AUTOSEEDING_H
#define TOOLBAR_AUTOSEEDING_H

#include <QtWidgets/QWidget>
#include "ui_toolbar_autoSeeding.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "gui/Dialog/DialogSetOfPoints.h"
#include "controller/messages/PlaneMessage.h"


class ToolBarAutoSeeding : public QWidget, public IPanel
{
	Q_OBJECT
public:
	explicit ToolBarAutoSeeding(IDataDispatcher &dataDispatcher, QWidget *parent, float guiScale);

	void informData(IGuiData *data);


private:
	~ToolBarAutoSeeding();

	void init1FromTop();
	void init1FromBot();
	void init2FromTop();
	void init2FromBot();
	void init3Plane();
	void init4Horiz();
	void init5Slope();
	void init6Vert();
	void init7Slope();
	void init8Horiz();
	void init8Plane();

	void launchAutoSeeding(SetOfPointsOptions options, bool userAxes, bool createMeasures, bool fromTop, bool horizontal);


private:
	Ui::toolbar_autoSeeding m_ui;
	IDataDispatcher &m_dataDispatcher;
};

#endif // TOOLBAR_LUCASEXPERIMENT_H

