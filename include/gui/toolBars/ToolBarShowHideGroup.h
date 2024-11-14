#ifndef TOOLBAR_SHOWHIDEGROUP_H
#define TOOLBAR_SHOWHIDEGROUP_H

#include <QtWidgets/qwidget.h>
#include "ui_toolbar_showhidegroup.h"

#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

#include "pointCloudEngine/RenderingTypes.h"
#include "models/OpenScanToolsModelEssentials.h"

class CameraNode;

class ToolBarShowHideGroup : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarShowHideGroup(IDataDispatcher &dataDispatcher, QWidget *parent, float guiScale);
	~ToolBarShowHideGroup();

	// From IPanel
	void informData(IGuiData *keyValue);

private:
	typedef void (ToolBarShowHideGroup::* GuiDataFunction)(IGuiData*);
	inline void registerGuiDataFunction(guiDType type, GuiDataFunction fct)
	{
		m_dataDispatcher.registerObserverOnKey(this, type);
		m_methods.insert({ type, fct });
	};
	std::unordered_map<guiDType, GuiDataFunction> m_methods;

	void onProjectLoad(IGuiData *data);
	void onFocusViewport(IGuiData* data);
	void onActiveCamera(IGuiData* data);

	void toggleShowHideIcon(QToolButton* button, bool show);

private slots:
	void slotToogleShowScanMarkers();
	void slotToogleShowTagMarkers();
	void slotToogleShowViewpointMarkers();
	void slotToogleShowCurrentObjects();
	void slotToogleShowUncurrentObjects();
    void slotToogleShowMarkersText();
	void slotToogleShowMeasures();
	void slotToogleShowClippings();
	void slotToogleShowAll();
	void slotToogleShowPipes();
	void slotToogleShowPoints();

private:
	Ui::ToolBarShowHideGroup m_ui;
    IDataDispatcher &m_dataDispatcher;

	//bool m_showScanMarkers = true;
	bool m_showTagMarkers = true;
	bool m_showViewpointMarkers = true;
	bool m_showMeasure = true;
	bool m_showClippings = true;
	bool m_showPoints = true;
	bool m_showPipes = true;
	bool m_showAll = true;
    bool m_showObjectTexts = true;
	bool m_showSelected = true;
	bool m_showUnselected = true;
	SafePtr<CameraNode>	m_focusCamera;
};

#endif // TOOLBAR_SHOWHIDEGROUP_H

