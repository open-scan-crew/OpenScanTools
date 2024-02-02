#include "gui/toolBars/ToolBarPointEdition.h"
#include "controller/controls/ControlExportPC.h"
#include "gui/GuiData/GuiDataGeneralProject.h"

ToolBarPointEdition::ToolBarPointEdition(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale)
    : QWidget(parent)
    , m_dataDispatcher(dataDispatcher)
{
    m_ui.setupUi(this);
    setEnabled(false);

    m_ui.toolButton_deleteClippedPoints->setIconSize(QSize(20, 20) * guiScale);

    QObject::connect(m_ui.toolButton_deleteClippedPoints, &QToolButton::released, this, &ToolBarPointEdition::slotDeleteClippedPoints);

    m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
    m_guiDataFunctions.insert({ guiDType::projectLoaded, &ToolBarPointEdition::onProjectLoad });


}

ToolBarPointEdition::~ToolBarPointEdition()
{
    m_dataDispatcher.unregisterObserver(this);
}

void ToolBarPointEdition::informData(IGuiData *data)
{
    if (m_guiDataFunctions.find(data->getType()) != m_guiDataFunctions.end())
    {
        GuiDataFct method = m_guiDataFunctions.at(data->getType());
        (this->*method)(data);
    }
}

void ToolBarPointEdition::onProjectLoad(IGuiData * data)
{
    GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
    setEnabled(plData->m_isProjectLoad);
}

void ToolBarPointEdition::slotDeleteClippedPoints()
{
    m_dataDispatcher.sendControl(new control::exportPC::StartDeletePoints());
}
