#include "gui/Dialog/DialogColorDenoiseFilter.h"

#include "controller/controls/ControlFunction.h"
#include "controller/messages/ColorDenoiseFilterMessage.h"
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
    struct DenoisePresetValues
    {
        int kNeighbors;
        int strength;
        double radiusFactor;
        int iterations;
        bool preserveLuminance;
    };

    constexpr DenoisePresetValues kPresetLow{24, 30, 3.0, 1, true};
    constexpr DenoisePresetValues kPresetMid{32, 60, 4.0, 1, true};
    constexpr DenoisePresetValues kPresetHigh{48, 85, 5.0, 2, true};
}

DialogColorDenoiseFilter::DialogColorDenoiseFilter(IDataDispatcher& dataDispatcher, QWidget* parent)
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
    connect(m_ui.pushButton_ok, &QPushButton::clicked, this, &DialogColorDenoiseFilter::startFiltering);
    connect(m_ui.pushButton_cancel, &QPushButton::clicked, this, &DialogColorDenoiseFilter::cancelFiltering);

    connect(m_ui.radioButton_denoiseLow, &QRadioButton::toggled, this, [this](bool checked)
    {
        if (checked)
            applyPreset(DenoisePreset::Low);
    });
    connect(m_ui.radioButton_denoiseMid, &QRadioButton::toggled, this, [this](bool checked)
    {
        if (checked)
            applyPreset(DenoisePreset::Mid);
    });
    connect(m_ui.radioButton_denoiseHigh, &QRadioButton::toggled, this, [this](bool checked)
    {
        if (checked)
            applyPreset(DenoisePreset::High);
    });

    connect(m_ui.spinBox_kNeighbors, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value)
    {
        m_kNeighbors = value;
    });
    connect(m_ui.spinBox_strength, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value)
    {
        m_strength = value;
    });
    connect(m_ui.doubleSpinBox_radiusFactor, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double value)
    {
        m_radiusFactor = value;
    });
    connect(m_ui.spinBox_iterations, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value)
    {
        m_iterations = value;
    });
    connect(m_ui.checkBox_preserveLuminance, &QCheckBox::toggled, this, [this](bool checked)
    {
        m_preserveLuminance = checked;
    });

    m_openPath = QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::colorDenoiseFilterDialogDisplay);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::projectPath);
    syncUiFromValues();
    adjustSize();
}

DialogColorDenoiseFilter::~DialogColorDenoiseFilter()
{
    m_dataDispatcher.unregisterObserver(this);
}

void DialogColorDenoiseFilter::informData(IGuiData* data)
{
    switch (data->getType())
    {
    case guiDType::colorDenoiseFilterDialogDisplay:
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

void DialogColorDenoiseFilter::startFiltering()
{
    m_outputFolder = m_ui.lineEdit_folder->text().toStdWString();
    if (m_outputFolder.empty())
    {
        m_dataDispatcher.updateInformation(new GuiDataWarning(TEXT_NO_DIRECTORY_SELECTED));
        return;
    }

    int kNeighbors = m_ui.spinBox_kNeighbors->value();
    int strength = m_ui.spinBox_strength->value();
    double radiusFactor = m_ui.doubleSpinBox_radiusFactor->value();
    int iterations = m_ui.spinBox_iterations->value();
    bool preserveLuminance = m_ui.checkBox_preserveLuminance->isChecked();

    bool openFolder = m_ui.checkBox_openFolderAfterExport->isChecked();
    m_dataDispatcher.sendControl(new control::function::ForwardMessage(new ColorDenoiseFilterMessage(kNeighbors, strength, radiusFactor, iterations, preserveLuminance, m_outputFolder, openFolder)));

    hide();
}

void DialogColorDenoiseFilter::cancelFiltering()
{
    m_dataDispatcher.sendControl(new control::function::Abort());
    hide();
}

void DialogColorDenoiseFilter::translateUI()
{
    setWindowTitle(TEXT_EXPORT_TITLE_COLOR_DENOISE);
}

void DialogColorDenoiseFilter::refreshUI()
{
    syncUiFromValues();
    show();
}

void DialogColorDenoiseFilter::applyPreset(DenoisePreset preset)
{
    m_preset = preset;

    DenoisePresetValues values = kPresetMid;
    switch (preset)
    {
    case DenoisePreset::Low:
        values = kPresetLow;
        break;
    case DenoisePreset::Mid:
        values = kPresetMid;
        break;
    case DenoisePreset::High:
        values = kPresetHigh;
        break;
    default:
        break;
    }

    m_kNeighbors = values.kNeighbors;
    m_strength = values.strength;
    m_radiusFactor = values.radiusFactor;
    m_iterations = values.iterations;
    m_preserveLuminance = values.preserveLuminance;

    syncUiFromValues();
}

void DialogColorDenoiseFilter::syncUiFromValues()
{
    const QSignalBlocker blockLow(m_ui.radioButton_denoiseLow);
    const QSignalBlocker blockMid(m_ui.radioButton_denoiseMid);
    const QSignalBlocker blockHigh(m_ui.radioButton_denoiseHigh);
    const QSignalBlocker blockK(m_ui.spinBox_kNeighbors);
    const QSignalBlocker blockStrength(m_ui.spinBox_strength);
    const QSignalBlocker blockRadius(m_ui.doubleSpinBox_radiusFactor);
    const QSignalBlocker blockIterations(m_ui.spinBox_iterations);
    const QSignalBlocker blockPreserve(m_ui.checkBox_preserveLuminance);

    m_ui.radioButton_denoiseLow->setChecked(m_preset == DenoisePreset::Low);
    m_ui.radioButton_denoiseMid->setChecked(m_preset == DenoisePreset::Mid);
    m_ui.radioButton_denoiseHigh->setChecked(m_preset == DenoisePreset::High);

    m_ui.spinBox_kNeighbors->setValue(m_kNeighbors);
    m_ui.spinBox_strength->setValue(m_strength);
    m_ui.doubleSpinBox_radiusFactor->setValue(m_radiusFactor);
    m_ui.spinBox_iterations->setValue(m_iterations);
    m_ui.checkBox_preserveLuminance->setChecked(m_preserveLuminance);
}
