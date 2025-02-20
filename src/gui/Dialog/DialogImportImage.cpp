#include "gui/Dialog/DialogImportImage.h"
#include "gui/texts/FileTypeTexts.hpp"

#include <gui/GuiData/GuiDataIO.h>
#include "gui/Texts.hpp"
#include "controller/controls/ControlIO.h"

#include <qstandardpaths.h>
#include <qmessagebox.h>
#include <qfiledialog.h>

DialogImportImage::DialogImportImage(IDataDispatcher& dataDispatcher, QWidget *parent)
	: ADialog(dataDispatcher, parent)
{
	m_ui.setupUi(this);
	m_ui.lengthLineEdit->setRules(ANumericLineEdit::LineEditRules::PositiveStrict);
	m_ui.lengthLineEdit->setType(NumericType::DISTANCE);
	m_ui.xOriginLineEdit->setType(NumericType::DISTANCE);
	m_ui.yOriginLineEdit->setType(NumericType::DISTANCE);
	m_ui.zOriginLineEdit->setType(NumericType::DISTANCE);

	m_ui.inputLineEdit->setRegExp(QRegExpEdit::FilePath);
	m_ui.outputLineEdit->setRegExp(QRegExpEdit::FilePath);

	m_openPath = QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory);

	connect(m_ui.inputToolButton, &QPushButton::clicked, this, &DialogImportImage::onInputBrowser);
	connect(m_ui.outputToolButton, &QPushButton::clicked, this, &DialogImportImage::onOutputBrowser);

    connect(m_ui.cancelButton, &QPushButton::clicked, this, &DialogImportImage::onCancel);
	connect(m_ui.convertButton, &QPushButton::clicked, this, &DialogImportImage::onConvert);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectPath);

    adjustSize();
}

DialogImportImage::~DialogImportImage()
{}

void DialogImportImage::informData(IGuiData *data)
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

void DialogImportImage::onCancel()
{
    hide();
}

void DialogImportImage::onConvert()
{
	std::filesystem::path inputPath = m_ui.inputLineEdit->text().toStdWString();
	if (!std::filesystem::exists(inputPath))
	{
		QMessageBox modal(QMessageBox::Icon::Warning, TEXT_DIALOG_ERROR_INPUT_PATH, TEXT_DIALOG_ERROR_INPUT_PATH, QMessageBox::StandardButton::Ok);
		modal.exec();
		return;
	}

	std::filesystem::path outputFolder = m_ui.outputLineEdit->text().toStdWString();
	if (outputFolder.empty())
	{
		QMessageBox modal(QMessageBox::Icon::Warning, TEXT_DIALOG_ERROR_OUTPUT_FOLDER, TEXT_DIALOG_ERROR_OUTPUT_FOLDER, QMessageBox::StandardButton::Ok);
		modal.exec();
		return;
	}

	std::filesystem::path outputFilename = m_ui.outputFileNamelineEdit->text().toStdWString();
	if (outputFilename.empty())
	{
		QMessageBox modal(QMessageBox::Icon::Warning, TEXT_DIALOG_ERROR_OUTPUT_PATH, TEXT_DIALOG_ERROR_OUTPUT_PATH, QMessageBox::StandardButton::Ok);
		modal.exec();
		return;
	}

	control::io::ConvertImageToPointCloud::ConvertImage params;
	
	params.inputImage = QImage(QString::fromStdWString(inputPath.wstring()));
	params.outputPath = outputFolder / m_ui.outputFileNamelineEdit->text().toStdWString();
	params.outputPath.replace_extension(FileUtils::getExtension(FileType::PTS));
	params.normalAxeMode = m_ui.xyOrientationRadioButton->isChecked() ? 0 :
							m_ui.xzOrientationRadioButton->isChecked() ? 1 : 2;
	params.length = m_ui.lengthLineEdit->getValue();
	params.origin = glm::vec3(m_ui.xOriginLineEdit->getValue(), m_ui.yOriginLineEdit->getValue(), m_ui.zOriginLineEdit->getValue());
	params.colorTransparencyMode = m_ui.ignoreTransparencyRadioButton->isChecked() ? 0 :
									m_ui.whiteTransparencyRadioButton->isChecked() ? 1 : 2;

	m_dataDispatcher.sendControl(new control::io::ConvertImageToPointCloud(params));
	
	onCancel();
}

void DialogImportImage::onInputBrowser()
{
	m_ui.inputLineEdit->setText(QFileDialog::getOpenFileName(this, TEXT_DIALOG_BROWSER_TITLE_IMPORT, m_openPath, TEXT_FILE_TYPE_IMAGE, nullptr));

	std::filesystem::path inputPath = m_ui.inputLineEdit->text().toStdWString();
	if (!inputPath.empty())
		m_openPath = QString::fromStdWString(inputPath.parent_path().wstring());
	
	m_ui.outputFileNamelineEdit->setText(QString::fromStdWString(inputPath.stem().wstring()));
}

void DialogImportImage::onOutputBrowser()
{
	m_ui.outputLineEdit->setText(QFileDialog::getExistingDirectory(this, TEXT_SELECT_DIRECTORY, m_openPath, QFileDialog::ShowDirsOnly));

	std::filesystem::path outputTemp = m_ui.outputLineEdit->text().toStdWString();
	if (!outputTemp.empty())
		m_openPath = QString::fromStdWString(outputTemp.parent_path().wstring());
}
