#include "gui/Dialog/DialogStatisticalOutlierFilter.h"

#include "controller/controls/ControlFunction.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataIO.h"
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
        QString initialPath = m_ui.lineEdit_folder->text().isEmpty() ? m_openPath : m_ui.lineEdit_folder->text();
        QString folderPath = QFileDialog::getExistingDirectory(this, tr("Open Directory"), initialPath, QFileDialog::ShowDirsOnly);
        if (!folderPath.isEmpty())
        {
            m_ui.lineEdit_folder->setText(folderPath);
            m_openPath = folderPath;
        }
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
    m_dataDispatcher.registerObserverOnKey(this, guiDType::projectPath);
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
    case guiDType::projectPath:
    {
        auto dataType = static_cast<GuiDataProjectPath*>(data);
        m_openPath = QString::fromStdWString(dataType->m_path.wstring());
    }
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

    bool openFolder = m_ui.checkBox_openFolderAfterExport->isChecked();
    m_dataDispatcher.sendControl(new control::function::ForwardMessage(new StatisticalOutlierFilterMessage(kNeighbors, nSigma, m_mode, m_outputFolder, openFolder)));

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
