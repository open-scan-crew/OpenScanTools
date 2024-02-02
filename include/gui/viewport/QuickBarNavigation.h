#ifndef QUICK_BAR_NAVIGATION_H
#define QUICK_BAR_NAVIGATION_H

#include "gui/IDataDispatcher.h"
#include "gui/IPanel.h"
#include "pointCloudEngine/RenderingTypes.h"

#include "crossguid/Guid.hpp"
#include <qwidget.h>
#include "ui_quickbar_navigation.h"
#include "models/OpenScanToolsModelEssentials.h"

class CameraNode;

class VulkanViewport;

enum class ManipulationMode;

class QuickBarNavigation : public QWidget, public IPanel
{
    Q_OBJECT

public:
    QuickBarNavigation(QWidget* parent, IDataDispatcher& dataDispatcher, float guiScale);
    ~QuickBarNavigation();

    void connectViewport(VulkanViewport* viewport);
    void connectCamera(SafePtr<CameraNode> camera);

signals:
    void alignViewSelected(AlignView align);

public slots:
    void onActiveExamine(bool isActive);

    void onFullScreenMode(bool fullScreenActivated);
    void onToggleVisible();

    void onManipulationMode(ManipulationMode mode);

    void onMoveManip();

private:
    void informData(IGuiData* guiData) override;
    typedef void (QuickBarNavigation::*GuiDataFunction)(IGuiData*);
    inline void registerGuiDataFunction(guiDType type, GuiDataFunction fct)
    {
        m_dataDispatcher.registerObserverOnKey(this, type);
        m_functions.insert({ type, fct });
    };

    void onProjectLoaded(IGuiData* data);
    void onActivateFunction(IGuiData* data);
    void onGuizmoDisplay(IGuiData* data);
    void onCameraToViewpoint(IGuiData* data);

    void onUpdateManipulationMode(IGuiData* data);

    void blockSignals_camera(bool block);
    void blockSignals_functions(bool block);

private:
    Ui::QuickBarNavigation m_ui;
    IDataDispatcher& m_dataDispatcher;
    std::unordered_map<guiDType, GuiDataFunction> m_functions; 
    SafePtr<CameraNode> m_camera;
    bool m_fullScreenActive;

};

#endif
