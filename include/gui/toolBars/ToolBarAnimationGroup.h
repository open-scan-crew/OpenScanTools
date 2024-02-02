#ifndef TOOLBAR_ANIMATION_H
#define TOOLBAR_ANIMATION_H

#include <QWidget>

#include "ui_toolbar_animationgroup.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

class ToolBarAnimationGroup;

typedef void (ToolBarAnimationGroup::* AnimGroupMethod)(IGuiData*);

class ToolBarAnimationGroup : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarAnimationGroup(IDataDispatcher &dataDispatcher, QWidget *parent = 0, float guiScale = 1.f);
	~ToolBarAnimationGroup();
	void informData(IGuiData *keyValue) override;

private:
	void onProjectLoad(IGuiData* keyValue);
	void updateUI();

private slots:
	void slotStartAnimation();
	void slotStopAnimation();
	void slotCleanAnimationList();
	void slotSpeedChange();
	void slotLoopAnimation();
	void slotRecordPerformance();
	void slotScansAnimation();

private:
	std::unordered_map<guiDType, AnimGroupMethod> m_methods;
	Ui::toolbar_animationgroup m_ui;
	IDataDispatcher &m_dataDispatcher;
	bool m_isStarted;
};

#endif // TOOLBAR_ANIMATION_H
