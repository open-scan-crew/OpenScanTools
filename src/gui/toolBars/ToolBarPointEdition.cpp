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
    m_ui.toolButton_smoothPointCloud->setIconSize(QSize(20, 20) * guiScale);

    QObject::connect(m_ui.toolButton_deleteClippedPoints, &QToolButton::released, this, &ToolBarPointEdition::slotDeleteClippedPoints);
    QObject::connect(m_ui.toolButton_smoothPointCloud, &QToolButton::released, this, &ToolBarPointEdition::slotSmoothPointCloud);

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

void ToolBarPointEdition::slotSmoothPointCloud()
{
    const double maxDisplacementMm = m_ui.doubleSpinBox_smoothMaxDisplacement->value();
    const double voxelSizeMm = m_ui.comboBox_smoothVoxelSize->currentText().toDouble();
    const bool adaptiveVoxel = m_ui.checkBox_smoothAdaptiveVoxel->isChecked();
    const bool preserveEdges = m_ui.checkBox_smoothPreserveEdges->isChecked();

    m_dataDispatcher.sendControl(new control::exportPC::StartSmoothPointCloud(maxDisplacementMm, voxelSizeMm, adaptiveVoxel, preserveEdges));
}
