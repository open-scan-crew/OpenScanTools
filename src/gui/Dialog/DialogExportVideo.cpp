#include "gui/Dialog/DialogExportVideo.h"

#include "controller/controls/ControlIO.h"

#include "gui/Texts.hpp"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataIO.h"

#include "models/graph/ViewPointNode.h"

#include <QtWidgets/qfiledialog.h>
#include <QtCore/qstandardpaths.h>


DialogExportVideo::DialogExportVideo(IDataDispatcher& dataDispatcher, QWidget *parent, float guiScale)
    : ADialog(dataDispatcher, parent)
	, m_parameters()
{
	setModal(false);
    m_ui.setupUi(this);

	m_openPath = QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory);

	connect(m_ui.betweenViewpointsRadioButton, &QPushButton::released, this, &DialogExportVideo::onAnimationModeSelection);
	connect(m_ui.orbital360RadioButton, &QPushButton::released, this, &DialogExportVideo::onAnimationModeSelection);

	connect(m_ui.pushButtonViewPoint1, &QPushButton::released, this, &DialogExportVideo::onViewpoint1Click);
	connect(m_ui.pushButtonViewPoint2, &QPushButton::released, this, &DialogExportVideo::onViewpoint2Click);

    connect(m_ui.folderToolButton, &QToolButton::released, this, &DialogExportVideo::onSelectOutFolder);
	connect(m_ui.fileToolButton, &QToolButton::released, this, &DialogExportVideo::onSelectOutFile);

    connect(m_ui.generatePushButton, &QPushButton::released, this, &DialogExportVideo::startGeneration);
    connect(m_ui.cancelPushButton, &QPushButton::released, this, &DialogExportVideo::cancelGeneration);

    m_dataDispatcher.registerObserverOnKey(this, guiDType::projectPath);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::objectSelected);
	this->setMinimumWidth(344 * guiScale);
	adjustSize();
}

DialogExportVideo::~DialogExportVideo()
{
    m_dataDispatcher.unregisterObserver(this);
}

void DialogExportVideo::informData(IGuiData *data)
{
    switch (data->getType())
    {
		case guiDType::projectPath:
		{
			auto dataType = static_cast<GuiDataProjectPath*>(data);
			m_openPath = QString::fromStdWString(dataType->m_path.wstring());
			m_ui.lineEditViewPoint1->clear();
			m_ui.lineEditViewPoint2->clear();
			m_parameters.start.reset();
			m_parameters.finish.reset();
		}
		break;
		case guiDType::objectSelected:
		{
			if (m_viewpointToEdit != 1 && m_viewpointToEdit != 2)
				return;
			auto dataType = static_cast<GuiDataObjectSelected*>(data);
			if (dataType->m_type != ElementType::ViewPoint)
				return;

			SafePtr<ViewPointNode> viewpoint = static_pointer_cast<ViewPointNode>(dataType->m_object);
			ReadPtr<ViewPointNode> rViewpoint = viewpoint.cget();
			if (!rViewpoint)
				return;
			if (rViewpoint->getProjectionMode() == ProjectionMode::Orthographic)
			{
				m_dataDispatcher.updateInformation(new GuiDataWarning(TEXT_EXPORT_VIDEO_ORTHO_VIEWPOINT));
				return;
			}

			if (m_viewpointToEdit == 1)
			{
				m_parameters.start = viewpoint;
				m_ui.lineEditViewPoint1->setText(QString::fromStdWString(rViewpoint->getComposedName()));
			}
			else if(m_viewpointToEdit == 2)
			{
				m_parameters.finish = viewpoint;
				m_ui.lineEditViewPoint2->setText(QString::fromStdWString(rViewpoint->getComposedName()));
			}

			m_viewpointToEdit = -1;
		}
		break;
    }
}

void DialogExportVideo::onAnimationModeSelection()
{
	bool betweenViewpoints = m_ui.betweenViewpointsRadioButton->isChecked();
	m_ui.betweenViewpointsWidget->setEnabled(betweenViewpoints);
	m_parameters.animMode = betweenViewpoints ? VideoAnimationMode::BETWEENVIEWPOINTS : VideoAnimationMode::ORBITAL;
}

void DialogExportVideo::onViewpoint1Click()
{
	m_ui.lineEditViewPoint1->clear();
	m_viewpointToEdit = 1;
}

void DialogExportVideo::onViewpoint2Click()
{
	m_ui.lineEditViewPoint2->clear();
	m_viewpointToEdit = 2;
}

void DialogExportVideo::onSelectOutFolder()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, TEXT_SELECT_DIRECTORY, m_openPath, QFileDialog::ShowDirsOnly);

    if (folderPath != "")
    {
        m_ui.folderLineEdit->setText(folderPath);
		m_openPath = folderPath;
    }
}

void DialogExportVideo::onSelectOutFile()
{
	QString filePath = QFileDialog::getSaveFileName(this, TEXT_SAVE_FILENAME, m_openPath);

	QString folderPath = QString::fromStdWString(std::filesystem::path(filePath.toStdWString()).parent_path().wstring());
	QString fileName = QString::fromStdWString(std::filesystem::path(filePath.toStdWString()).stem().wstring());
	if (folderPath != "")
	{
		m_ui.folderLineEdit->setText(folderPath);
		m_openPath = folderPath;
	}

	if (fileName != "")
		m_ui.fileLineEdit->setText(fileName);
}

void DialogExportVideo::startGeneration()
{

    std::wstring filename = m_ui.fileLineEdit->text().toStdWString();
    if (filename == L"")
	{
		m_dataDispatcher.updateInformation(new GuiDataWarning(TEXT_MISSING_FILE_NAME));
        return;
    }

	std::filesystem::path exportFolder = m_ui.folderLineEdit->text().toStdWString();
    if (exportFolder.empty())
    {
		m_dataDispatcher.updateInformation(new GuiDataWarning(TEXT_NO_DIRECTORY_SELECTED));
        return;
    }

	m_parameters.animMode = m_ui.betweenViewpointsRadioButton->isChecked() ? VideoAnimationMode::BETWEENVIEWPOINTS : VideoAnimationMode::ORBITAL;
	if (m_parameters.animMode == VideoAnimationMode::BETWEENVIEWPOINTS)
	{
		if (!m_parameters.start || !m_parameters.finish)
		{
			m_dataDispatcher.updateInformation(new GuiDataWarning(TEXT_EXPORT_VIDEO_MISSING_VIEWPOINTS));
			return;
		}

		if (m_parameters.start == m_parameters.finish)
		{
			m_dataDispatcher.updateInformation(new GuiDataWarning(TEXT_EXPORT_VIDEO_SAME_VIEWPOINTS));
			return;
		}
	}
	
	m_parameters.length = m_ui.lengthSpinBox->value();
	m_parameters.fps = m_ui.fpsSpinBox->value();
	m_parameters.hdImage = m_ui.imageHDRadioButton->isChecked();
	m_parameters.openFolderAfterExport = m_ui.openExplorerFolderCheckBox->isChecked();
	m_parameters.interpolateRenderingBetweenViewpoints = m_ui.interpolateCheckBox->isChecked();

	m_dataDispatcher.sendControl(new control::io::GenerateVideoHD(exportFolder / filename, m_parameters));

    hide();
}

void DialogExportVideo::closeEvent(QCloseEvent* event)
{
	cancelGeneration();
}


void DialogExportVideo::cancelGeneration()
{
    hide();
}