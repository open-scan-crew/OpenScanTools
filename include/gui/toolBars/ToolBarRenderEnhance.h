#ifndef TOOLBAR_RENDER_ENHANCE_H
#define TOOLBAR_RENDER_ENHANCE_H

#include "ui_toolbar_renderenhancegroup.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "pointCloudEngine/RenderingTypes.h"
#include "utils/safe_ptr.h"

#include <unordered_map>

class CameraNode;

class ToolBarRenderEnhance : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarRenderEnhance(IDataDispatcher& dataDispatcher, QWidget* parent = 0, float guiScale = 1.f);
	~ToolBarRenderEnhance() override = default;

private:
	void informData(IGuiData* data) override;
	void onProjectLoad(IGuiData* data);
	void onActiveCamera(IGuiData* data);
	void onFocusViewport(IGuiData* data);
	void blockAllSignals(bool block);
	void updateEdgeAwareBlurUi(bool enabled);
	EdgeAwareBlur getEdgeAwareBlurFromUi() const;
	void updateDepthLiningUi(bool enabled);
	DepthLining getDepthLiningFromUi() const;

	typedef void (ToolBarRenderEnhance::* GuiDataFunction)(IGuiData*);
	inline void registerGuiDataFunction(guiDType type, GuiDataFunction fct)
	{
		m_dataDispatcher.registerObserverOnKey(this, type);
		m_methods.insert({ type, fct });
	};

	std::unordered_map<guiDType, GuiDataFunction> m_methods;
	Ui::toolbar_renderenhancegroup m_ui;
	IDataDispatcher& m_dataDispatcher;
	SafePtr<CameraNode> m_focusCamera;

private slots:
	void slotEdgeAwareBlurToggled(int state);
	void slotEdgeAwareBlurValueChanged(int value);
	void slotDepthLiningToggled(int state);
	void slotDepthLiningValueChanged(int value);
	void slotDepthLiningSensitivityChanged(int value);
	void slotDepthLiningStrongModeToggled(int state);
};

#endif // TOOLBAR_RENDER_ENHANCE_H
