#ifndef TOOLBAR_VIEWPOINT_H
#define TOOLBAR_VIEWPOINT_H

#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

#include <QtWidgets/qwidget.h>

#include "ui_toolbar_viewPoint.h"

class ToolBarViewPoint : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarViewPoint(IDataDispatcher& dataDispatcher, QWidget* parent, float scale);

	void informData(IGuiData* data);

private:
	~ToolBarViewPoint();
	void onProjectLoad(IGuiData* data);
	void onActivateFunction(IGuiData* data);
	void initViewPointCreation();
private:

	typedef void (ToolBarViewPoint::* ViewPointGroupMethod)(IGuiData*);

private:
	std::unordered_map<guiDType, ViewPointGroupMethod> m_methods;
	Ui::toolbar_viewPoint m_ui;
	IDataDispatcher& m_dataDispatcher;
};

#endif // TOOLBAR_VIEWPOINT_H

