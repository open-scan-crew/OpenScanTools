#ifndef TOOLBAR_ANIMATION_H
#define TOOLBAR_ANIMATION_H

#include <QtWidgets/qwidget.h>

#include "ui_toolbar_animationgroup.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "gui/Dialog/DialogExportVideo.h"

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
	void onProjectTreeActualize(IGuiData* keyValue);
	void onAnimationToolbarState(IGuiData* keyValue);
	void onRenderStopAnimation(IGuiData* keyValue);
	void updateUI();
	void refreshAnimationAvailability();

private slots:
	void slotStartAnimation();
	void slotStopAnimation();
	void slotGenerateVideo();
	void slotAnimationModeChanged();

private:
	std::unordered_map<guiDType, AnimGroupMethod> m_methods;
	Ui::toolbar_animationgroup m_ui;
	IDataDispatcher &m_dataDispatcher;
	DialogExportVideo* m_dialog;
	bool m_isStarted;
	bool m_isProjectLoaded;
	bool m_canStartAnimation;
};

#endif // TOOLBAR_ANIMATION_H
