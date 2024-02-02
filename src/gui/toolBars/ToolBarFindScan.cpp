#include "gui/toolBars/ToolBarFindScan.h"
#include "controller/controls/ControlPicking.h"
#include "gui/GuiData/GuiDataGeneralProject.h"

ToolBarFindScan::ToolBarFindScan(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale)
    : QWidget(parent)
    , m_dataDispatcher(dataDispatcher)
{
    m_ui.setupUi(this);
    setEnabled(false);

    QObject::connect(m_ui.pushButton_findScan, &QPushButton::released, this, &ToolBarFindScan::slotFindScan);

    m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
    m_guiDataFunctions.insert({ guiDType::projectLoaded, &ToolBarFindScan::onProjectLoad });


}

ToolBarFindScan::~ToolBarFindScan()
{
    m_dataDispatcher.unregisterObserver(this);
}

void ToolBarFindScan::informData(IGuiData *data)
{
    if (m_guiDataFunctions.find(data->getType()) != m_guiDataFunctions.end())
    {
        GuiDataFct method = m_guiDataFunctions.at(data->getType());
        (this->*method)(data);
    }
}

void ToolBarFindScan::onProjectLoad(IGuiData * data)
{
    GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
    setEnabled(plData->m_isProjectLoad);
}

void ToolBarFindScan::slotFindScan()
{
    m_dataDispatcher.sendControl(new control::picking::FindScanFromPick());
}
