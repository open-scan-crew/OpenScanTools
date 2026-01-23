#include "gui/Dialog/DialogExportVideo.h"

#include "controller/controls/ControlIO.h"

#include "gui/Texts.hpp"
#include "gui/texts/FileSystemTexts.hpp"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataIO.h"
#include "utils/Config.h"

#include "models/graph/ViewPointNode.h"

#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpinBox>
#include <QtCore/QProcess>
#include <QtCore/qstandardpaths.h>
#include <filesystem>

namespace
{
QString resolveFfmpegExecutable()
{
    std::filesystem::path ffmpegDir = Config::getFFmpegPath();
    if (!ffmpegDir.empty())
    {
        std::filesystem::path candidate = ffmpegDir / "ffmpeg.exe";
        if (!std::filesystem::exists(candidate))
            candidate = ffmpegDir / "ffmpeg";
        return QString::fromStdWString(candidate.wstring());
    }
    return QStringLiteral("ffmpeg");
}
}


DialogExportVideo::DialogExportVideo(IDataDispatcher& dataDispatcher, QWidget *parent, float guiScale)
    : ADialog(dataDispatcher, parent)
	, m_parameters()
{
	setModal(false);
    m_ui.setupUi(this);

	m_openPath = QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory);

	connect(m_ui.betweenViewpointsRadioButton, &QPushButton::clicked, this, &DialogExportVideo::onAnimationModeSelection);
	connect(m_ui.orbital360RadioButton, &QPushButton::clicked, this, &DialogExportVideo::onAnimationModeSelection);

	connect(m_ui.pushButtonViewPoint1, &QPushButton::clicked, this, &DialogExportVideo::onViewpoint1Click);
	connect(m_ui.pushButtonViewPoint2, &QPushButton::clicked, this, &DialogExportVideo::onViewpoint2Click);

    connect(m_ui.folderToolButton, &QToolButton::clicked, this, &DialogExportVideo::onSelectOutFolder);
	connect(m_ui.fileToolButton, &QToolButton::clicked, this, &DialogExportVideo::onSelectOutFile);

    connect(m_ui.generatePushButton, &QPushButton::clicked, this, &DialogExportVideo::startGeneration);
    connect(m_ui.cancelPushButton, &QPushButton::clicked, this, &DialogExportVideo::cancelGeneration);

    m_dataDispatcher.registerObserverOnKey(this, guiDType::projectPath);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::objectSelected);
	this->setMinimumWidth(344 * guiScale);
	adjustSize();

    connect(m_ui.mp4RadioButton, &QRadioButton::toggled, this, &DialogExportVideo::onOutputTypeChanged);
    connect(m_ui.imageRadioButton, &QRadioButton::toggled, this, &DialogExportVideo::onOutputTypeChanged);
    onOutputTypeChanged();
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
	
    m_parameters.outputType = m_ui.mp4RadioButton->isChecked() ? VideoExportOutputType::MP4 : VideoExportOutputType::IMAGES;
    m_parameters.bitrateKbps = m_ui.bitrateSpinBox->value();

    if (m_parameters.outputType == VideoExportOutputType::MP4)
    {
        if (!checkResolutionForMp4())
            return;

        if (!isX265Available())
        {
            m_dataDispatcher.updateInformation(new GuiDataWarning(TEXT_EXPORT_VIDEO_MP4_X265_MISSING));
            return;
        }
    }

    std::filesystem::path exportBasePath = exportFolder / filename;
    exportBasePath.replace_extension();
    if (m_parameters.outputType == VideoExportOutputType::MP4)
    {
        m_parameters.outputFilePath = exportBasePath;
        m_parameters.outputFilePath.replace_extension(".mp4");
    }
    else
    {
        m_parameters.outputFilePath.clear();
    }

	m_parameters.length = m_ui.lengthSpinBox->value();
	m_parameters.fps = m_ui.fpsSpinBox->value();
	m_parameters.hdImage = m_ui.imageHDRadioButton->isChecked();
	m_parameters.openFolderAfterExport = m_ui.openExplorerFolderCheckBox->isChecked();
	m_parameters.interpolateRenderingBetweenViewpoints = m_ui.interpolateCheckBox->isChecked();

	m_dataDispatcher.sendControl(new control::io::GenerateVideoHD(exportBasePath, m_parameters));

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

void DialogExportVideo::onOutputTypeChanged()
{
    bool mp4Selected = m_ui.mp4RadioButton->isChecked();
    m_ui.bitrateSpinBox->setEnabled(mp4Selected);
    m_ui.bitrateLabel->setEnabled(mp4Selected);
}

std::optional<std::pair<uint32_t, uint32_t>> DialogExportVideo::currentImageResolution() const
{
    for (QWidget* topLevel : QApplication::topLevelWidgets())
    {
        if (!topLevel)
            continue;
        QLineEdit* widthEdit = topLevel->findChild<QLineEdit*>("lineEdit_imageW");
        QLineEdit* heightEdit = topLevel->findChild<QLineEdit*>("lineEdit_imageH");
        if (widthEdit && heightEdit)
        {
            bool okW = false;
            bool okH = false;
            uint32_t w = widthEdit->text().toUInt(&okW, 10);
            uint32_t h = heightEdit->text().toUInt(&okH, 10);
            if (okW && okH)
                return std::make_pair(w, h);
            return std::nullopt;
        }
    }
    return std::nullopt;
}

bool DialogExportVideo::checkResolutionForMp4() const
{
    auto res = currentImageResolution();
    if (!res.has_value())
        return true;

    uint64_t pixels = static_cast<uint64_t>(res->first) * static_cast<uint64_t>(res->second);
    if (pixels > MAX_MP4_PIXELS)
    {
        m_dataDispatcher.updateInformation(new GuiDataWarning(TEXT_EXPORT_VIDEO_MP4_RESOLUTION_TOO_HIGH));
        return false;
    }
    return true;
}

bool DialogExportVideo::isX265Available() const
{
    QProcess ffmpegCheck;
    ffmpegCheck.start(resolveFfmpegExecutable(), { "-hide_banner", "-encoders" });
    if (!ffmpegCheck.waitForStarted(3000))
    {
        ffmpegCheck.kill();
        return false;
    }
    if (!ffmpegCheck.waitForFinished(3000))
    {
        ffmpegCheck.kill();
        return false;
    }

    QString output = ffmpegCheck.readAllStandardOutput();
    output.append(ffmpegCheck.readAllStandardError());
    return output.contains("libx265", Qt::CaseInsensitive) || output.contains("x265", Qt::CaseInsensitive);
}
