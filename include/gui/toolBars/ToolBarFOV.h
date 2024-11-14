#ifndef TOOLBAR_FOV_H
#define TOOLBAR_FOV_H

#include <map>
#include <QtWidgets/qwidget.h>

#include "ui_toolbar_fov.h"

#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

#include "models/OpenScanToolsModelEssentials.h"

class CameraNode;

class ToolBarFOV;

typedef void (ToolBarFOV::*fovToolBarMethod)(IGuiData*);

class ToolBarFOV : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarFOV(IDataDispatcher &dataDispatcher, QWidget *parent, float guiScale);
	~ToolBarFOV();

	// From IPanel
	void informData(IGuiData *keyValue);

private:
	void onNaviPanoramic(IGuiData* data);
	void onProjectLoad(IGuiData* data);
	void onActiveCamera(IGuiData* data);
	void onFocusViewport(IGuiData* data);

private slots:
	void slotFov(double fov);


private:
	Ui::ToolBarFOV m_ui;
	IDataDispatcher &m_dataDispatcher;
	std::unordered_map<guiDType, fovToolBarMethod> m_methods;

	SafePtr<CameraNode> m_focusCamera;
};

#endif // TOOLBAR_FOV_H

