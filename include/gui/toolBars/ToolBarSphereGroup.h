#ifndef TOOLBAR_SPHEREGROUP_H
#define TOOLBAR_SPHEREGROUP_H

#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

#include <unordered_map>
#include <QtWidgets/qwidget.h>

#include "ui_toolbar_spheregroup.h"

class ToolBarSphereGroup;

typedef void (ToolBarSphereGroup::* SphereGroupMethod)(IGuiData*);

class ToolBarSphereGroup : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarSphereGroup(IDataDispatcher& dataDispatcher, QWidget* parent, float scale);

	void informData(IGuiData* data);

private:
	~ToolBarSphereGroup();
	void onProjectLoad(IGuiData* data);
	void onActivateFunction(IGuiData* data);
	void initSphereDetection();
	void initLargeSphereDetection();
	void initSphereStandardManagement();

private:
	std::unordered_map<guiDType, SphereGroupMethod> m_methods;
	Ui::toolbar_spheregroup m_ui;
	IDataDispatcher& m_dataDispatcher;
};

#endif

