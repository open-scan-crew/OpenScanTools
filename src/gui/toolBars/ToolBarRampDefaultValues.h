#ifndef TOOLBAR_RAMP_DEFAULT_VALUES_H
#define TOOLBAR_RAMP_DEFAULT_VALUES_H

#include <QtWidgets/QWidget>

#include "ui_toolbar_ramp_default_values.h"

#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"

#include "models/OpenScanToolsModelEssentials.h"

class ToolBarRampDefaultValues : public QWidget, public IPanel
{
    Q_OBJECT

public:
    explicit ToolBarRampDefaultValues(IDataDispatcher& dataDispatcher, QWidget* parent, float guiScale);
    ~ToolBarRampDefaultValues();

    // From IPanel
    void informData(IGuiData* keyValue);

private:
    void onProjectLoad(IGuiData* idata);
    void onRenderUnitUsage(IGuiData* idata);
    void onDefaultValues(IGuiData* idata);
    void sendDefaultValues();

private:
    Ui::toolbar_ramp_default_values m_ui;
    IDataDispatcher& m_dataDispatcher;

    typedef void (ToolBarRampDefaultValues::*GuiDataFunction)(IGuiData*);
    std::unordered_map<guiDType, GuiDataFunction> m_methods;
    inline void registerGuiDataFunction(guiDType type, GuiDataFunction fct)
    {
        m_dataDispatcher.registerObserverOnKey(this, type);
        m_methods.insert({ type, fct });
    };

};

#endif // TOOLBAR_RAMP_DEFAULT_VALUES_H
