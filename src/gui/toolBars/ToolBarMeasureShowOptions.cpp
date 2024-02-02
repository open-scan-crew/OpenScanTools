#include "gui/toolBars/ToolBarMeasureShowOptions.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"

ToolBarMeasureShowOptions::ToolBarMeasureShowOptions(IDataDispatcher &dataDispatcher, QWidget *parent, float guiScale)
    : QWidget(parent)
    , m_dataDispatcher(dataDispatcher)
{
    m_ui.setupUi(this);
    setEnabled(false);

    QObject::connect(m_ui.checkBox_showValues, &QCheckBox::stateChanged, this, &ToolBarMeasureShowOptions::showOptionsChanged);
    QObject::connect(m_ui.checkBox_total, &QCheckBox::stateChanged, this, &ToolBarMeasureShowOptions::showOptionsChanged);
    QObject::connect(m_ui.checkBox_horizontal, &QCheckBox::stateChanged, this, &ToolBarMeasureShowOptions::showOptionsChanged);
    QObject::connect(m_ui.checkBox_vertical, &QCheckBox::stateChanged, this, &ToolBarMeasureShowOptions::showOptionsChanged);

    m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
    m_methods.insert({ guiDType::projectLoaded, &ToolBarMeasureShowOptions::onProjectLoad });


}

ToolBarMeasureShowOptions::~ToolBarMeasureShowOptions()
{
    m_dataDispatcher.unregisterObserver(this);
}

void ToolBarMeasureShowOptions::informData(IGuiData* data)
{
    if (m_methods.find(data->getType()) != m_methods.end())
    {
        MeasureShowGroupMethod method = m_methods.at(data->getType());
        (this->*method)(data);
    }
}

void ToolBarMeasureShowOptions::onProjectLoad(IGuiData* data)
{
    GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
    setEnabled(plData->m_isProjectLoad);
}

void ToolBarMeasureShowOptions::showOptionsChanged()
{
    MeasureShowMask mask = 0;
    mask |= m_ui.checkBox_total->isChecked() ? SHOW_MAIN_SEGMENT : 0;
    mask |= m_ui.checkBox_horizontal->isChecked() ? SHOW_HORIZONTAL_SEGMENT : 0;
    mask |= m_ui.checkBox_vertical->isChecked() ? SHOW_VERTICAL_SEGMENT : 0;
    mask |= m_ui.checkBox_showValues->isChecked() ? SHOW_VALUES : 0;

    m_dataDispatcher.updateInformation(new GuiDataRenderMeasureOptions(mask, SafePtr<CameraNode>()), this);
}