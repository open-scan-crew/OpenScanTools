#include "gui/Dialog/DialogColorBalanceFilter.h"

#include "controller/controls/ControlFunction.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataIO.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ExportTexts.hpp"
#include "gui/texts/FileSystemTexts.hpp"

#include <QtCore/QSignalBlocker>
#include <QtCore/QStandardPaths>
#include <QtWidgets/QFileDialog>

namespace
{
    struct BalancePresetValues
    {
        int kMin;
        int kMax;
        double trimPercent;
    };

    constexpr BalancePresetValues kSeparateLight{12, 20, 12.0};
    constexpr BalancePresetValues kSeparateMedium{24, 40, 18.0};
    constexpr BalancePresetValues kSeparateStrong{48, 80, 25.0};

    constexpr BalancePresetValues kGlobalLight{16, 24, 12.0};
    constexpr BalancePresetValues kGlobalMedium{32, 48, 18.0};
    constexpr BalancePresetValues kGlobalStrong{64, 96, 25.0};

    BalancePresetValues presetForMode(ColorBalanceMode mode, DialogColorBalanceFilter::BalancePreset preset)
    {
        if (mode == ColorBalanceMode::Global)
        {
            switch (preset)
            {
            case DialogColorBalanceFilter::BalancePreset::Light:
                return kGlobalLight;
            case DialogColorBalanceFilter::BalancePreset::Strong:
                return kGlobalStrong;
            case DialogColorBalanceFilter::BalancePreset::Medium:
            default:
                return kGlobalMedium;
            }
        }

        switch (preset)
        {
        case DialogColorBalanceFilter::BalancePreset::Light:
            return kSeparateLight;
        case DialogColorBalanceFilter::BalancePreset::Strong:
            return kSeparateStrong;
        case DialogColorBalanceFilter::BalancePreset::Medium:
        default:
            return kSeparateMedium;
        }
    }
}

DialogColorBalanceFilter::DialogColorBalanceFilter(IDataDispatcher& dataDispatcher, QWidget* parent)
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

    connect(m_ui.pushButton_ok, &QPushButton::clicked, this, &DialogColorBalanceFilter::startBalancing);
    connect(m_ui.pushButton_cancel, &QPushButton::clicked, this, &DialogColorBalanceFilter::cancelBalancing);

    connect(m_ui.radioButton_separate, &QRadioButton::toggled, this, [this](bool checked)
    {
        if (!checked)
            return;
        m_mode = ColorBalanceMode::Separate;
        applyPreset(m_preset);
    });
    connect(m_ui.radioButton_global, &QRadioButton::toggled, this, [this](bool checked)
    {
        if (!checked)
            return;
        m_mode = ColorBalanceMode::Global;
        applyPreset(m_preset);
    });
    connect(m_ui.radioButton_balanceLight, &QRadioButton::toggled, this, [this](bool checked)
    {
        if (checked)
            applyPreset(BalancePreset::Light);
    });
    connect(m_ui.radioButton_balanceMedium, &QRadioButton::toggled, this, [this](bool checked)
    {
        if (checked)
            applyPreset(BalancePreset::Medium);
    });
    connect(m_ui.radioButton_balanceStrong, &QRadioButton::toggled, this, [this](bool checked)
    {
        if (checked)
            applyPreset(BalancePreset::Strong);
    });

    connect(m_ui.spinBox_kMin, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value)
    {
        m_kMin = value;
    });
    connect(m_ui.spinBox_kMax, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value)
    {
        m_kMax = value;
    });
    connect(m_ui.doubleSpinBox_trim, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double value)
    {
        m_trimPercent = value;
    });
    connect(m_ui.checkBox_balanceIntensityRGB, &QCheckBox::toggled, this, [this](bool checked)
    {
        m_applyOnIntensityAndRgb = checked;
    });

    m_openPath = QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::colorBalanceFilterDialogDisplay);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::projectPath);

    applyPreset(m_preset);
    syncUiFromValues();
    adjustSize();
}

DialogColorBalanceFilter::~DialogColorBalanceFilter()
{
    m_dataDispatcher.unregisterObserver(this);
}

void DialogColorBalanceFilter::informData(IGuiData* data)
{
    switch (data->getType())
    {
    case guiDType::colorBalanceFilterDialogDisplay:
    {
        auto dialogData = static_cast<GuiDataColorBalanceFilterDialogDisplay*>(data);
        updateAvailability(dialogData->m_rgbAvailable, dialogData->m_intensityAvailable, dialogData->m_rgbAndIntensityAvailable);
        refreshUI();
    }
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

void DialogColorBalanceFilter::startBalancing()
{
    m_outputFolder = m_ui.lineEdit_folder->text().toStdWString();
    if (m_outputFolder.empty())
    {
        m_dataDispatcher.updateInformation(new GuiDataWarning(TEXT_NO_DIRECTORY_SELECTED));
        return;
    }

    int kMin = m_ui.spinBox_kMin->value();
    int kMax = std::max(kMin, m_ui.spinBox_kMax->value());
    double trimPercent = m_ui.doubleSpinBox_trim->value();
    bool openFolder = m_ui.checkBox_openFolderAfterExport->isChecked();
    bool applyOnIntensityAndRgb = m_ui.checkBox_balanceIntensityRGB->isChecked();

    m_dataDispatcher.sendControl(new control::function::ForwardMessage(new ColorBalanceFilterMessage(kMin, kMax, trimPercent, m_mode, applyOnIntensityAndRgb, m_outputFolder, openFolder)));

    hide();
}

void DialogColorBalanceFilter::cancelBalancing()
{
    m_dataDispatcher.sendControl(new control::function::Abort());
    hide();
}

void DialogColorBalanceFilter::translateUI()
{
    setWindowTitle(TEXT_EXPORT_TITLE_COLOR_BALANCE);
}

void DialogColorBalanceFilter::refreshUI()
{
    syncUiFromValues();
    show();
}

void DialogColorBalanceFilter::applyPreset(BalancePreset preset)
{
    m_preset = preset;
    BalancePresetValues values = presetForMode(m_mode, preset);

    m_kMin = values.kMin;
    m_kMax = values.kMax;
    m_trimPercent = values.trimPercent;

    syncUiFromValues();
}

void DialogColorBalanceFilter::syncUiFromValues()
{
    const QSignalBlocker blockLight(m_ui.radioButton_balanceLight);
    const QSignalBlocker blockMedium(m_ui.radioButton_balanceMedium);
    const QSignalBlocker blockStrong(m_ui.radioButton_balanceStrong);
    const QSignalBlocker blockKMin(m_ui.spinBox_kMin);
    const QSignalBlocker blockKMax(m_ui.spinBox_kMax);
    const QSignalBlocker blockTrim(m_ui.doubleSpinBox_trim);
    const QSignalBlocker blockModeSeparate(m_ui.radioButton_separate);
    const QSignalBlocker blockModeGlobal(m_ui.radioButton_global);
    const QSignalBlocker blockIntensityRgb(m_ui.checkBox_balanceIntensityRGB);

    m_ui.radioButton_balanceLight->setChecked(m_preset == BalancePreset::Light);
    m_ui.radioButton_balanceMedium->setChecked(m_preset == BalancePreset::Medium);
    m_ui.radioButton_balanceStrong->setChecked(m_preset == BalancePreset::Strong);

    m_ui.radioButton_separate->setChecked(m_mode == ColorBalanceMode::Separate);
    m_ui.radioButton_global->setChecked(m_mode == ColorBalanceMode::Global);

    m_ui.spinBox_kMin->setValue(m_kMin);
    m_ui.spinBox_kMax->setValue(m_kMax);
    m_ui.doubleSpinBox_trim->setValue(m_trimPercent);
    m_ui.checkBox_balanceIntensityRGB->setChecked(m_applyOnIntensityAndRgb);

    m_ui.checkBox_balanceIntensityRGB->setEnabled(m_rgbAndIntensityAvailable);
}

void DialogColorBalanceFilter::updateAvailability(bool rgbAvailable, bool intensityAvailable, bool rgbAndIntensityAvailable)
{
    m_rgbAvailable = rgbAvailable;
    m_intensityAvailable = intensityAvailable;
    m_rgbAndIntensityAvailable = rgbAndIntensityAvailable;

    if (!m_rgbAndIntensityAvailable)
    {
        m_applyOnIntensityAndRgb = false;
    }
}
