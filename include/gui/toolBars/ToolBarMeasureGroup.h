#ifndef TOOLBAR_MEASUREGROUP_H
#define TOOLBAR_MEASUREGROUP_H

#include <QtWidgets/qwidget.h>
#include "ui_toolbar_measuregroup.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

class ToolBarMeasureGroup;

typedef void (ToolBarMeasureGroup::* MeasureGroupMethod)(IGuiData*);

class ToolBarMeasureGroup : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarMeasureGroup(IDataDispatcher &dataDispatcher, QWidget *parent, float scale);

	void informData(IGuiData *data);

private:
	~ToolBarMeasureGroup();
	void initPointPlaneMeasure();
	void initPipePipeMeasure();
	void initPointPipeMeasure();
	void initPipePlaneMeasure();
	void initPointMeshMeasure();
	void switchTo3Plan(bool triplancheck);

	void onProjectLoad(IGuiData* data);
	void onActivateFunction(IGuiData* data);

private:
	std::unordered_map<guiDType, MeasureGroupMethod> m_methods;
	Ui::toolbar_measuregroup m_ui;
	IDataDispatcher &m_dataDispatcher;
};

#endif // TOOLBAR_MEASUREGROUP_H

