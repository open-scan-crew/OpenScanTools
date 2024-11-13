#include "DialogExportPointCloud.h"

#include "gui/texts/ExportTexts.hpp"
#include "gui/texts/FileTypeTexts.hpp"
#include "gui/texts/TlsTexts.hpp"
#include "gui/Texts.hpp"
#include "io/FileUtils.h"
#include "controller/controls/ControlApplication.h"
#include "controller/controls/ControlFunction.h"
#include "controller/messages/ClippingExportParametersMessage.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataIO.h"

#include "models/graph/AClippingNode.h"

#include <QtWidgets/QFileDialog>
#include <QtCore/QStandardPaths>


const static std::vector<FileType> ExportFileTypePermited = {
    FileType::E57,
    FileType::TLS,
    FileType::RCP,
    FileType::PTS
};

template<typename Enum_T>
void initComboBoxRestricted(QComboBox* comboBox, const std::vector<Enum_T> valuesDisplayable, const std::unordered_map<Enum_T, QString>& labelDictionnary)
{
    for (Enum_T value : valuesDisplayable)
    {
        if (labelDictionnary.find(value) != labelDictionnary.cend())
            comboBox->addItem(labelDictionnary.at(value));
        else
            comboBox->addItem(TEXT_EXPORT_LABEL_MISSING);
    }
}

DialogExportPointCloud::DialogExportPointCloud(IDataDispatcher& dataDispatcher, QWidget *parent)
    : ADialog(dataDispatcher, parent)
    , m_parameters{ ObjectStatusFilter::ALL,
                    true, false,
                    ExportClippingFilter::SELECTED,
                    ExportClippingMethod::SCAN_SEPARATED,
                    FileType::TLS, tls::PrecisionType::TL_OCTREE_100UM, "", "", "" }
    , m_showClippingOptions(false)
    , m_showGridOptions(false)
    , m_showMergeOption(true)
{
    m_ui.setupUi(this);
    translateUI();
    m_ui.lineEdit_fileName->setVisible(false);
    m_ui.toolButton_outFile->setVisible(false);
    m_ui.lineEdit_tempFolder->setVisible(false);
    m_ui.toolButton_tempFolder->setVisible(false);
    m_ui.label_temp->setVisible(false);

    initComboBoxPointCloud();
    initComboBoxRestricted<FileType>(m_ui.comboBox_format, ExportFileTypePermited, s_OutputFormatTexts);

    //connect(m_ui.comboBox_pointClouds, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DialogExportPointCloud::onSelectPointCloudSource);
    connect(m_ui.comboBox_clippings, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DialogExportPointCloud::onSelectSource);
    connect(m_ui.comboBox_method, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DialogExportPointCloud::onSelectMethod);
    connect(m_ui.comboBox_format, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DialogExportPointCloud::onSelectFormat);

    m_ui.comboBox_clippings->setCurrentIndex((int)m_parameters.clippingFilter);
    m_ui.comboBox_method->setCurrentIndex((int)m_parameters.method);
    m_ui.comboBox_format->setCurrentIndex((int)m_parameters.outFileType);

    m_openPath = QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory);

    connect(m_ui.toolButton_folder, &QToolButton::released, this, &DialogExportPointCloud::onSelectOutFolder);
    connect(m_ui.toolButton_tempFolder, &QToolButton::released, this, &DialogExportPointCloud::onSelectTempFolder);
    connect(m_ui.toolButton_outFile, &QToolButton::released, this, &DialogExportPointCloud::onSelectOutFile);

    connect(m_ui.pushButton_export, &QPushButton::released, this, &DialogExportPointCloud::startExport);
    connect(m_ui.pushButton_cancel, &QPushButton::released, this, &DialogExportPointCloud::cancelExport);

    m_dataDispatcher.registerObserverOnKey(this, guiDType::exportParametersDisplay);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::projectPath);
    adjustSize();
}

DialogExportPointCloud::~DialogExportPointCloud()
{
    m_dataDispatcher.unregisterObserver(this);
}

void DialogExportPointCloud::informData(IGuiData *data)
{
    switch (data->getType())
    {
    case guiDType::exportParametersDisplay:
    {
        auto displayParam = static_cast<GuiDataExportParametersDisplay*>(data);
        m_showClippingOptions = displayParam->m_useClippings;
        m_showGridOptions = displayParam->m_useGrids;
        m_showMergeOption = displayParam->m_showMergeOption;
        m_clippings = displayParam->m_clippings;
        m_parameters = displayParam->preset_export_params_;
        m_ui.label_msg->clear();
        // Show the names of the clippings in the dialog
        refreshUI();
        break;
    }
    case guiDType::projectPath:
    {
        auto dataType = static_cast<GuiDataProjectPath*>(data);
        m_openPath = QString::fromStdWString(dataType->m_path.wstring());
    }
    break;
    }
}

void DialogExportPointCloud::onSelectSource(int index)
{
    m_parameters.clippingFilter = (ExportClippingFilter)m_ui.comboBox_clippings->currentData().toInt();
    refreshClippingNames();
}

void DialogExportPointCloud::onSelectMethod(int index)
{
    m_parameters.method = (ExportClippingMethod)m_ui.comboBox_method->currentData().toInt();
    refreshClippingNames();
    refreshShowFileName();
}

void DialogExportPointCloud::onSelectFormat(int index)
{
    m_parameters.outFileType = ExportFileTypePermited[index];
    refreshShowFileName();
    refreshUI();
}

void DialogExportPointCloud::onSelectOutFolder()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, TEXT_SELECT_DIRECTORY, m_openPath, QFileDialog::ShowDirsOnly);

    if (folderPath != "")
    {
        m_openPath = folderPath;
        m_ui.lineEdit_folder->setText(folderPath);
    }
}

void DialogExportPointCloud::onSelectTempFolder()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, TEXT_SELECT_DIRECTORY, m_openPath
        , QFileDialog::ShowDirsOnly);

    if (folderPath != "")
    {
        m_openPath = folderPath;
        m_ui.lineEdit_tempFolder->setText(folderPath); 
    }
}

void DialogExportPointCloud::onSelectOutFile()
{
    QString openFormat;
    switch (m_parameters.outFileType) {
    case FileType::E57:
        openFormat = QString("(*.e57)");
        break;
    case FileType::TLS:
        openFormat = QString("(*.tls)");
        break;
    case FileType::RCP:
        openFormat = QString("(*.rcp *.rcs)");
        break;
    }
    QString filePath = QFileDialog::getSaveFileName(this, TEXT_SAVE_FILENAME, m_openPath, openFormat, nullptr);

    QString folderPath = QString::fromStdWString(std::filesystem::path(filePath.toStdWString()).parent_path().wstring());
    QString fileName = QString::fromStdWString(std::filesystem::path(filePath.toStdWString()).stem().wstring());
    if (folderPath != "")
    {
        m_ui.lineEdit_folder->setText(folderPath);
        m_openPath = folderPath;
    }

    if (fileName != "")
        m_ui.lineEdit_fileName->setText(fileName);
}

void DialogExportPointCloud::startExport()
{
    m_parameters.pointCloudFilter = (ObjectStatusFilter)(m_ui.comboBox_pointClouds->currentData().toInt());

    m_parameters.outFileType = ExportFileTypePermited[m_ui.comboBox_format->currentIndex()];
    // Get the export precision
    m_parameters.encodingPrecision = (tls::PrecisionType)(m_ui.comboBox_precision->currentData().toInt());

    m_parameters.fileName = m_ui.lineEdit_fileName->text().toStdWString();
    if (m_parameters.fileName == "" && m_ui.lineEdit_fileName->isVisible())
    {
        m_ui.label_msg->setText(TEXT_MISSING_FILE_NAME);
        m_ui.label_msg->setVisible(true);
        return;
    }

    m_parameters.outFolder = m_ui.lineEdit_folder->text().toStdWString();
    if (m_parameters.outFolder == "")
    {
        m_ui.label_msg->setText(TEXT_NO_DIRECTORY_SELECTED);
        m_ui.label_msg->setVisible(true);
        return;
    }
    m_parameters.tempFolder = m_ui.lineEdit_tempFolder->text().toStdWString();
    bool validDensity;
    double density = m_ui.lineEdit_density->text().toDouble(&validDensity);
    m_parameters.pointDensity = validDensity ? density : -1.0;
    m_parameters.addOriginCube = m_ui.checkBox_addOriginCube->isChecked();
    if (m_ui.groupBox_maxScan->isChecked())
    {
        bool valid;
        int value = m_ui.lineEdit_maxScan->text().toInt(&valid);
        m_parameters.maxScanPerProject = valid ? value : 0;
    }
    else
        m_parameters.maxScanPerProject = 0;

    m_parameters.openFolderAfterExport = m_ui.checkBox_openFolderAfterExport->isChecked();
    m_parameters.exportWithScanImportTranslation = m_ui.checkBox_restoreOriginalCoordinates->isChecked();

    m_dataDispatcher.sendControl(new control::function::ForwardMessage(new ClippingExportParametersMessage(m_parameters)));

    hide();
}

void DialogExportPointCloud::closeEvent(QCloseEvent* event)
{
    cancelExport();
}

void DialogExportPointCloud::cancelExport()
{
    m_dataDispatcher.sendControl(new control::function::Abort());
    hide();
}

void DialogExportPointCloud::translateUI()
{
    m_exportSourceTexts = {
        { ExportClippingFilter::SELECTED, TEXT_EXPORT_CLIPPING_SOURCE_SELECTED },
        { ExportClippingFilter::ACTIVE, TEXT_EXPORT_CLIPPING_SOURCE_ACTIVE }
    };

    m_exportMethodTexts = {
        { ExportClippingMethod::SCAN_SEPARATED, TEXT_EXPORT_METHOD_SCAN_SEPARATED },
        { ExportClippingMethod::CLIPPING_SEPARATED, TEXT_EXPORT_METHOD_CLIPPING_SEPARATED },
        { ExportClippingMethod::CLIPPING_AND_SCAN_MERGED, TEXT_EXPORT_METHOD_SCAN_MERGED }
    };

    // Init the precision combo box (only for the TLS format)
    m_ui.comboBox_precision->clear();
    m_ui.comboBox_precision->addItem(TEXT_TLS_PRECISION_1_MILLIMETER, QVariant((int)tls::PrecisionType::TL_OCTREE_1MM));
    m_ui.comboBox_precision->addItem(TEXT_TLS_PRECISION_100_MICROMETER, QVariant((int)tls::PrecisionType::TL_OCTREE_100UM));
    m_ui.comboBox_precision->addItem(TEXT_TLS_PRECISION_10_MICROMETER, QVariant((int)tls::PrecisionType::TL_OCTREE_10UM));
    m_ui.comboBox_precision->setCurrentIndex(1);
}

void DialogExportPointCloud::refreshUI()
{
    // Set the Dialog title
    if (m_showClippingOptions)
        setWindowTitle(TEXT_EXPORT_TITLE_CLIPPING);
    else if (m_showGridOptions)
        setWindowTitle(TEXT_EXPORT_TITLE_GRID);
    else
        setWindowTitle(TEXT_EXPORT_TITLE_NORMAL);

    // grid
    m_ui.groupBox_maxScan->setVisible(m_showGridOptions && (m_parameters.outFileType == FileType::RCP));
    m_ui.checkBox_addOriginCube->setVisible(m_parameters.outFileType == FileType::RCP);

    int index = m_ui.comboBox_pointClouds->findData(QVariant((int)m_parameters.pointCloudFilter));
    m_ui.comboBox_pointClouds->setCurrentIndex(index);
    // Show source options (all, some or none)
    refreshSourceOption();
    refreshMethodOption();
    refreshClippingNames();

    m_ui.label_precision->setVisible(m_parameters.outFileType == FileType::TLS);
    m_ui.comboBox_precision->setVisible(m_parameters.outFileType == FileType::TLS);
    m_ui.label_density->setVisible(m_parameters.outFileType == FileType::RCP);
    m_ui.label_density_unit->setVisible(m_parameters.outFileType == FileType::RCP);
    m_ui.lineEdit_density->setVisible(m_parameters.outFileType == FileType::RCP);

    m_ui.label_msg->setVisible(m_ui.label_msg->text() != "");
    adjustSize();
    show();
}

void DialogExportPointCloud::initComboBoxPointCloud()
{
    m_ui.comboBox_pointClouds->clear();

    m_ui.comboBox_pointClouds->addItem(TEXT_EXPORT_FILTER_ALL, QVariant((int)ObjectStatusFilter::ALL));
    m_ui.comboBox_pointClouds->addItem(TEXT_EXPORT_FILTER_SELECTED, QVariant((int)ObjectStatusFilter::SELECTED));
    m_ui.comboBox_pointClouds->addItem(TEXT_EXPORT_FILTER_DISPLAYED, QVariant((int)ObjectStatusFilter::VISIBLE));
}

void DialogExportPointCloud::refreshClippingNames()
{
    m_ui.listWidget_clippingNames->clear();

    for (const SafePtr<AClippingNode>& clip : m_clippings)
    {
        ReadPtr<AClippingNode> rClip = clip.cget();
        if (!rClip)
            continue;

        if ((rClip->isSelected() && m_parameters.clippingFilter == ExportClippingFilter::SELECTED) ||
            (rClip->isClippingActive() && m_parameters.clippingFilter == ExportClippingFilter::ACTIVE))
        {
            m_ui.listWidget_clippingNames->addItem(QString::fromStdWString(rClip->getComposedName()));
        }
    }
}

void DialogExportPointCloud::refreshSourceOption()
{
    std::set<ExportClippingFilter> authorizedValues;
    if (m_showClippingOptions)
    {
        authorizedValues.insert(ExportClippingFilter::SELECTED);
        authorizedValues.insert(ExportClippingFilter::ACTIVE);
    }
    if (m_showGridOptions)
        authorizedValues.insert(ExportClippingFilter::GRIDS);
    if (!m_showClippingOptions && !m_showGridOptions)
        authorizedValues.insert(ExportClippingFilter::NONE);

    // Work with a local value because m_parameters.clippingFilter will be modified automaticaly (signal) by m_ui.comboBox_clippings
    ExportClippingFilter storedSource = m_parameters.clippingFilter;
    if (authorizedValues.find(storedSource) == authorizedValues.end())
        storedSource = *authorizedValues.begin();

    m_ui.comboBox_clippings->clear();
    for (auto value : authorizedValues)
    {
        if (m_exportSourceTexts.find(value) != m_exportSourceTexts.end())
            m_ui.comboBox_clippings->addItem(m_exportSourceTexts.at(value), QVariant((int)value));
        else
            m_ui.comboBox_clippings->addItem(TEXT_EXPORT_LABEL_MISSING, QVariant((int)value));

        if (value == storedSource)
            m_ui.comboBox_clippings->setCurrentIndex(m_ui.comboBox_clippings->count() - 1);
    }

    m_ui.label_clippings->setVisible(m_showClippingOptions);
    m_ui.comboBox_clippings->setVisible(m_showClippingOptions);
    m_ui.listWidget_clippingNames->setVisible(m_showClippingOptions);
}

void DialogExportPointCloud::refreshMethodOption()
{
    std::set<ExportClippingMethod> authorizedValues;
    authorizedValues.insert(ExportClippingMethod::SCAN_SEPARATED);
    if (m_showMergeOption)
        authorizedValues.insert(ExportClippingMethod::CLIPPING_AND_SCAN_MERGED);
    if (m_showClippingOptions)
        authorizedValues.insert(ExportClippingMethod::CLIPPING_SEPARATED);

    ExportClippingMethod storedMethod = m_parameters.method;
    if (authorizedValues.find(storedMethod) == authorizedValues.end())
        storedMethod = *authorizedValues.begin();

    // Init the type combo box
    m_ui.comboBox_method->clear();
    for (auto value : authorizedValues)
    {
        if (m_exportMethodTexts.find(value) != m_exportMethodTexts.end())
            m_ui.comboBox_method->addItem(m_exportMethodTexts.at(value), QVariant((int)value));
        else
            m_ui.comboBox_method->addItem(TEXT_EXPORT_LABEL_MISSING, QVariant((int)value));

        if (value == storedMethod)
            m_ui.comboBox_method->setCurrentIndex(m_ui.comboBox_method->count() - 1);
    }

    m_ui.label_method->setVisible(!m_showGridOptions);
    m_ui.comboBox_method->setVisible(!m_showGridOptions);
}

void DialogExportPointCloud::refreshShowFileName()
{
    bool askFileName = (!m_showGridOptions) && (m_parameters.method == ExportClippingMethod::CLIPPING_AND_SCAN_MERGED);
    askFileName |= m_parameters.outFileType == FileType::RCP;

    m_ui.label_fileName->setVisible(askFileName);
    m_ui.lineEdit_fileName->setVisible(askFileName);
    m_ui.toolButton_outFile->setVisible(askFileName);
}