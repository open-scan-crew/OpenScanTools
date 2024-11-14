#ifndef TOOLBAR_SLABGROUP_H
#define TOOLBAR_SLABGROUP_H

#include <QtWidgets/qwidget.h>
#include "ui_toolbar_slabgroup.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

class ToolBarSlabGroup;

typedef void (ToolBarSlabGroup::* SlabGroupMethod)(IGuiData*);

class ToolBarSlabGroup : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarSlabGroup(IDataDispatcher& dataDispatcher, QWidget* parent, float scale);

	void informData(IGuiData* data);

private:
	~ToolBarSlabGroup();
	void onProjectLoad(IGuiData* data);
	void onActivateFunction(IGuiData* data);
	void init1Click();
	void init2Click();

private:
	std::unordered_map<guiDType, SlabGroupMethod> m_methods;
	Ui::toolbar_slabgroup m_ui;
	IDataDispatcher& m_dataDispatcher;
};

#endif // TOOLBAR_PIPEGROUP_H

