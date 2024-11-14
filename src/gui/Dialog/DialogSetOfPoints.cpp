#include "gui/Dialog/DialogSetOfPoints.h"


DialogSetOfPoints::DialogSetOfPoints(IDataDispatcher& dataDispatcher, QWidget* parent)
    : ADialog(dataDispatcher, parent)
{
    m_ui.setupUi(this);

    connect(m_ui.okButton, &QPushButton::clicked, this, &DialogSetOfPoints::onOk);
    connect(m_ui.cancelButton, &QPushButton::clicked, this, &DialogSetOfPoints::onCancel);

    m_ui.stepDoubleEdit->setType(NumericType::DISTANCE);
    m_ui.thresholdDoubleEdit->setType(NumericType::DISTANCE);
}

DialogSetOfPoints::~DialogSetOfPoints()
{
}

void DialogSetOfPoints::onCancel()
{
    hide();
}

void DialogSetOfPoints::onOk()
{
    double step = m_ui.stepDoubleEdit->getValue();
    double threshold = m_ui.thresholdDoubleEdit->getValue();

    //m_dataDispatcher.sendControl(new control::measure::ActivateSetOfPoints(step, threshold));
    hide();
}

void DialogSetOfPoints::informData(IGuiData* keyValue)
{
}
