#ifndef TOOLBAR_RENDERING_ADVANCED_H
#define TOOLBAR_RENDERING_ADVANCED_H

#include "ui_toolbar_renderingadvancedgroup.h"

#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "pointCloudEngine/RenderingTypes.h"

#include "utils/safe_ptr.h"

#include <unordered_map>

class CameraNode;

class ToolBarRenderingAdvanced : public QWidget, public IPanel
{
    Q_OBJECT

public:
    explicit ToolBarRenderingAdvanced(IDataDispatcher& dataDispatcher, QWidget* parent = nullptr, float guiScale = 1.f);
    ~ToolBarRenderingAdvanced() {};

private:
    void informData(IGuiData* data) override;
    void onProjectLoad(IGuiData* data);
    void onActiveCamera(IGuiData* data);
    void onFocusViewport(IGuiData* data);

    void updateBillboardUi(const BillboardRendering& billboardSettings);
    void updateEyeDomeUi(const EyeDomeLighting& edlSettings);
    void blockAllSignals(bool block);

    BillboardRendering getBillboardFromUi() const;
    EyeDomeLighting getEdlFromUi() const;

    void sendBillboard();
    void sendEdl();

private slots:
    void slotBillboardToggled(int state);
    void slotBillboardFeatherSlider(int value);
    void slotBillboardFeatherSpin(double value);
    void slotAdaptiveBillboardToggled(int state);
    void slotAdaptiveStrengthSlider(int value);
    void slotAdaptiveStrengthSpin(double value);

    void slotEdlToggled(int state);
    void slotEdlStrengthSlider(int value);
    void slotEdlStrengthSpin(double value);
    void slotEdlRadiusSlider(int value);
    void slotEdlRadiusSpin(double value);
    void slotEdlBiasSlider(int value);
    void slotEdlBiasSpin(double value);

private:
    typedef void (ToolBarRenderingAdvanced::*GuiDataFunction)(IGuiData*);
    std::unordered_map<guiDType, GuiDataFunction> m_methods;

    inline void registerGuiDataFunction(guiDType type, GuiDataFunction fct)
    {
        m_dataDispatcher.registerObserverOnKey(this, type);
        m_methods.insert({ type, fct });
    };

    Ui::toolbar_renderingadvancedgroup m_ui;
    IDataDispatcher& m_dataDispatcher;
    SafePtr<CameraNode> m_focusCamera;
};

#endif // TOOLBAR_RENDERING_ADVANCED_H
