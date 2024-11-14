#ifndef TOOLBAR_CLIPPINGPARAMETERS_H
#define TOOLBAR_CLIPPINGPARAMETERS_H

#include <QtWidgets/qwidget.h>
#include "controller/controls/ControlClippingEdition.h"
#include "ui_toolbar_clippingParameters.h"
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "gui/UnitConverter.h"
#include "utils/Color32.hpp"

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

#endif // TOOLBAR_CLIPPINGPARAMETERS_H

