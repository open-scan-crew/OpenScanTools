#ifndef TOOLBAR_RENDER_TRANSPARENCY_H
#define TOOLBAR_RENDER_TRANSPARENCY_H

#include "ui_toolbar_rendertransparencygroup.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

#include "utils/safe_ptr.h"

class CameraNode;

class ToolBarRenderTransparency : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarRenderTransparency(IDataDispatcher& dataDispatcher, QWidget* parent = 0, float guiScale = 1.f);
	~ToolBarRenderTransparency();

private:
	void informData(IGuiData* data) override;
	void onProjectLoad(IGuiData* data);
	void onActiveCamera(IGuiData* data);
	void onFocusViewport(IGuiData* data);

	typedef void (ToolBarRenderTransparency::* GuiDataFunction)(IGuiData*);
	inline void registerGuiDataFunction(guiDType type, GuiDataFunction fct)
	{
		m_dataDispatcher.registerObserverOnKey(this, type);
		m_methods.insert({ type, fct });
	};

	void blockAllSignals(bool block);
	void enableUI(bool transparencyActive);

    void sendTransparency();
    void sendTransparencyOptions();

private slots:
    void slotTranparencyActivationChanged(int value);
    void slotTransparencyValueChanged(int value);
    void slotTransparencyOptionsChanged(int value);

private:
	std::unordered_map<guiDType, GuiDataFunction> m_methods;
	Ui::toolbar_rendertransparencygroup m_ui;
	IDataDispatcher& m_dataDispatcher;
	SafePtr<CameraNode>	m_focusCamera;
};

#endif // TOOLBAR_RENDER_TRANSPARENCY_H
