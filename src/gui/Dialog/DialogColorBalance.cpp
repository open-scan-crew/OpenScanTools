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

namespace
{
    struct ColorBalancePresetValues
    {
        int photometricDepth;
        int nMin;
        int nGood;
        double beta;
        int smoothingLevels;
    };

    constexpr ColorBalancePresetValues kPresetLow{6, 200, 1500, 0.7, 5};
    constexpr ColorBalancePresetValues kPresetMid{7, 150, 1000, 0.65, 4};
    constexpr ColorBalancePresetValues kPresetHigh{8, 100, 800, 0.6, 3};
}

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
    connect(m_ui.pushButton_ok, &QPushButton::clicked, this, &DialogColorBalance::startColorBalance);
    connect(m_ui.pushButton_cancel, &QPushButton::clicked, this, &DialogColorBalance::cancelColorBalance);

    connect(m_ui.radioButton_cbLow, &QRadioButton::toggled, this, [this](bool checked)
    {
        if (checked)
            applyPreset(ColorBalancePreset::Low);
    });
    connect(m_ui.radioButton_cbMid, &QRadioButton::toggled, this, [this](bool checked)
    {
        if (checked)
            applyPreset(ColorBalancePreset::Mid);
    });
    connect(m_ui.radioButton_cbHigh, &QRadioButton::toggled, this, [this](bool checked)
    {
        if (checked)
            applyPreset(ColorBalancePreset::High);
    });

    connect(m_ui.spinBox_photometricDepth, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value)
    {
        m_photometricDepth = value;
    });
    connect(m_ui.spinBox_nMin, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value)
    {
        m_nMin = value;
    });
    connect(m_ui.spinBox_nGood, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value)
    {
        m_nGood = value;
    });
    connect(m_ui.doubleSpinBox_beta, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double value)
    {
        m_beta = value;
    });
    connect(m_ui.spinBox_smoothingLevels, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value)
    {
        m_smoothingLevels = value;
    });
    connect(m_ui.checkBox_applyIntensity, &QCheckBox::toggled, this, [this](bool checked)
    {
        m_applyIntensityWhenAvailable = checked;
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

void DialogColorBalance::startColorBalance()
{
    m_outputFolder = m_ui.lineEdit_folder->text().toStdWString();
    if (m_outputFolder.empty())
    {
        m_dataDispatcher.updateInformation(new GuiDataWarning(TEXT_NO_DIRECTORY_SELECTED));
        return;
    }

    int photometricDepth = m_ui.spinBox_photometricDepth->value();
    int nMin = m_ui.spinBox_nMin->value();
    int nGood = m_ui.spinBox_nGood->value();
    double beta = m_ui.doubleSpinBox_beta->value();
    int smoothingLevels = m_ui.spinBox_smoothingLevels->value();
    bool applyIntensity = m_ui.checkBox_applyIntensity->isChecked();
    bool openFolder = m_ui.checkBox_openFolderAfterExport->isChecked();

    m_dataDispatcher.sendControl(new control::function::ForwardMessage(new ColorBalanceMessage(photometricDepth, nMin, nGood, beta, smoothingLevels, applyIntensity, m_outputFolder, openFolder)));

    hide();
}

void DialogColorBalance::cancelColorBalance()
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

void DialogColorBalance::applyPreset(ColorBalancePreset preset)
{
    m_preset = preset;

    ColorBalancePresetValues values = kPresetMid;
    switch (preset)
    {
    case ColorBalancePreset::Low:
        values = kPresetLow;
        break;
    case ColorBalancePreset::Mid:
        values = kPresetMid;
        break;
    case ColorBalancePreset::High:
        values = kPresetHigh;
        break;
    default:
        break;
    }

    m_photometricDepth = values.photometricDepth;
    m_nMin = values.nMin;
    m_nGood = values.nGood;
    m_beta = values.beta;
    m_smoothingLevels = values.smoothingLevels;

    syncUiFromValues();
}

void DialogColorBalance::syncUiFromValues()
{
    const QSignalBlocker blockLow(m_ui.radioButton_cbLow);
    const QSignalBlocker blockMid(m_ui.radioButton_cbMid);
    const QSignalBlocker blockHigh(m_ui.radioButton_cbHigh);
    const QSignalBlocker blockDepth(m_ui.spinBox_photometricDepth);
    const QSignalBlocker blockNMin(m_ui.spinBox_nMin);
    const QSignalBlocker blockNGood(m_ui.spinBox_nGood);
    const QSignalBlocker blockBeta(m_ui.doubleSpinBox_beta);
    const QSignalBlocker blockLevels(m_ui.spinBox_smoothingLevels);
    const QSignalBlocker blockIntensity(m_ui.checkBox_applyIntensity);

    m_ui.radioButton_cbLow->setChecked(m_preset == ColorBalancePreset::Low);
    m_ui.radioButton_cbMid->setChecked(m_preset == ColorBalancePreset::Mid);
    m_ui.radioButton_cbHigh->setChecked(m_preset == ColorBalancePreset::High);

    m_ui.spinBox_photometricDepth->setValue(m_photometricDepth);
    m_ui.spinBox_nMin->setValue(m_nMin);
    m_ui.spinBox_nGood->setValue(m_nGood);
    m_ui.doubleSpinBox_beta->setValue(m_beta);
    m_ui.spinBox_smoothingLevels->setValue(m_smoothingLevels);
    m_ui.checkBox_applyIntensity->setChecked(m_applyIntensityWhenAvailable);
}
