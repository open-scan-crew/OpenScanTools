#include "gui/Dialog/DialogStatisticalOutlierFilter.h"

#include "controller/controls/ControlFunction.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ExportTexts.hpp"
#include "gui/texts/FileSystemTexts.hpp"

#include <QtCore/qstandardpaths.h>
#include <QtWidgets/qfiledialog.h>

DialogStatisticalOutlierFilter::DialogStatisticalOutlierFilter(IDataDispatcher& dataDispatcher, QWidget* parent)
    : ADialog(dataDispatcher, parent)
{
    m_ui.setupUi(this);
    translateUI();

    connect(m_ui.toolButton_folder, &QToolButton::released, this, [this]()
    {
        QString folderPath = QFileDialog::getExistingDirectory(this, tr("Open Directory"), m_openPath, QFileDialog::ShowDirsOnly);
        if (!folderPath.isEmpty())
            m_ui.lineEdit_folder->setText(folderPath);
    });
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

    m_openPath = QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory);
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
    m_outputFolder = m_ui.lineEdit_folder->text().toStdWString();
    if (m_outputFolder.empty())
    {
        m_dataDispatcher.updateInformation(new GuiDataWarning(TEXT_NO_DIRECTORY_SELECTED));
        return;
    }

    int kNeighbors = m_ui.spinBox_kNeighbors->value();
    double nSigma = m_ui.doubleSpinBox_nSigma->value();

    m_dataDispatcher.sendControl(new control::function::ForwardMessage(new StatisticalOutlierFilterMessage(kNeighbors, nSigma, m_mode, m_outputFolder)));

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
