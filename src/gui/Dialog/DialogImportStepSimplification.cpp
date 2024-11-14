#include "gui/Dialog/DialogImportStepSimplification.h"

#include "gui/widgets/CustomWidgets/qdoubleedit.h"
#include "gui/widgets/CustomWidgets/regexpedit.h"

#include "controller/controls/ControlMeshObject.h"

#include "gui/texts/FileTypeTexts.hpp"
#include "gui/Texts.hpp"
#include "gui/GuiData/GuiDataIO.h"

#include "magic_enum/magic_enum.hpp"
#include "utils/Utils.h"

#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qmessagebox.h>
#include <QtCore/qstandardpaths.h>


DialogImportStepSimplification::DialogImportStepSimplification(IDataDispatcher& dataDispatcher, QWidget *parent)
	: ADialog(dataDispatcher, parent)
{
	m_ui.setupUi(this);
	m_ui.keepPercentLineEdit->setRules(ANumericLineEdit::LineEditRules::Nothing);
	m_ui.keepPercentLineEdit->setUnit(UnitType::NO_UNIT);
	m_ui.inputLineEdit->setRegExp(QRegExpEdit::FilePath);
	m_ui.outputLineEdit->setRegExp(QRegExpEdit::FilePath);

	m_importAfter = false;
	m_openPath = QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory);

	connect(m_ui.keepPercentHorizontalSlider, &QSlider::valueChanged, this, [this](int keep) { updateKeepPercent(keep); });
	connect(m_ui.keepPercentLineEdit, &QDoubleEdit::editingFinished, this, [this]() { updateKeepPercent(m_ui.keepPercentLineEdit->getValue()); });

	connect(m_ui.inputToolButton, &QPushButton::clicked, this, &DialogImportStepSimplification::onInputBrowser);
	connect(m_ui.outputToolButton, &QPushButton::clicked, this, &DialogImportStepSimplification::onOutputBrowser);

    connect(m_ui.cancelButton, &QPushButton::clicked, this, &DialogImportStepSimplification::onCancel);
	connect(m_ui.simplificateButton, &QPushButton::clicked, this, &DialogImportStepSimplification::onSimplify);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectPath);

    adjustSize();
}

DialogImportStepSimplification::~DialogImportStepSimplification()
{}

void DialogImportStepSimplification::informData(IGuiData *data)
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

void DialogImportStepSimplification::setImportInputData(const FileInputData& data)
{
	m_data = data;
	m_importAfter = true;

	m_ui.inputLineEdit->setText(QString::fromStdWString(m_data.file.wstring()));
	m_ui.inputLineEdit->setEnabled(false);
	m_ui.inputToolButton->setEnabled(false);
}

void DialogImportStepSimplification::onCancel()
{
	m_ui.inputLineEdit->setText("");
	m_ui.inputLineEdit->setEnabled(true);
	m_ui.inputToolButton->setEnabled(true);

    hide();
}

void DialogImportStepSimplification::onSimplify()
{
	std::filesystem::path outputPath = m_ui.outputLineEdit->text().toStdWString();
	if (outputPath.empty())
	{
		QMessageBox modal(QMessageBox::Icon::Warning, TEXT_DIALOG_ERROR_OUTPUT_FOLDER, TEXT_DIALOG_ERROR_OUTPUT_FOLDER, QMessageBox::StandardButton::Ok);
		modal.exec();
		return;
	}
	if (!std::filesystem::exists(m_data.file))
	{
		QMessageBox modal(QMessageBox::Icon::Warning, TEXT_DIALOG_ERROR_INPUT_PATH, TEXT_DIALOG_ERROR_INPUT_PATH, QMessageBox::StandardButton::Ok);
		modal.exec();
		return;
	}

	StepClassification classification = m_ui.complexityRadioButton->isChecked() ? StepClassification::Complexity : (m_ui.similarityRadioButton->isChecked() ? StepClassification::Similarity : StepClassification::Volume);
	double keepPercent = (m_ui.keepPercentLineEdit->getValue() / 100.0);

	std::string filename = Utils::to_utf8(m_data.file.stem().wstring()) + '_' + std::string(magic_enum::enum_name<StepClassification>(classification)) + '_' + std::to_string(keepPercent) + Utils::to_utf8(m_data.file.filename().extension().wstring());

	m_dataDispatcher.sendControl(new control::meshObject::StepSimplification(m_data, classification, (m_ui.keepPercentLineEdit->getValue() / 100.0), outputPath / filename, m_importAfter));
	
	onCancel();
}

void DialogImportStepSimplification::onInputBrowser()
{
	m_ui.inputLineEdit->setText(QFileDialog::getOpenFileName(this, TEXT_DIALOG_BROWSER_TITLE_IMPORT, m_openPath, TEXT_FILE_TYPE_STEP, nullptr));

	m_data.file = m_ui.inputLineEdit->text().toStdWString();
	if (!m_data.file.empty())
		m_openPath = QString::fromStdWString(m_data.file.parent_path().wstring());
	
}

void DialogImportStepSimplification::onOutputBrowser()
{
	m_ui.outputLineEdit->setText(QFileDialog::getExistingDirectory(this, TEXT_SELECT_DIRECTORY, m_openPath, QFileDialog::ShowDirsOnly));

	std::filesystem::path outputTemp = m_ui.outputLineEdit->text().toStdWString();
	if (!outputTemp.empty())
		m_openPath = QString::fromStdWString(outputTemp.parent_path().wstring());
}

void DialogImportStepSimplification::updateKeepPercent(double keep)
{
	if (keep > 99.)
		keep = 99.;
	else if (keep < 1.)
		keep = 1.;

	m_ui.keepPercentLineEdit->blockSignals(true);
	m_ui.keepPercentHorizontalSlider->blockSignals(true);

	m_ui.keepPercentLineEdit->setValue(keep);
	m_ui.keepPercentHorizontalSlider->setValue(keep);

	m_ui.keepPercentLineEdit->blockSignals(false);
	m_ui.keepPercentHorizontalSlider->blockSignals(false);
}
