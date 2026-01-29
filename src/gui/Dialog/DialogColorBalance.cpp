#include "gui/Dialog/DialogColorBalance.h"

#include "controller/controls/ControlFunction.h"
#include "controller/messages/ColorBalanceMessage.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ExportTexts.hpp"
#include "gui/texts/FileTypeTexts.hpp"
#include "gui/texts/FileSystemTexts.hpp"

#include <QtCore/QSignalBlocker>
#include <QtCore/qstandardpaths.h>
#include <QtWidgets/qfiledialog.h>

#include <vector>

namespace
{
    struct ColorBalancePresetValues
    {
        int kMin;
        int kMax;
        double trimPercent;
    };

    constexpr ColorBalancePresetValues kPresetLight{8, 16, 12.0};
    constexpr ColorBalancePresetValues kPresetMedium{16, 32, 20.0};
    constexpr ColorBalancePresetValues kPresetStrong{24, 40, 30.0};

    const static std::vector<FileType> kColorBalanceExportFileTypes = {
        FileType::TLS,
        FileType::E57,
        FileType::PTS
    };

    template<typename Enum_T>
    void initComboBoxRestricted(QComboBox* comboBox, const std::vector<Enum_T>& valuesDisplayable, const std::unordered_map<Enum_T, QString>& labelDictionnary)
    {
        for (Enum_T value : valuesDisplayable)
        {
            if (labelDictionnary.find(value) != labelDictionnary.cend())
                comboBox->addItem(labelDictionnary.at(value), QVariant(static_cast<int>(value)));
            else
                comboBox->addItem(TEXT_EXPORT_LABEL_MISSING, QVariant(static_cast<int>(value)));
        }
    }
}

DialogColorBalance::DialogColorBalance(IDataDispatcher& dataDispatcher, QWidget* parent)
    : ADialog(dataDispatcher, parent)
{
    m_ui.setupUi(this);
    translateUI();
    initComboBoxRestricted<FileType>(m_ui.comboBox_file_format, kColorBalanceExportFileTypes, s_OutputFormatTexts);

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

    connect(m_ui.radioButton_strengthLight, &QRadioButton::toggled, this, [this](bool checked)
    {
        if (checked)
            applyPreset(StrengthPreset::Light);
    });
    connect(m_ui.radioButton_strengthMedium, &QRadioButton::toggled, this, [this](bool checked)
    {
        if (checked)
            applyPreset(StrengthPreset::Medium);
    });
    connect(m_ui.radioButton_strengthStrong, &QRadioButton::toggled, this, [this](bool checked)
    {
        if (checked)
            applyPreset(StrengthPreset::Strong);
    });

    connect(m_ui.spinBox_kMin, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value)
    {
        m_settings.kMin = value;
        if (m_ui.spinBox_kMax->value() < value)
            m_ui.spinBox_kMax->setValue(value);
    });
    connect(m_ui.spinBox_kMax, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value)
    {
        m_settings.kMax = value;
        if (m_ui.spinBox_kMin->value() > value)
            m_ui.spinBox_kMin->setValue(value);
    });
    connect(m_ui.doubleSpinBox_trim, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double value)
    {
        m_settings.trimPercent = value;
    });

    connect(m_ui.checkBox_applyRgbIntensity, &QCheckBox::toggled, this, [this](bool checked)
    {
        m_settings.applyOnIntensity = checked;
    });

    connect(m_ui.checkBox_preserveDetails, &QCheckBox::toggled, this, [this](bool checked)
    {
        m_settings.preserveDetails = checked;
    });

    m_openPath = QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::colorBalanceDialogDisplay);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::projectPath);
    applyPreset(m_preset);
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
        auto displayData = static_cast<GuiDataColorBalanceDialogDisplay*>(data);
        m_enableApplyRgbIntensity = displayData->m_enableRgbIntensity;
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

void DialogColorBalance::startColorBalance()
{
    m_outputFolder = m_ui.lineEdit_folder->text().toStdWString();
    if (m_outputFolder.empty())
    {
        m_dataDispatcher.updateInformation(new GuiDataWarning(TEXT_NO_DIRECTORY_SELECTED));
        return;
    }

    m_settings.kMin = m_ui.spinBox_kMin->value();
    m_settings.kMax = m_ui.spinBox_kMax->value();
    m_settings.trimPercent = m_ui.doubleSpinBox_trim->value();
    m_settings.applyOnIntensity = m_ui.checkBox_applyRgbIntensity->isChecked();
    m_settings.preserveDetails = m_ui.checkBox_preserveDetails->isChecked();
    m_outputFileType = static_cast<FileType>(m_ui.comboBox_file_format->currentData().toInt());

    bool openFolder = m_ui.checkBox_openFolderAfterExport->isChecked();
    m_dataDispatcher.sendControl(new control::function::ForwardMessage(new ColorBalanceMessage(m_settings, m_outputFileType, m_outputFolder, openFolder)));

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

void DialogColorBalance::applyPreset(StrengthPreset preset)
{
    m_preset = preset;

    ColorBalancePresetValues values = kPresetMedium;
    switch (preset)
    {
    case StrengthPreset::Light:
        values = kPresetLight;
        break;
    case StrengthPreset::Medium:
        values = kPresetMedium;
        break;
    case StrengthPreset::Strong:
        values = kPresetStrong;
        break;
    default:
        break;
    }

    m_settings.kMin = values.kMin;
    m_settings.kMax = values.kMax;
    m_settings.trimPercent = values.trimPercent;

    syncUiFromValues();
}

void DialogColorBalance::syncUiFromValues()
{
    const QSignalBlocker blockLight(m_ui.radioButton_strengthLight);
    const QSignalBlocker blockMedium(m_ui.radioButton_strengthMedium);
    const QSignalBlocker blockStrong(m_ui.radioButton_strengthStrong);
    const QSignalBlocker blockKMin(m_ui.spinBox_kMin);
    const QSignalBlocker blockKMax(m_ui.spinBox_kMax);
    const QSignalBlocker blockTrim(m_ui.doubleSpinBox_trim);
    const QSignalBlocker blockFormat(m_ui.comboBox_file_format);
    const QSignalBlocker blockApply(m_ui.checkBox_applyRgbIntensity);
    const QSignalBlocker blockDetails(m_ui.checkBox_preserveDetails);

    m_ui.radioButton_strengthLight->setChecked(m_preset == StrengthPreset::Light);
    m_ui.radioButton_strengthMedium->setChecked(m_preset == StrengthPreset::Medium);
    m_ui.radioButton_strengthStrong->setChecked(m_preset == StrengthPreset::Strong);

    m_ui.spinBox_kMin->setValue(m_settings.kMin);
    m_ui.spinBox_kMax->setValue(m_settings.kMax);
    m_ui.doubleSpinBox_trim->setValue(m_settings.trimPercent);

    int formatIndex = m_ui.comboBox_file_format->findData(QVariant(static_cast<int>(m_outputFileType)));
    if (formatIndex >= 0)
        m_ui.comboBox_file_format->setCurrentIndex(formatIndex);

    m_ui.checkBox_applyRgbIntensity->setEnabled(m_enableApplyRgbIntensity);
    if (!m_enableApplyRgbIntensity)
    {
        m_settings.applyOnIntensity = false;
        m_ui.checkBox_applyRgbIntensity->setChecked(false);
    }
    m_ui.checkBox_preserveDetails->setChecked(m_settings.preserveDetails);
}
