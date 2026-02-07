#ifndef TOOLBAR_COLORIMETRIC_FILTER_H
#define TOOLBAR_COLORIMETRIC_FILTER_H

#include "ui_toolbar_colorimetric_filter.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "models/3d/DisplayParameters.h"
#include "pointCloudEngine/RenderingTypes.h"
#include "utils/Color32.hpp"

#include <array>
#include <unordered_map>

class CameraNode;

class ToolBarColorimetricFilter : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarColorimetricFilter(IDataDispatcher& dataDispatcher, QWidget* parent = nullptr, float guiScale = 1.f);

private:
	void informData(IGuiData* data) override;
	void onProjectLoad(IGuiData* data);
	void onActiveCamera(IGuiData* data);
	void onFocusViewport(IGuiData* data);
	void onRenderMode(IGuiData* data);
	void onRenderColorimetricFilter(IGuiData* data);
	void onColorPickValue(IGuiData* data);

	void applySettings(bool enable);
	void resetSettings();
	void updateUiFromSettings(const ColorimetricFilterSettings& settings);
	void updatePreview();
	void updateFieldStates();
	void updateColorField(int index, const Color32& color, bool enabled);
	bool parseColorField(int index, Color32& outColor) const;
	bool parseIntensityField(Color32& outColor) const;
	ColorimetricFilterSettings readSettingsFromUi(bool keepEnabled) const;
	void setFieldPlaceholder(bool intensityMode);

	void startPick();

	typedef void (ToolBarColorimetricFilter::* GuiDataFunction)(IGuiData*);
	inline void registerGuiDataFunction(guiDType type, GuiDataFunction fct)
	{
		m_dataDispatcher.registerObserverOnKey(this, type);
		m_methods.insert({ type, fct });
	};

private:
	std::unordered_map<guiDType, GuiDataFunction> m_methods;
	Ui::toolbar_colorimetric_filter m_ui;
	IDataDispatcher& m_dataDispatcher;
	SafePtr<CameraNode> m_focusCamera;
	UiRenderMode m_currentRenderMode = UiRenderMode::RGB;
	ColorimetricFilterSettings m_settings;
};

#endif // TOOLBAR_COLORIMETRIC_FILTER_H
