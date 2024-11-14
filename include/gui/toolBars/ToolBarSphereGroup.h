#ifndef TOOLBAR_SPHEREGROUP_H
#define TOOLBAR_SPHEREGROUP_H

#include <QtWidgets/qwidget.h>
#include "ui_toolbar_spheregroup.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "models/application/List.h"
#include "models/OpenScanToolsModelEssentials.h"

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
	void onStandardRecieved(IGuiData* data);
	void initSphereDetection();
	void initLargeSphereDetection();
	void slotStandardChanged(const int& index);
	void initSphereStandardManagement();

private:
	std::unordered_map<guiDType, SphereGroupMethod> m_methods;
	Ui::toolbar_spheregroup m_ui;
	IDataDispatcher& m_dataDispatcher;
	std::vector<SafePtr<StandardList>> m_standardsElems;
};

#endif // TOOLBAR_PIPEGROUP_H

