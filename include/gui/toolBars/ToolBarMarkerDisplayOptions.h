#ifndef TOOLBAR_MARKER_DISPLAY_OPTIONS_H
#define TOOLBAR_MARKER_DISPLAY_OPTIONS_H

//#include <QtWidgets/QWidget>
#include "ui_toolbar_markerdisplayoptions.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "gui/UnitConverter.h"
#include "pointCloudEngine/RenderingTypes.h"

#include "models/OpenScanToolsModelEssentials.h"

class CameraNode;

class ToolBarMarkerDisplayOptions : public QWidget, public IPanel
{
    Q_OBJECT

public:
    explicit ToolBarMarkerDisplayOptions(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale);
    ~ToolBarMarkerDisplayOptions();


    void onProjectLoad(IGuiData *data);
    void onInitOptions(IGuiData *data);
    void onActiveCamera(IGuiData* data);
    void onFocusViewport(IGuiData* data);

private:
    void informData(IGuiData *data) override;
    typedef void (ToolBarMarkerDisplayOptions::*GuiDataFunction)(IGuiData*);
    inline void registerGuiDataFunction(guiDType type, GuiDataFunction fct)
    {
        m_dataDispatcher.registerObserverOnKey(this, type);
        m_functions.insert({ type, fct });
    };

	void updateUI();

private slots:
    void sendOptions();

private:
    Ui::ToolBarMarkerDisplayOptions m_ui;
    IDataDispatcher &m_dataDispatcher;
    std::unordered_map<guiDType, GuiDataFunction> m_functions;
    SafePtr<CameraNode>       m_focusCamera;
	MarkerDisplayOptions m_storedParameters;
};

#endif
