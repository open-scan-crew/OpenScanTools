#include "gui/toolBars/ToolBarRampDefaultValues.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "controller/controls/ControlClippingEdition.h"

ToolBarRampDefaultValues::ToolBarRampDefaultValues(IDataDispatcher& dataDispatcher, QWidget* parent, float guiScale)
    : QWidget(parent)
    , m_dataDispatcher(dataDispatcher)
{
    m_ui.setupUi(this);
    setEnabled(false);

    connect(m_ui.spinBox_steps, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this](int) { this->sendDefaultValues(); });
    connect(m_ui.lineEdit_max, &QLineEdit::editingFinished , [this]() { this->sendDefaultValues(); });
    connect(m_ui.lineEdit_min, &QLineEdit::editingFinished, [this]() { this->sendDefaultValues(); });

    registerGuiDataFunction(guiDType::projectLoaded, &ToolBarRampDefaultValues::onProjectLoad);
    registerGuiDataFunction(guiDType::renderValueDisplay, &ToolBarRampDefaultValues::onRenderUnitUsage);
    registerGuiDataFunction(guiDType::defaultRampParams, &ToolBarRampDefaultValues::onDefaultValues);
}

ToolBarRampDefaultValues::~ToolBarRampDefaultValues()
{
    m_dataDispatcher.unregisterObserver(this);
}

void ToolBarRampDefaultValues::informData(IGuiData* data)
{
    if (m_methods.find(data->getType()) != m_methods.end())
    {
        GuiDataFunction method = m_methods.at(data->getType());
        (this->*method)(data);
    }
}

void ToolBarRampDefaultValues::onProjectLoad(IGuiData* data)
{
    GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
    setEnabled(plData->m_isProjectLoad);
}

void ToolBarRampDefaultValues::onRenderUnitUsage(IGuiData* idata)
{
    auto castdata = static_cast<GuiDataRenderUnitUsage*>(idata);

    m_ui.lineEdit_min->setUnit(castdata->m_valueParameters.distanceUnit);
    m_ui.lineEdit_max->setUnit(castdata->m_valueParameters.distanceUnit);
}

void ToolBarRampDefaultValues::onDefaultValues(IGuiData* idata)
{
    auto castdata = static_cast<GuiDataDefaultRampParams*>(idata);
    m_ui.lineEdit_min->setValue(castdata->m_minRampDistance);
    m_ui.lineEdit_max->setValue(castdata->m_maxRampDistance);
    m_ui.spinBox_steps->setValue(castdata->m_steps);
}

void ToolBarRampDefaultValues::sendDefaultValues()
{
    float minV = m_ui.lineEdit_min->getValue();
    float maxV = m_ui.lineEdit_max->getValue();
    int steps = m_ui.spinBox_steps->value();

    m_dataDispatcher.sendControl(new control::clippingEdition::SetDefaultRampValues(minV, maxV, steps));
}
