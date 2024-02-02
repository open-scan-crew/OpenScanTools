#include "gui/widgets/NewProjectDialog.h"
#include "controller/controls/ControlFunction.h"
#include "controller/messages/NewProjectMessage.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/Texts.hpp"

#include "utils/Config.h"

#include <QtWidgets/QGridLayout>
#include <QtGui/QIcon>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtCore/QStandardPaths>
#include "gui/widgets/CustomWidgets/regexpedit.h"
//FIXME
#include <winrt/Windows.Foundation.Collections.h>

NewProjectDialog::NewProjectDialog(IDataDispatcher &dataDispatcher, QString folderPath, QWidget *parent)
    : QDialog(parent)
    , m_dataDispatcher(dataDispatcher)
{
    // Init Ui
    this->setModal(true);
    this->setWindowTitle(TEXT_PROJECT_CREATION);

    // Labels
    QLabel* projectNameLabel = new QLabel(TEXT_ENTER_PROJECT_NAME + " : ", this);
    QLabel* projectDirPathLabel = new QLabel(TEXT_ENTER_PROJECT_DIRECTORY + " : ", this);
    QLabel* companyNameLabel = new QLabel(TEXT_COMPANY + " : ", this);
    QLabel* locationLabel = new QLabel(TEXT_LOCATION + " : ", this);
    QLabel* descriptionLabel = new QLabel(TEXT_DESCRIPTION + " : ", this);

    // Line Edit
    m_projectName = new QRegExpEdit(QRegExpEdit::AlphaNumericWithSeparator, this);
    m_projectDirPath = new QRegExpEdit(QRegExpEdit::Path, this);
    m_companyName = new QRegExpEdit(QRegExpEdit::Name, this);
    m_location = new QRegExpEdit(QRegExpEdit::Name, this);
    m_description = new QTextEdit(this);

    if(folderPath.isEmpty())
	    m_projectDirPath->setText(QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory));
    else
        m_projectDirPath->setText(folderPath);
    // Buttons
    const QIcon openIcon = QIcon(":icons/100x100/open_project.png");
    QPushButton* openFileBrowser = new QPushButton(this);
    openFileBrowser->setIcon(openIcon);

    QPushButton* validateProjectCreation = new QPushButton(TEXT_CREATE_PROJECT, this);
    QPushButton* cancel = new QPushButton(TEXT_CANCEL, this);

    connect(openFileBrowser, &QPushButton::clicked, this, &NewProjectDialog::launchFileBrowser);
    connect(validateProjectCreation, &QPushButton::clicked, this, &NewProjectDialog::createProject);
    connect(cancel, &QPushButton::clicked, this, &NewProjectDialog::onCancel);

    // Layout
    QGridLayout * layout = new QGridLayout(this);
    layout->addWidget(projectNameLabel, 0, 0);
    layout->addWidget(projectDirPathLabel, 1, 0);
    layout->addWidget(companyNameLabel, 2, 0);
    layout->addWidget(locationLabel, 3, 0);
    layout->addWidget(descriptionLabel, 4, 0);

    layout->addWidget(m_projectName, 0, 1);
    layout->addWidget(m_projectDirPath, 1, 1);
    layout->addWidget(m_companyName, 2, 1);
    layout->addWidget(m_location, 3, 1);
    layout->addWidget(m_description, 4, 1);

    layout->addWidget(openFileBrowser, 1, 2);
    layout->addWidget(validateProjectCreation, 5, 0);
    layout->addWidget(cancel, 5, 1);

    this->setLayout(layout);
}

NewProjectDialog::~NewProjectDialog()
{

}

void NewProjectDialog::informData(IGuiData *keyValue)
{
    // Nothing
}

QString NewProjectDialog::getName() const
{
    return (TEXT_NEWPROJ_DIALOG_NAME);
}

void NewProjectDialog::createProject()
{
	if (m_projectName->text() == "" || m_projectDirPath->text() == "")
	{
		QMessageBox::warning(this, TEXT_PROJECT_ERROR_TITLE, TEXT_PROJECT_ERROR_NOT_EMPTY);
		return;
	}

	ProjectInfo infos;
    std::filesystem::path folderPath(m_projectDirPath->text().toStdWString());

	infos.m_projectName = m_projectName->text().toStdWString();
	infos.m_company = m_companyName->text().toStdString();
	infos.m_location = m_location->text().toStdString();
	infos.m_description = m_description->toPlainText().toStdString();

    NewProjectMessage* message = new NewProjectMessage(infos, folderPath);
    m_dataDispatcher.sendControl(new control::function::ForwardMessage(message));

    m_dataDispatcher.updateInformation(new GuiDataUiInternalModificationReset());
    close();
	delete(this);
}

void NewProjectDialog::launchFileBrowser()
{
    QFileDialog dialog;
    dialog.setModal(true);

    QString qFilepath = dialog.getExistingDirectory(this, TEXT_SELECT_DIRECTORY,
        QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory), QFileDialog::ShowDirsOnly | QFileDialog::DontUseNativeDialog);
        
    if (qFilepath.isEmpty())
        return;
    m_projectDirPath->setText(qFilepath);
}

void NewProjectDialog::onCancel()
{
    m_dataDispatcher.sendControl(new control::function::Abort());
    close();
}