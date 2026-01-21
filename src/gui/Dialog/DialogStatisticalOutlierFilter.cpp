#include "gui/Dialog/DialogStatisticalOutlierFilter.h"

#include "controller/controls/ControlFunction.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/texts/ExportTexts.hpp"

DialogStatisticalOutlierFilter::DialogStatisticalOutlierFilter(IDataDispatcher& dataDispatcher, QWidget* parent)
    : ADialog(dataDispatcher, parent)
{
    m_ui.setupUi(this);
    translateUI();

    connect(m_ui.pushButton_ok, &QPushButton::clicked, this, &DialogStatisticalOutlierFilter::startFiltering);
    connect(m_ui.pushButton_cancel, &QPushButton::clicked, this, &DialogStatisticalOutlierFilter::cancelFiltering);

    connect(m_ui.radioButton_separate, &QRadioButton::toggled, this, [this](bool checked)
    {
        if (checked)
            m_mode = OutlierFilterMode::Separate;
    });
    connect(m_ui.radioButton_global, &QRadioButton::toggled, this, [this](bool checked)
    {
        if (checked)
            m_mode = OutlierFilterMode::Global;
    });

    m_dataDispatcher.registerObserverOnKey(this, guiDType::statisticalOutlierFilterDialogDisplay);
    adjustSize();
}

DialogStatisticalOutlierFilter::~DialogStatisticalOutlierFilter()
{
    m_dataDispatcher.unregisterObserver(this);
}

void DialogStatisticalOutlierFilter::informData(IGuiData* data)
{
    switch (data->getType())
    {
    case guiDType::statisticalOutlierFilterDialogDisplay:
        refreshUI();
        break;
    default:
        break;
    }
}

void DialogStatisticalOutlierFilter::startFiltering()
{
    int kNeighbors = m_ui.spinBox_kNeighbors->value();
    double nSigma = m_ui.doubleSpinBox_nSigma->value();

    m_dataDispatcher.sendControl(new control::function::ForwardMessage(new StatisticalOutlierFilterMessage(kNeighbors, nSigma, m_mode)));

    hide();
}

void DialogStatisticalOutlierFilter::cancelFiltering()
{
    m_dataDispatcher.sendControl(new control::function::Abort());
    hide();
}

void DialogStatisticalOutlierFilter::translateUI()
{
    setWindowTitle(TEXT_EXPORT_TITLE_STAT_OUTLIER);
}

void DialogStatisticalOutlierFilter::refreshUI()
{
    show();
}
