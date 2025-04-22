#include "gui/Dialog/DialogExportPrimitives.h"
#include "controller/controls/ControlIO.h"
#include "io/FileUtils.h"
#include "gui/texts/FileSystemTexts.hpp"
#include "gui/texts/ExportTexts.hpp"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataIO.h"

#include <QtWidgets/qfiledialog.h>
#include <QtCore/qstandardpaths.h>


DialogExportPrimitives::DialogExportPrimitives(IDataDispatcher& dataDispatcher, QWidget *parent, float guiScale)
    : ADialog(dataDispatcher, parent)
	, m_parameters({false})
	, m_filename(L"")
	, m_exportFolder("")
	, m_format(ObjectExportType::CSV)
{
    m_ui.setupUi(this);

	//this->setAttribute(Qt::WA_DeleteOnClose);

	std::list<std::pair<QVariant, QString>> comboFilter({ {QVariant((uint32_t)ObjectStatusFilter::ALL),TEXT_EXPORT_FILTER_ALL},
	{QVariant((uint32_t)ObjectStatusFilter::SELECTED),TEXT_EXPORT_FILTER_SELECTED},
	{QVariant((uint32_t)ObjectStatusFilter::VISIBLE),TEXT_EXPORT_FILTER_DISPLAYED} });


	m_ui.radioButton_mergeOneFile->setVisible(false);
	m_ui.radioButton_oneFilePerObjectType->setVisible(false);

	m_openPath = QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory);

	for (auto iterator : comboFilter)
		m_ui.comboBox_filter->addItem(iterator.second, iterator.first);
	m_ui.comboBox_filter->setCurrentIndex(0);

    connect(m_ui.toolButton_folder, &QToolButton::released, this, &DialogExportPrimitives::onSelectOutFolder);
	connect(m_ui.toolButton_file, &QToolButton::released, this, &DialogExportPrimitives::onSelectOutFile);

    connect(m_ui.pushButton_export, &QPushButton::released, this, &DialogExportPrimitives::startExport);
    connect(m_ui.pushButton_cancel, &QPushButton::released, this, &DialogExportPrimitives::cancelExport);

    m_dataDispatcher.registerObserverOnKey(this, guiDType::projectPath);
	this->setMinimumWidth(344 * guiScale);
	adjustSize();
}

DialogExportPrimitives::~DialogExportPrimitives()
{
    m_dataDispatcher.unregisterObserver(this);
}

void DialogExportPrimitives::setFormat(ObjectExportType format)
{
	m_format = format;
	std::list<std::pair<QVariant, QString>> comboType;

	m_ui.scanBox->setVisible(true);
	m_ui.tagsBox->setVisible(true);
	m_ui.pointsBox->setVisible(true);
	m_ui.viewpointBox->setVisible(true);
	m_ui.spheresBox->setVisible(true);

	m_ui.pcoBox->setVisible(true);
	m_ui.boxesBox->setVisible(true);
	m_ui.externBox->setVisible(true);
	m_ui.measuresBox->setVisible(true);
	m_ui.pipingBox->setVisible(true);

	m_ui.restoreTruncatedCoordinatesCheckBox->setVisible(true);

	switch (m_format) {
		case ObjectExportType::STEP:
		case ObjectExportType::CSV:
		case ObjectExportType::DXF:
		{
			m_ui.toolButton_file->setVisible(true);
			m_ui.label_fileName->setVisible(true);
			m_ui.lineEdit_fileName->setVisible(true);

			m_ui.viewpointBox->setVisible(false);
			m_ui.externBox->setVisible(false);
			m_ui.pcoBox->setVisible(false);

			m_ui.viewpointBox->setChecked(false);
			m_ui.externBox->setChecked(false);
			m_ui.pcoBox->setChecked(false);
		}
		break;
		case ObjectExportType::OST:
		{
			m_ui.lineEdit_fileName->setVisible(false);
			m_ui.toolButton_file->setVisible(false);
			m_ui.label_fileName->setVisible(false);

			m_ui.restoreTruncatedCoordinatesCheckBox->setVisible(false);
		}
		break;
		case ObjectExportType::OBJ:
		case ObjectExportType::FBX:
		{
			m_ui.toolButton_file->setVisible(true);
			m_ui.label_fileName->setVisible(true);
			m_ui.lineEdit_fileName->setVisible(true);

			m_ui.scanBox->setVisible(false);
			m_ui.tagsBox->setVisible(false);
			m_ui.pointsBox->setVisible(false);
			m_ui.viewpointBox->setVisible(false);
			m_ui.pcoBox->setVisible(false);

			m_ui.scanBox->setChecked(false);
			m_ui.tagsBox->setChecked(false);
			m_ui.pointsBox->setChecked(false);
			m_ui.viewpointBox->setChecked(false);
			m_ui.pcoBox->setChecked(false);
		}
		break;
	}

	refreshUI();
}

void DialogExportPrimitives::informData(IGuiData *data)
{
    switch (data->getType())
    {
		case guiDType::projectPath:
		{
			auto dataType = static_cast<GuiDataProjectPath*>(data);
			m_openPath = QString::fromStdWString(dataType->m_path.wstring());
		}
		break;
    }
}

void DialogExportPrimitives::onSelectOutFolder()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, TEXT_SELECT_DIRECTORY, m_openPath, QFileDialog::ShowDirsOnly);

    if (folderPath != "")
    {
        m_ui.lineEdit_folder->setText(folderPath);
		m_openPath = folderPath;
    }
}

void DialogExportPrimitives::onSelectOutFile()
{
	QString openFormat;
	switch (m_format) {
	case ObjectExportType::CSV:
		openFormat = QString("(*.csv)");
		break;
	case ObjectExportType::DXF:
		openFormat = QString("(*.dxf)");
		break;
	case ObjectExportType::STEP:
		openFormat = QString("(*.step)");
		break;
	case ObjectExportType::OBJ:
		openFormat = QString("(*.obj)");
		break;
	case ObjectExportType::FBX:
		openFormat = QString("(*.fbx)");
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

void DialogExportPrimitives::startExport()
{

    m_filename = m_ui.lineEdit_fileName->text().toStdWString();
    if (m_filename == L"" && m_ui.lineEdit_fileName->isVisible())
    {
		m_dataDispatcher.updateInformation(new GuiDataWarning(TEXT_MISSING_FILE_NAME));
        return;
    }

	m_exportFolder = m_ui.lineEdit_folder->text().toStdWString();
    if (m_exportFolder == "")
    {
		m_dataDispatcher.updateInformation(new GuiDataWarning(TEXT_NO_DIRECTORY_SELECTED));
        return;
    }

	m_parameters.oneFilePerType = false;// m_ui.radioButton_oneFilePerObjectType->isChecked();
	m_parameters.openFolderWindowsAfterExport = m_ui.openExplorerFolderCheckBox->isChecked();
	m_parameters.exportWithScanImportTranslation = m_ui.restoreTruncatedCoordinatesCheckBox->isChecked();

	std::unordered_set<ElementType> selectedTypes = getSelectedTypes();

	switch (m_format) {
		case ObjectExportType::DXF:
		{
			m_dataDispatcher.sendControl(new control::io::ItemsToDxf(m_exportFolder / m_filename, selectedTypes, ObjectStatusFilter(m_ui.comboBox_filter->currentData().toInt()), m_parameters));
		}
		break;
		case ObjectExportType::CSV:
		{
			m_dataDispatcher.sendControl(new control::io::ItemsToCSV(m_exportFolder / m_filename, selectedTypes, ObjectStatusFilter(m_ui.comboBox_filter->currentData().toInt()), m_parameters));
		}
		break;
		case ObjectExportType::STEP:
		{
			m_dataDispatcher.sendControl(new control::io::ItemsToStep(m_exportFolder / m_filename, selectedTypes, ObjectStatusFilter(m_ui.comboBox_filter->currentData().toInt()), m_parameters));
		}
		break;
		case ObjectExportType::OST:
		{
			m_dataDispatcher.sendControl(new control::io::ItemsToOST(m_exportFolder / m_filename, selectedTypes, ObjectStatusFilter(m_ui.comboBox_filter->currentData().toInt()), m_parameters));
		}
		break;
		case ObjectExportType::OBJ:
		{
			m_dataDispatcher.sendControl(new control::io::ItemsToObj(m_exportFolder / m_filename, selectedTypes, ObjectStatusFilter(m_ui.comboBox_filter->currentData().toInt()), m_parameters));
		}
		break;
		case ObjectExportType::FBX:
		{
			m_dataDispatcher.sendControl(new control::io::ItemsToFbx(m_exportFolder / m_filename, selectedTypes, ObjectStatusFilter(m_ui.comboBox_filter->currentData().toInt()), m_parameters));
		}
		break;

	}

    hide();
}

void DialogExportPrimitives::closeEvent(QCloseEvent* event)
{
    cancelExport();
}


void DialogExportPrimitives::cancelExport()
{
    hide();
}

void DialogExportPrimitives::refreshUI()
{
    // Set the Dialog title
	switch (m_format) {
		case ObjectExportType::DXF:
		{
			setWindowTitle(TEXT_EXPORT_GENERAL_FILE.arg(".dxf"));
		}
		break;
		case ObjectExportType::CSV:
		{
			setWindowTitle(TEXT_EXPORT_GENERAL_FILE.arg(".csv"));
		}
		break;
		case ObjectExportType::STEP:
		{
			setWindowTitle(TEXT_EXPORT_GENERAL_FILE.arg(".step"));
		}
		break;
		case ObjectExportType::OST:
		{
			setWindowTitle(TEXT_EXPORT_OPENSCANTOOLS_FILE);
		}
		break;
		case ObjectExportType::FBX:
		{
			setWindowTitle(TEXT_EXPORT_GENERAL_FILE.arg(".fbx"));
		}
		break;
		case ObjectExportType::OBJ:
		{
			setWindowTitle(TEXT_EXPORT_GENERAL_FILE.arg(".obj"));
		}
		break;
		default:
		{
			setWindowTitle("ERROR : NO_TITLE");
		}
		break;
	}

    adjustSize();
    //show();
}

std::unordered_set<ElementType> DialogExportPrimitives::getSelectedTypes()
{
	std::unordered_set<ElementType> selectedTypes;
	std::vector<std::pair<QCheckBox*, std::unordered_set<ElementType>>> typeBoxes =
	{
		{m_ui.scanBox, {ElementType::Scan}},
		{m_ui.pcoBox, {ElementType::PCO}},
		{m_ui.tagsBox, {ElementType::Tag}},
		{m_ui.pointsBox, {ElementType::Point}},
		{m_ui.measuresBox, {ElementType::SimpleMeasure, ElementType::PolylineMeasure
						, ElementType::PointToPlaneMeasure, ElementType::PointToPipeMeasure
						, ElementType::PipeToPlaneMeasure, ElementType::PipeToPipeMeasure
						, ElementType::ColumnTiltMeasure, ElementType::BeamBendingMeasure}},
		{m_ui.spheresBox, {ElementType::Sphere}},
		{m_ui.pipingBox, {ElementType::Piping, ElementType::Cylinder, ElementType::Torus}},
		{m_ui.boxesBox, {ElementType::Box}},
		{m_ui.externBox, {ElementType::MeshObject}},
		{m_ui.viewpointBox, {ElementType::ViewPoint}}
	};

	for (auto box : typeBoxes)
		if (box.first->isEnabled() && box.first->isChecked())
			selectedTypes.insert(box.second.begin(), box.second.end());

	return selectedTypes;
}
