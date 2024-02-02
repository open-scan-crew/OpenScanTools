#ifndef TOOLBAR_MANIPULATEOBJECTS_H
#define TOOLBAR_MANIPULATEOBJECTS_H

#include "ui_toolbar_manipulateObjects.h"

#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

class ToolBarManipulateObjects;

typedef void (ToolBarManipulateObjects::*constraintToolBarMethod)(IGuiData*);

enum class NaviConstraint;

class ToolBarManipulateObjects : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarManipulateObjects(IDataDispatcher &dataDispatcher, QWidget *parent, float guiScale);
	~ToolBarManipulateObjects();

	// From IPanel
	void informData(IGuiData *keyValue);

private:
	void onProjectLoad(IGuiData* data);
	void onActivateFunction(IGuiData* data);

	void translate();
	void translateLockZ();
	void translateAlongZ();
	void rotate2points();
	void rotate3points();

private:
	Ui::ToolBarManipulateObjects m_ui;

	IDataDispatcher &m_dataDispatcher;

	std::unordered_map<guiDType, constraintToolBarMethod> m_methods;
};

#endif // TOOLBAR_NAVICONSTRAINT_H

