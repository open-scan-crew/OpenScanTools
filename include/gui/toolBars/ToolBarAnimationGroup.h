#ifndef TOOLBAR_ANIMATION_H
#define TOOLBAR_ANIMATION_H

#include <QtWidgets/qwidget.h>
#include <QtWidgets/qbuttongroup.h>
#include <QtCore/qelapsedtimer.h>
#include <QtCore/qtimer.h>

#include "ui_toolbar_animationgroup.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "gui/Dialog/DialogExportVideo.h"
#include "models/application/ViewPointAnimation.h"

class ToolBarAnimationGroup;
class DialogAnimationConfig;

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
	void onAnimationPlaybackStart(IGuiData* keyValue);
	void onRenderStopAnimation(IGuiData* keyValue);
	void updateUI();
	void refreshAnimationAvailability();
	const ViewPointAnimationConfig* getSelectedAnimationConfig() const;
	void updateChronometerDisplay();
	void startChronometer();
	void pauseChronometer();
	void resetChronometer();
	void finishChronometer();
	void startViewpointsAnimationPlayback();

private slots:
	void slotStartAnimation();
	void slotStopAnimation();
	void slotPauseAnimation();
	void slotGenerateVideo();
	void slotAnimationModeChanged();
	void slotNewViewPointAnimationConfig();
	void slotEditViewPointAnimationConfig();
	void slotAnimationConfigChanged(int index);
	void onAnimationData(IGuiData* keyValue);
	void slotChronometerTick();

private:
	std::unordered_map<guiDType, AnimGroupMethod> m_methods;
	Ui::toolbar_animationgroup m_ui;
	IDataDispatcher &m_dataDispatcher;
	DialogExportVideo* m_dialog;
	DialogAnimationConfig* m_animationConfigDialog;
	QButtonGroup m_animationModeButtons;
	bool m_isStarted;
	bool m_isPaused;
	bool m_isOrbitalRunning;
	bool m_isProjectLoaded;
	bool m_canStartAnimation;
	bool m_isStopRequested;
	bool m_pendingViewpointsStart;
	bool m_waitingChronometerStartAtFirstViewpoint;
	QElapsedTimer m_chronometerRunTimer;
	QTimer m_chronometerUpdateTimer;
	qint64 m_chronometerAccumulatedMs;
	std::vector<ViewPointAnimationConfig> m_animationConfigs;
	std::vector<AnimationViewpointInfo> m_availableViewpoints;
};

#endif // TOOLBAR_ANIMATION_H
