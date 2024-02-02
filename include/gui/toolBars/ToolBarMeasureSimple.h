#ifndef TOOLBAR_MEASURESIMPLE_H
#define TOOLBAR_MEASURESIMPLE_H

#include <QtWidgets/QWidget>
#include "ui_toolbar_measuressimple.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "gui/GuiData/GuiDataGeneralProject.h"

class ToolBarMeasureSimple;

typedef void (ToolBarMeasureSimple::* MeasureSimpleMethod)(IGuiData*);

class ToolBarMeasureSimple : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarMeasureSimple(IDataDispatcher &dataDispatcher, QWidget *parent, float scale);

	void informData(IGuiData *data);

	void setPolyligneOptions(bool visible);

private:
	~ToolBarMeasureSimple();
	void initSimpleMeasure();
	void initPolylineMeasure();

	void activeOptions();

	void onProjectLoad(IGuiData* data);
	void onActivateFunction(IGuiData* data);
	void onUserOrientation(IGuiData* data);
	void onProjectOrientation(IGuiData* data);

private:
	std::unordered_map<guiDType, MeasureSimpleMethod> m_methods;
	Ui::toolbar_measuresimple m_ui;
	IDataDispatcher &m_dataDispatcher;

	double m_userOrientationAngle = 0.0;
};

#endif // TOOLBAR_MEASUREGROUP_H

