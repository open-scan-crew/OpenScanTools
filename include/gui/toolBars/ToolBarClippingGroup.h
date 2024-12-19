#ifndef TOOLBAR_CLIPPINGGAGROUP_H
#define TOOLBAR_CLIPPINGGAGROUP_H

#include <QtWidgets/qwidget.h>
#include "ui_toolbar_clippingGroup.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

class ToolBarClippingGroup;

typedef void (ToolBarClippingGroup::*ClipGroupMethod)(IGuiData*);

class ToolBarClippingGroup : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarClippingGroup(IDataDispatcher &dataDispatcher, QWidget *parent, float scale);

	void informData(IGuiData *data) override;

private:
	void onCopyDone(IGuiData* data);
	void onProjectLoad(IGuiData *data);

public slots:
	void clickClippingProperties();
	void clickAttachedBox();
	void clickAttachedBox2Points();
	void clickLocalBox();
	void clickGlobalBox();
	void clickCopyBox();

private:
	~ToolBarClippingGroup();

private:
	std::unordered_map<guiDType, ClipGroupMethod> m_methods;

	Ui::toolbar_clippinggroup m_ui;
	IDataDispatcher &m_dataDispatcher;
};

#endif // TOOLBAR_CLIPPINGGAGROUP_H

