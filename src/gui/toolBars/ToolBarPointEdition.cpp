#include "gui/toolBars/ToolBarPointEdition.h"
#include "controller/controls/ControlExportPC.h"
#include "gui/GuiData/GuiDataGeneralProject.h"

ToolBarPointEdition::ToolBarPointEdition(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale)
    : QWidget(parent)
    , m_dataDispatcher(dataDispatcher)
    , m_smoothWarning(this)
{
    m_ui.setupUi(this);
    setEnabled(false);

    m_ui.toolButton_deleteClippedPoints->setIconSize(QSize(20, 20) * guiScale);

    QObject::connect(m_ui.toolButton_deleteClippedPoints, &QToolButton::released, this, &ToolBarPointEdition::slotDeleteClippedPoints);
    QObject::connect(m_ui.checkBox_smoothPlaneFilter, &QCheckBox::toggled, this, &ToolBarPointEdition::slotPlaneFilterToggled);
    QObject::connect(m_ui.checkBox_smoothNormalFilter, &QCheckBox::toggled, this, &ToolBarPointEdition::slotNormalFilterToggled);
    QObject::connect(m_ui.pushButton_applySmooth, &QPushButton::released, this, &ToolBarPointEdition::slotApplySmoothPoints);

    // Keep the parameter controls aligned with the active smoothing filters.
    slotPlaneFilterToggled(m_ui.checkBox_smoothPlaneFilter->isChecked());
    slotNormalFilterToggled(m_ui.checkBox_smoothNormalFilter->isChecked());

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

void ToolBarPointEdition::slotPlaneFilterToggled(bool checked)
{
    m_ui.doubleSpinBox_smoothPlaneDistFactor->setEnabled(checked);
}

void ToolBarPointEdition::slotNormalFilterToggled(bool checked)
{
    m_ui.comboBox_smoothNormalAngle->setEnabled(checked);
}

void ToolBarPointEdition::slotApplySmoothPoints()
{
    const QString warning = QStringLiteral("Warning: points will be permanently moved. This action cannot be undone.\nDo you confirm?");
    if (!m_smoothWarning.confirmMessage(warning))
        return;

    SmoothPointsParameters params;
    params.maxDistanceMm = m_ui.doubleSpinBox_smoothMaxDistance->value();
    params.kNeighbors = m_ui.comboBox_smoothK->currentText().toInt();
    params.alpha = m_ui.comboBox_smoothAlpha->currentText().toDouble();
    params.planeFilterEnabled = m_ui.checkBox_smoothPlaneFilter->isChecked();
    params.planeDistanceFactor = m_ui.doubleSpinBox_smoothPlaneDistFactor->value();
    params.normalFilterEnabled = m_ui.checkBox_smoothNormalFilter->isChecked();
    params.normalAngleDeg = m_ui.comboBox_smoothNormalAngle->currentText().toDouble();
    params.keepBestPercent = m_ui.spinBox_smoothKeepPercent->value();

    // Irreversible: smoothing modifies the TLS files.
    m_dataDispatcher.sendControl(new control::exportPC::StartSmoothPoints(params));
}
