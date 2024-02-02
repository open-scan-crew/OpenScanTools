#include <QtWidgets/QFileDialog>
#include <QtCore/QStandardPaths>

#include "gui/Dialog/HyperlinkAddDialog.h"
#include "controller/controls/ControlApplication.h"
#include "gui/Texts.hpp"
#include "controller/controls/ControlDataEdition.h"

#include "gui/GuiData/GuiDataIO.h"

#include <qevent.h>

HyperlinkAddDialog::HyperlinkAddDialog(IDataDispatcher& dataDispacher, QWidget *parent)
	: ADialog(dataDispacher, parent)
	, ui(new Ui::AddHyperlinkDialog)
{
	// ----- Important | Window Flags -----
	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::MSWindowsFixedSizeDialogHint;
	flags ^= Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);
	// ------------------------------------

	ui->setupUi(this);
	ui->URLInfield->setEnabled(false);
	ui->FileInfield->setEnabled(false);
	ui->FileBtn->setEnabled(false);
	ui->isRelativePathCheckBox->setEnabled(false);

	QObject::connect(ui->URLRadio, SIGNAL(clicked()), this, SLOT(enableURL()));
	QObject::connect(ui->FileRadio, SIGNAL(clicked()), this, SLOT(enableFile()));
	QObject::connect(ui->FileBtn, SIGNAL(clicked()), this, SLOT(clickfileSearch()));
	QObject::connect(ui->CancelBtn, SIGNAL(clicked()), this, SLOT(cancelCreation()));
	QObject::connect(ui->OkBtn, SIGNAL(clicked()), this, SLOT(acceptCreation()));
	QObject::connect(ui->isRelativePathCheckBox, &QCheckBox::stateChanged, this, &HyperlinkAddDialog::updateFilePath);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectPath);
}

HyperlinkAddDialog::~HyperlinkAddDialog()
{
}

void HyperlinkAddDialog::closeEvent(QCloseEvent* event)
{
	event->ignore();
	close();
	accept();
}

void HyperlinkAddDialog::informData(IGuiData* data)
{
	switch (data->getType())
	{
		case guiDType::projectPath:
		{
			auto dataType = static_cast<GuiDataProjectPath*>(data);
			m_projectFolderPath = dataType->m_path;
			m_openPath = QString::fromStdWString(m_projectFolderPath);
		}
	}
}

void HyperlinkAddDialog::enableURL()
{
	if (ui->URLRadio->isChecked() == true)
	{
		ui->URLInfield->setEnabled(true);
		ui->FileInfield->setEnabled(false);
		ui->FileBtn->setEnabled(false);
		ui->isRelativePathCheckBox->setEnabled(false);
	}
}

void HyperlinkAddDialog::enableFile()
{
	if (ui->FileRadio->isChecked() == true)
	{
		ui->FileInfield->setEnabled(true);
		ui->FileBtn->setEnabled(true);
		ui->isRelativePathCheckBox->setEnabled(true);
		ui->URLInfield->setEnabled(false);
	}
}

void HyperlinkAddDialog::clickfileSearch()
{
	QFileDialog dialog;
	dialog.setModal(true);

	QString qFilepath = dialog.getOpenFileName(this, TEXT_SELECT_DIRECTORY, m_openPath);

	if (qFilepath.isEmpty())
		return;

	m_openPath = qFilepath;
	m_absPath = qFilepath.toStdWString();
	updateFilePath();

}

void HyperlinkAddDialog::updateFilePath()
{
	std::filesystem::path path;
	if (ui->isRelativePathCheckBox->isChecked())
		path = std::filesystem::proximate(m_absPath, m_projectFolderPath);
	else
		path = m_absPath;

	QUrl fileUrl = QUrl::fromLocalFile(QString::fromStdWString(path.wstring()));
	ui->FileInfield->blockSignals(true);
	ui->FileInfield->setText(fileUrl.toString());
	ui->FileInfield->blockSignals(false);
}

void HyperlinkAddDialog::acceptCreation()
{
	std::wstring name = ui->NameInfield->text().toStdWString();

	if (ui->FileRadio->isChecked() == true && !ui->FileInfield->text().isEmpty())
	{
		if (ui->NameInfield->text().isEmpty())
			name = ui->FileInfield->text().toStdWString();
		emit onCreatedLink(ui->FileInfield->text().toStdWString(), name);
		ui->URLInfield->clear();
		ui->FileInfield->clear();
		ui->NameInfield->clear();
		hide();
	}
	else if (ui->URLRadio->isChecked() == true && ui->URLInfield->text() != "")
	{
		if (ui->NameInfield->text().isEmpty())
			name = ui->URLInfield->text().toStdWString();
		emit onCreatedLink(ui->URLInfield->text().toStdWString(), name);
		ui->URLInfield->clear();
		ui->FileInfield->clear();
		ui->NameInfield->clear();
		hide();
	}
	//ajouter popupo de refus
}

void HyperlinkAddDialog::cancelCreation()
{
	ui->URLInfield->clear();
	ui->FileInfield->clear();
	ui->NameInfield->clear();
	hide();
}