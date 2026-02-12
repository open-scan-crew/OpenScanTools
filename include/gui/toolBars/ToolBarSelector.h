#ifndef TOOLBAR_SELECTOR_H
#define TOOLBAR_SELECTOR_H

#include <QtWidgets/qwidget.h>
#include "ui_toolbar_selector.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "models/3d/DisplayParameters.h"

#include <unordered_map>

class ToolBarSelector : public QWidget, public IPanel
{
    Q_OBJECT

public:
    explicit ToolBarSelector(IDataDispatcher& dataDispatcher, QWidget* parent, float guiScale);
    void informData(IGuiData* data) override;

private:
    ~ToolBarSelector();

    void onProjectLoad(IGuiData* data);
    void onFocusViewport(IGuiData* data);
    void onActiveCamera(IGuiData* data);
    void onRenderSelector(IGuiData* data);

    void applySettings(bool enabled);
    void resetSettings();
    void refreshUiFromSettings();

    typedef void (ToolBarSelector::*GuiDataFunction)(IGuiData*);
    inline void registerGuiDataFunction(guiDType type, GuiDataFunction fct)
    {
        m_dataDispatcher.registerObserverOnKey(this, type);
        m_methods.insert({ type, fct });
    }

private:
    std::unordered_map<guiDType, GuiDataFunction> m_methods;
    Ui::toolbar_selector m_ui;
    IDataDispatcher& m_dataDispatcher;
    SafePtr<CameraNode> m_focusCamera;
    PolygonalSelectorSettings m_settings;
};

#endif
