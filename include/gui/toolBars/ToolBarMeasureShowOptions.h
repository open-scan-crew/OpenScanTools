#ifndef TOOLBAR_MEASURE_SHOW_OPTIONS_H_
#define TOOLBAR_MEASURE_SHOW_OPTIONS_H_

#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "ui_toolbar_measureshowoptions.h"

#include <QtWidgets/qwidget.h>

class ToolBarMeasureShowOptions;

typedef void (ToolBarMeasureShowOptions::* MeasureShowGroupMethod)(IGuiData*);

class ToolBarMeasureShowOptions : public QWidget, public IPanel
{
    Q_OBJECT

public:
    ToolBarMeasureShowOptions(IDataDispatcher &dataDispatcher, QWidget *parent, float guiScale);
    ~ToolBarMeasureShowOptions();

    // From IPanel
    void informData(IGuiData *keyValue) override;

private:
    void showOptionsChanged();
    void onProjectLoad(IGuiData* keyValue);

private:
    std::unordered_map<guiDType, MeasureShowGroupMethod> m_methods;
    Ui::ToolBarMeasureShowOptions m_ui;
    IDataDispatcher &m_dataDispatcher;
};

#endif // !TOOLBAR_MEASURE_SHOW_OPTIONS_H_