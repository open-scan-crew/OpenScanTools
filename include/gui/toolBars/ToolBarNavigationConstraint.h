#ifndef TOOLBAR_NAVICONSTRAINT_H
#define TOOLBAR_NAVICONSTRAINT_H

#include <map>
#include <QtWidgets/QWidget>

#include "ui_toolbar_navigationconstraint.h"

#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

#include "models/OpenScanToolsModelEssentials.h"

class CameraNode;

class ToolBarNavigationConstraint;

typedef void (ToolBarNavigationConstraint::*constraintToolBarMethod)(IGuiData*);

enum class NaviConstraint;

class ToolBarNavigationConstraint : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarNavigationConstraint(IDataDispatcher &dataDispatcher, QWidget *parent, float guiScale);
	~ToolBarNavigationConstraint();

	// From IPanel
	void informData(IGuiData *keyValue);

private:
	void onProjectLoad(IGuiData* data);
	void onFocusViewport(IGuiData* data);

	void applyConstraint(bool checked);

private slots:
	void slotApplyConstraint(bool checked);
	void slotLock(NaviConstraint constraint);


private:
	Ui::ToolBarNavigationConstraint m_ui;

	IDataDispatcher &m_dataDispatcher;

	std::unordered_map<guiDType, constraintToolBarMethod> m_methods;
	SafePtr<CameraNode> m_focusCamera;
};

#endif // TOOLBAR_NAVICONSTRAINT_H

