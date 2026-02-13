#ifndef TOOLBAR_POLYGONAL_SELECTOR_H
#define TOOLBAR_POLYGONAL_SELECTOR_H

#include "ui_toolbar_selector.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "models/3d/DisplayParameters.h"

#include <unordered_map>

class ToolBarPolygonalSelector : public QWidget, public IPanel
{
    Q_OBJECT

public:
    explicit ToolBarPolygonalSelector(IDataDispatcher& dataDispatcher, QWidget* parent = nullptr, float guiScale = 1.f);
    ~ToolBarPolygonalSelector();

private:
    void informData(IGuiData* data) override;
    void onProjectLoad(IGuiData* data);
    void onActivatedFunctions(IGuiData* data);
    void onRenderSettings(IGuiData* data);

    void activateSelector();
    void applySettings(bool enabled);
    void resetSettings();
    void setManageMode(bool enabled);
    void onPolygonSelectionChanged(int index);
    void deleteSelectedPolygon();
    void refreshPolygonList();
    void sendSettingsUpdate();
    void clearManageSelection();

private:
    typedef void (ToolBarPolygonalSelector::*GuiDataMethod)(IGuiData*);
    std::unordered_map<guiDType, GuiDataMethod> m_methods;

    Ui::toolbar_selector m_ui;
    IDataDispatcher& m_dataDispatcher;
    PolygonalSelectorSettings m_settings;
};

#endif
