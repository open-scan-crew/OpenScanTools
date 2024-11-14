#include "gui/toolbars/ToolBarClippingParameters.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/IDataDispatcher.h"

#define _USE_MATH_DEFINES
//#include <math.h>

ToolBarClippingParameters::ToolBarClippingParameters(IDataDispatcher &dataDispatcher, QWidget *parent, float guiScale)
    : QWidget(parent)
    , m_dataDispatcher(dataDispatcher)
{
    m_ui.setupUi(this);
    setEnabled(false);

    m_ui.lineEdit_defaultMin->setType(NumericType::DISTANCE);
    m_ui.lineEdit_defaultMax->setType(NumericType::DISTANCE);

    connect(m_ui.lineEdit_defaultMin, &QLineEdit::editingFinished, this, &ToolBarClippingParameters::sendDefaultDistances);
    connect(m_ui.lineEdit_defaultMax, &QLineEdit::editingFinished, this, &ToolBarClippingParameters::sendDefaultDistances);

    connect(m_ui.exteriorRadioButton, &QRadioButton::released, this, [this]() { updateDefaultClippingMode(ClippingMode::showExterior); });
    connect(m_ui.interiorRadioButton, &QRadioButton::released, this, [this]() { updateDefaultClippingMode(ClippingMode::showInterior); });

    m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::defaultClipParams);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::renderValueDisplay);

    m_methods.insert({ guiDType::projectLoaded, &ToolBarClippingParameters::onProjectLoad });
    m_methods.insert({ guiDType::defaultClipParams, &ToolBarClippingParameters::onValues });
    m_methods.insert({ guiDType::renderValueDisplay, &ToolBarClippingParameters::onRenderUnitUsage });
}

ToolBarClippingParameters::~ToolBarClippingParameters()
{
    m_dataDispatcher.unregisterObserver(this);
}

void ToolBarClippingParameters::informData(IGuiData *data)
{
    if (m_methods.find(data->getType()) != m_methods.end())
    {
        ClipParametersMethod method = m_methods.at(data->getType());
        (this->*method)(data);
    }
}

void ToolBarClippingParameters::onProjectLoad(IGuiData* data)
{
    GuiDataProjectLoaded* castData = static_cast<GuiDataProjectLoaded*>(data);
    setEnabled(castData->m_isProjectLoad);
}

void ToolBarClippingParameters::onRenderUnitUsage(IGuiData* idata)
{
    auto castdata = static_cast<GuiDataRenderUnitUsage*>(idata);

    m_ui.lineEdit_defaultMin->setUnit(castdata->m_valueParameters.distanceUnit);
    m_ui.lineEdit_defaultMax->setUnit(castdata->m_valueParameters.distanceUnit);
}

void ToolBarClippingParameters::onValues(IGuiData* data)
{
    GuiDataDefaultClipParams* castData = static_cast<GuiDataDefaultClipParams*>(data);

    blockAllSignals(true);

    m_ui.lineEdit_defaultMin->setValue(castData->m_minClipDistance);
    m_ui.lineEdit_defaultMax->setValue(castData->m_maxClipDistance);
    m_ui.exteriorRadioButton->setChecked(castData->m_mode == ClippingMode::showExterior);

    blockAllSignals(false);
}

void ToolBarClippingParameters::blockAllSignals(bool value)
{
    m_ui.lineEdit_defaultMin->blockSignals(value);
    m_ui.lineEdit_defaultMax->blockSignals(value);
}

void ToolBarClippingParameters::sendDefaultDistances()
{
    float min = m_ui.lineEdit_defaultMin->getValue();
    float max = m_ui.lineEdit_defaultMax->getValue();
    m_dataDispatcher.sendControl(new control::clippingEdition::SetDefaultMinClipDistance(min));
    m_dataDispatcher.sendControl(new control::clippingEdition::SetDefaultMaxClipDistance(max));
}

void ToolBarClippingParameters::updateDefaultClippingMode(ClippingMode mode)
{
    m_dataDispatcher.sendControl(new control::clippingEdition::SetDefaultClippingMode(mode));
}
