#ifndef TOOLBAR_RENDERINGS_H
#define TOOLBAR_RENDERINGS_H

#include "ui_toolbar_renderings.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "pointCloudEngine/RenderingTypes.h"
#include "utils/safe_ptr.h"

#include <unordered_map>

class CameraNode;

class ToolBarRenderings : public QWidget, public IPanel
{
	Q_OBJECT

public:
	explicit ToolBarRenderings(IDataDispatcher& dataDispatcher, QWidget* parent = 0, float guiScale = 1.f);
	~ToolBarRenderings();

	void switchRenderMode(const int& mode = 0);

private:
	void informData(IGuiData* data) override;
	void onProjectLoad(IGuiData* data);
	void onRenderBrightness(IGuiData* data);
	void onRenderContrast(IGuiData* data);
	void onRenderColorMode(IGuiData* data);
	void onRenderLuminance(IGuiData* data);
	void onRenderBlending(IGuiData* data);
	void onRenderPointSize(IGuiData* data);
	void onRenderSaturation(IGuiData* data);
	void onRenderAlphaObjects(IGuiData* data);
	void onRenderUnitUsage(IGuiData* data);
	void onActiveCamera(IGuiData* data);
	void onFocusViewport(IGuiData* data);
	void onRenderTransparency(IGuiData* data);

	void updateNormals(const PostRenderingNormals& normalsParams);
	void updateAmbientOcclusion(const PostRenderingAmbientOcclusion& aoParams);
	void updateAmbientOcclusionUi(bool aoEnabled);

	void blockAllSignals(bool block);
	void showContrastBrightness();
	void showSaturationLuminance();
	void enableFalseColor(bool enable);
	bool rampValidValue(float& min, float& max, int& step);
	void sendTransparency();
	void sendTransparencyOptions();
	void updateFlashControlState();
	void updateEdgeAwareBlurUi(bool enabled);
	EdgeAwareBlur getEdgeAwareBlurFromUi() const;
	void populateEdgeAwareResolutionCombo();
	void updateDepthLiningUi(bool enabled);
	DepthLining getDepthLiningFromUi() const;

	void changeEvent(QEvent* event) override;

private slots:
	void slotBrightnessLuminanceValueChanged(int value);
	void slotContrastSaturationValueChanged(int value);
	void slotTranparencyActivationChanged(int value);
	void slotTransparencyValueChanged(int value);
	void slotTransparencyOptionsChanged(int value);
	void slotFakeColorValueChanged(int value);
	void slotAlphaBoxesValueChanged(int value);
	void slotSetPointSize(int pointSize);
	void slotSetRenderMode(int mode);
	void slotColorPicking();
	void slotRampValues();
	void slotNormalsChanged();
	void slotSharpnessChanged(double value);
	void slotAmbientOcclusionChanged();
	void slotEdgeAwareBlurToggled(int state);
	void slotEdgeAwareBlurValueChanged(int value);
	void slotEdgeAwareBlurResolutionChanged(int index);
	void slotDepthLiningToggled(int state);
	void slotDepthLiningValueChanged(int value);
	void slotDepthLiningSensitivityChanged(int value);
	void slotDepthLiningStrongModeToggled(int state);

private:
	typedef void (ToolBarRenderings::*GuiDataFunction)(IGuiData*);
	inline void registerGuiDataFunction(guiDType type, GuiDataFunction fct)
	{
		m_dataDispatcher.registerObserverOnKey(this, type);
		m_methods.insert({ type, fct });
	};

	std::unordered_map<guiDType, GuiDataFunction> m_methods;
	Ui::toolbar_renderings m_ui;
	IDataDispatcher& m_dataDispatcher;
	UiRenderMode m_currentRenderMode;
	int m_brightness;
	int m_contrast;
	int m_lumiance;
	int m_saturation;
	int m_alphaBoxes;
	QColor m_selectedColor;
	bool m_intensityActive;
	SafePtr<CameraNode> m_focusCamera;
	bool m_transparencyActive = false;
};

#endif // TOOLBAR_RENDERINGS_H
