#include "gui/Dialog/DialogExportSubProject.h"
#include "gui/Texts.hpp"
#include "io/FileUtils.h"

#include "controller/controls/ControlApplication.h"

#include "controller/controls/ControlIO.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataIO.h"

#include <QtWidgets/QFileDialog>
#include <QtCore/QStandardPaths>


DialogExportSubProject::DialogExportSubProject(IDataDispatcher& dataDispatcher, QWidget* parent, float guiScale)
	: ADialog(dataDispatcher, parent)
	, m_exportFolder("")
	, m_subProjectInfoDialog(new DialogSubProjectInfo(dataDispatcher, parent))
{
    m_ui.setupUi(this);

	//this->setAttribute(Qt::WA_DeleteOnClose);

	std::list<std::pair<QVariant, QString>> comboFilter({
	{QVariant((uint32_t)ObjectStatusFilter::NONE),TEXT_EXPORT_FILTER_NONE},
	{QVariant((uint32_t)ObjectStatusFilter::ALL),TEXT_EXPORT_FILTER_ALL},
	{QVariant((uint32_t)ObjectStatusFilter::SELECTED),TEXT_EXPORT_FILTER_SELECTED},
	{QVariant((uint32_t)ObjectStatusFilter::VISIBLE),TEXT_EXPORT_FILTER_DISPLAYED} });


	m_openPath = QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory);

	for (auto iterator : comboFilter)
		m_ui.objectsFilterComboBox->addItem(iterator.second, iterator.first);
	m_ui.objectsFilterComboBox->setCurrentIndex(0);

    connect(m_ui.toolButton_folder, &QToolButton::released, this, &DialogExportSubProject::onSelectOutFolder);
	connect(m_ui.subProjectInfoButton, &QToolButton::released, this, &DialogExportSubProject::onSubProjectInfo);

    connect(m_ui.pushButton_export, &QPushButton::released, this, &DialogExportSubProject::startExport);
    connect(m_ui.pushButton_cancel, &QPushButton::released, this, &DialogExportSubProject::cancelExport);

    m_dataDispatcher.registerObserverOnKey(this, guiDType::projectPath);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectDataProperties);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectDataPropertiesNoOpen);

	this->setMinimumWidth(344 * guiScale);
	adjustSize();
}

DialogExportSubProject::~DialogExportSubProject()
{
    m_dataDispatcher.unregisterObserver(this);
	m_subProjectInfoDialog->close();
}

void DialogExportSubProject::informData(IGuiData *data)
{
    switch (data->getType())
    {
		case guiDType::projectPath:
		{
			auto dataType = static_cast<GuiDataProjectPath*>(data);
			m_openPath = QString::fromStdWString(dataType->m_path.wstring());
		}
		break;
		case guiDType::projectDataProperties:
		case guiDType::projectDataPropertiesNoOpen:
		{
			auto dataType = static_cast<GuiDataProjectProperties*>(data);
			m_storedProjectInfo = dataType->m_projectInfo;
		}
		break;
    }
}

void DialogExportSubProject::onSelectOutFolder()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, TEXT_SELECT_DIRECTORY, m_openPath, QFileDialog::ShowDirsOnly);

    if (folderPath != "")
    {
        m_ui.lineEdit_folder->setText(folderPath);
		m_openPath = folderPath;
    }
}

void DialogExportSubProject::onSubProjectInfo()
{
	m_subProjectInfoDialog->setProjectInfos(m_storedProjectInfo);
	m_subProjectInfoDialog->exec();

	m_storedProjectInfo = m_subProjectInfoDialog->getProjectInfos();

}

void DialogExportSubProject::startExport()
{
	m_exportFolder = m_ui.lineEdit_folder->text().toStdWString();
    if (m_exportFolder == "")
    {
		m_dataDispatcher.updateInformation(new GuiDataWarning(TEXT_NO_DIRECTORY_SELECTED));
        return;
    }
	
	m_dataDispatcher.sendControl(new control::io::ExportSubProject(m_exportFolder, m_storedProjectInfo, ObjectStatusFilter(m_ui.objectsFilterComboBox->currentData().toInt()), m_ui.openExplorerFolderCheckBox->isChecked()));
    hide();
}

void DialogExportSubProject::closeEvent(QCloseEvent* event)
{
    cancelExport();
}


void DialogExportSubProject::cancelExport()
{
    hide();
}

void DialogExportSubProject::refreshUI()
{
    
}
