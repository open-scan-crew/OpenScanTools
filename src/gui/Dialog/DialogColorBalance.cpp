#include "gui/Dialog/DialogColorBalance.h"

#include "controller/controls/ControlFunction.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataIO.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ExportTexts.hpp"
#include "gui/texts/FileSystemTexts.hpp"

#include <QtCore/QSignalBlocker>
#include <QtCore/qstandardpaths.h>
#include <QtWidgets/qfiledialog.h>

DialogColorBalance::DialogColorBalance(IDataDispatcher& dataDispatcher, QWidget* parent)
    : ADialog(dataDispatcher, parent)
{
    m_ui.setupUi(this);
    translateUI();

    connect(m_ui.toolButton_folder, &QToolButton::clicked, this, [this]()
    {
        QString initialPath = m_ui.lineEdit_folder->text().isEmpty() ? m_openPath : m_ui.lineEdit_folder->text();
        QString folderPath = QFileDialog::getExistingDirectory(this, tr("Open Directory"), initialPath, QFileDialog::ShowDirsOnly);
        if (!folderPath.isEmpty())
        {
            m_ui.lineEdit_folder->setText(folderPath);
            m_openPath = folderPath;
        }
    });
    connect(m_ui.pushButton_ok, &QPushButton::clicked, this, &DialogColorBalance::startFiltering);
    connect(m_ui.pushButton_cancel, &QPushButton::clicked, this, &DialogColorBalance::cancelFiltering);

    connect(m_ui.radioButton_balanceLow, &QRadioButton::toggled, this, [this](bool checked)
    {
        if (checked)
            m_strength = ColorBalanceStrength::Low;
    });
    connect(m_ui.radioButton_balanceMid, &QRadioButton::toggled, this, [this](bool checked)
    {
        if (checked)
            m_strength = ColorBalanceStrength::Mid;
    });
    connect(m_ui.radioButton_balanceHigh, &QRadioButton::toggled, this, [this](bool checked)
    {
        if (checked)
            m_strength = ColorBalanceStrength::High;
    });

    connect(m_ui.radioButton_separate, &QRadioButton::toggled, this, [this](bool checked)
    {
        if (checked)
            m_mode = ColorBalanceMode::Separate;
    });
    connect(m_ui.radioButton_global, &QRadioButton::toggled, this, [this](bool checked)
    {
        if (checked)
            m_mode = ColorBalanceMode::Global;
    });

    connect(m_ui.checkBox_balanceIntensityRGB, &QCheckBox::toggled, this, [this](bool checked)
    {
        m_applyOnRGBAndIntensity = checked;
    });

    m_openPath = QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::colorBalanceDialogDisplay);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::projectPath);
    syncUiFromValues();
    adjustSize();
}

DialogColorBalance::~DialogColorBalance()
{
    m_dataDispatcher.unregisterObserver(this);
}

void DialogColorBalance::informData(IGuiData* data)
{
    switch (data->getType())
    {
    case guiDType::colorBalanceDialogDisplay:
    {
        auto dialogData = static_cast<GuiDataColorBalanceDialogDisplay*>(data);
        m_hasBothComponents = dialogData->m_hasBothComponents;
        refreshUI();
        break;
    }
    case guiDType::projectPath:
    {
        auto dataType = static_cast<GuiDataProjectPath*>(data);
        m_openPath = QString::fromStdWString(dataType->m_path.wstring());
        break;
    }
    default:
        break;
    }
}

void DialogColorBalance::startFiltering()
{
    m_outputFolder = m_ui.lineEdit_folder->text().toStdWString();
    if (m_outputFolder.empty())
    {
        m_dataDispatcher.updateInformation(new GuiDataWarning(TEXT_NO_DIRECTORY_SELECTED));
        return;
    }

    bool openFolder = m_ui.checkBox_openFolderAfterExport->isChecked();
    m_dataDispatcher.sendControl(new control::function::ForwardMessage(new ColorBalanceMessage(m_strength, m_mode, m_applyOnRGBAndIntensity, m_outputFolder, openFolder)));

    hide();
}

void DialogColorBalance::cancelFiltering()
{
    m_dataDispatcher.sendControl(new control::function::Abort());
    hide();
}

void DialogColorBalance::translateUI()
{
    setWindowTitle(TEXT_EXPORT_TITLE_COLOR_BALANCE);
}

void DialogColorBalance::refreshUI()
{
    syncUiFromValues();
    show();
}

void DialogColorBalance::syncUiFromValues()
{
    const QSignalBlocker blockLow(m_ui.radioButton_balanceLow);
    const QSignalBlocker blockMid(m_ui.radioButton_balanceMid);
    const QSignalBlocker blockHigh(m_ui.radioButton_balanceHigh);
    const QSignalBlocker blockApply(m_ui.checkBox_balanceIntensityRGB);

    m_ui.radioButton_balanceLow->setChecked(m_strength == ColorBalanceStrength::Low);
    m_ui.radioButton_balanceMid->setChecked(m_strength == ColorBalanceStrength::Mid);
    m_ui.radioButton_balanceHigh->setChecked(m_strength == ColorBalanceStrength::High);

    m_ui.checkBox_balanceIntensityRGB->setEnabled(m_hasBothComponents);
    if (!m_hasBothComponents)
        m_applyOnRGBAndIntensity = false;
    m_ui.checkBox_balanceIntensityRGB->setChecked(m_applyOnRGBAndIntensity);
}
