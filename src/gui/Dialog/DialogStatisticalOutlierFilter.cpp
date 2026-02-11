#include "gui/Dialog/DialogStatisticalOutlierFilter.h"

#include "controller/controls/ControlFunction.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataIO.h"
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
    struct OutlierPresetValues
    {
        int kNeighbors;
        double nSigma;
        int samplingPercent;
        double beta;
    };

    constexpr OutlierPresetValues kPresetLow{30, 1.8, 3, 5.0};
    constexpr OutlierPresetValues kPresetMid{20, 1.0, 2, 4.0};
    constexpr OutlierPresetValues kPresetHigh{10, 0.8, 1, 2.5};

    const static std::vector<FileType> kOutlierExportFileTypes = {
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

DialogStatisticalOutlierFilter::DialogStatisticalOutlierFilter(IDataDispatcher& dataDispatcher, QWidget* parent)
    : ADialog(dataDispatcher, parent)
{
    m_ui.setupUi(this);
    translateUI();
    initComboBoxRestricted<FileType>(m_ui.comboBox_file_format, kOutlierExportFileTypes, s_OutputFormatTexts);

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
    connect(m_ui.radioButton_soLow, &QRadioButton::toggled, this, [this](bool checked)
    {
        if (checked)
            applyPreset(OutlierPreset::Low);
    });
    connect(m_ui.radioButton_soMid, &QRadioButton::toggled, this, [this](bool checked)
    {
        if (checked)
            applyPreset(OutlierPreset::Mid);
    });
    connect(m_ui.radioButton_soHigh, &QRadioButton::toggled, this, [this](bool checked)
    {
        if (checked)
            applyPreset(OutlierPreset::High);
    });

    connect(m_ui.spinBox_kNeighbors, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value)
    {
        m_kNeighbors = value;
    });
    connect(m_ui.doubleSpinBox_nSigma, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double value)
    {
        m_nSigma = value;
    });
    connect(m_ui.spinBox_soSampling, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value)
    {
        m_samplingPercent = value;
    });
    connect(m_ui.doubleSpinBox_soBeta, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double value)
    {
        m_beta = value;
    });

    m_openPath = QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::statisticalOutlierFilterDialogDisplay);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::projectPath);
    syncUiFromValues();
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
    int samplingPercent = m_ui.spinBox_soSampling->value();
    double beta = m_ui.doubleSpinBox_soBeta->value();
    m_outputFileType = static_cast<FileType>(m_ui.comboBox_file_format->currentData().toInt());

    bool openFolder = m_ui.checkBox_openFolderAfterExport->isChecked();
    m_dataDispatcher.sendControl(new control::function::ForwardMessage(new StatisticalOutlierFilterMessage(kNeighbors, nSigma, samplingPercent, beta, m_mode, m_outputFileType, m_outputFolder, openFolder)));

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
    syncUiFromValues();
    show();
}

void DialogStatisticalOutlierFilter::applyPreset(OutlierPreset preset)
{
    m_preset = preset;

    OutlierPresetValues values = kPresetMid;
    switch (preset)
    {
    case OutlierPreset::Low:
        values = kPresetLow;
        break;
    case OutlierPreset::Mid:
        values = kPresetMid;
        break;
    case OutlierPreset::High:
        values = kPresetHigh;
        break;
    default:
        break;
    }

    m_kNeighbors = values.kNeighbors;
    m_nSigma = values.nSigma;
    m_samplingPercent = values.samplingPercent;
    m_beta = values.beta;

    syncUiFromValues();
}

void DialogStatisticalOutlierFilter::syncUiFromValues()
{
    const QSignalBlocker blockLow(m_ui.radioButton_soLow);
    const QSignalBlocker blockMid(m_ui.radioButton_soMid);
    const QSignalBlocker blockHigh(m_ui.radioButton_soHigh);
    const QSignalBlocker blockK(m_ui.spinBox_kNeighbors);
    const QSignalBlocker blockSigma(m_ui.doubleSpinBox_nSigma);
    const QSignalBlocker blockSampling(m_ui.spinBox_soSampling);
    const QSignalBlocker blockBeta(m_ui.doubleSpinBox_soBeta);
    const QSignalBlocker blockFormat(m_ui.comboBox_file_format);

    m_ui.radioButton_soLow->setChecked(m_preset == OutlierPreset::Low);
    m_ui.radioButton_soMid->setChecked(m_preset == OutlierPreset::Mid);
    m_ui.radioButton_soHigh->setChecked(m_preset == OutlierPreset::High);

    m_ui.spinBox_kNeighbors->setValue(m_kNeighbors);
    m_ui.doubleSpinBox_nSigma->setValue(m_nSigma);
    m_ui.spinBox_soSampling->setValue(m_samplingPercent);
    m_ui.doubleSpinBox_soBeta->setValue(m_beta);
    int formatIndex = m_ui.comboBox_file_format->findData(QVariant(static_cast<int>(m_outputFileType)));
    if (formatIndex >= 0)
        m_ui.comboBox_file_format->setCurrentIndex(formatIndex);
}
