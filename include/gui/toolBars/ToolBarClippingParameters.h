#ifndef TOOLBAR_CLIPPING_PARAMETERS_H
#define TOOLBAR_CLIPPING_PARAMETERS_H

#include "models/data/Clipping/ClippingTypes.h"
#include "ui_toolbar_clippingParameters.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include <QtWidgets/qwidget.h>

class ToolBarClippingParameters;

typedef void (ToolBarClippingParameters::*ClipParametersMethod)(IGuiData*);

class ToolBarClippingParameters : public QWidget, public IPanel
{
    Q_OBJECT

public:
    explicit ToolBarClippingParameters(IDataDispatcher &dataDispatcher, QWidget *parent, float scale);

    void informData(IGuiData *data) override;

private:
    void onProjectLoad(IGuiData *data);
    void onRenderUnitUsage(IGuiData* idata);
    void onValues(IGuiData* data);

public slots:
    void sendDefaultDistances();
    void updateDefaultClippingMode(ClippingMode mode);

private:
    ~ToolBarClippingParameters();
    void blockAllSignals(bool value);

private:
    std::unordered_map<guiDType, ClipParametersMethod> m_methods;

    Ui::toolbar_clippingParameters m_ui;
    IDataDispatcher &m_dataDispatcher;

};

#endif // TOOLBAR_CLIPPING_PARAMETERS_H
