#ifndef TOOLBAR_RENDER_RAMP_GROUP_H
#define TOOLBAR_RENDER_RAMP_GROUP_H

#include "ui_toolbar_render_ramp_group.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

#include "pointCloudEngine/GuiRenderingTypes.h"

#include "models/OpenScanToolsModelEssentials.h"

class CameraNode;

class ToolBarRenderRampGroup : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarRenderRampGroup(IDataDispatcher& dataDispatcher, QWidget* parent = 0, float guiScale = 1.f);
	~ToolBarRenderRampGroup() {};

private:
	void informData(IGuiData* data) override;
	void onProjectLoad(IGuiData* data);
	void onActiveCamera(IGuiData* data);
	void onFocusViewport(IGuiData* data);

	void updateRamp(const RampScale& normalsParams);
	void blockAllSignals(bool block);

	void sendGuiData();

private:
	typedef void (ToolBarRenderRampGroup::* GuiDataFunction)(IGuiData*);
	std::unordered_map<guiDType, GuiDataFunction> m_methods;

	inline void registerGuiDataFunction(guiDType type, GuiDataFunction fct)
	{
		m_dataDispatcher.registerObserverOnKey(this, type);
		m_methods.insert({ type, fct });
	};

	Ui::toolbar_render_ramp_group m_ui;
	IDataDispatcher& m_dataDispatcher;
	SafePtr<CameraNode> m_focusCamera;

};

#endif // TOOLBAR_RENDERSETTINGS_H
