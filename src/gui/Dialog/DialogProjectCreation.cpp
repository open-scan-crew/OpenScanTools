#include "gui/Dialog/DialogProjectCreation.h"
#include "controller/controls/ControlFunction.h"
#include "controller/messages/NewProjectMessage.h"
#include "gui/Texts.hpp"
#include "gui/texts/FileSystemTexts.hpp"
#include "gui/Translator.h"

#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qmessagebox.h>
#include <QtCore/qstandardpaths.h>
#include "gui/widgets/CustomWidgets/regexpedit.h"
#include "utils/System.h"
#include "utils/Logger.h"

DialogProjectCreation::DialogProjectCreation(IDataDispatcher& dataDispatcher, QWidget* parent)
    : ADialog(dataDispatcher, parent)
{
    // Init Ui
    m_ui.setupUi(this);

    // Line Edit
    m_ui.lineEditBusiness->setRegExp(QRegExpEdit::AlphaNumericWithSeparator);
    m_ui.lineEditLocation->setRegExp(QRegExpEdit::AlphaNumericWithSeparator);
    m_ui.lineEditPath->setRegExp(QRegExpEdit::Path);
    m_ui.lineEditName->setRegExp(QRegExpEdit::AlphaNumericWithSeparator);
    m_ui.lineEditCustomScanFolder->setRegExp(QRegExpEdit::Path);

    m_ui.lineEditCustomScanFolder->setEnabled(false);
    m_ui.toolButtonCustomScanFolder->setEnabled(false);

    m_ui.comboBoxProjectTemplate->addItem(Translator::getLanguageQStr(LanguageType::English), QVariant((int)LanguageType::English));
    m_ui.comboBoxProjectTemplate->addItem(Translator::getLanguageQStr(LanguageType::Francais), QVariant((int)LanguageType::Francais));

    connect(m_ui.toolButtonPath, &QPushButton::clicked, this, &DialogProjectCreation::launchFileBrowser);
    connect(m_ui.buttonBox->button(QDialogButtonBox::StandardButton::Ok), &QPushButton::clicked, this, &DialogProjectCreation::createProject);
    connect(m_ui.buttonBox->button(QDialogButtonBox::StandardButton::Cancel), &QPushButton::clicked, this, &DialogProjectCreation::onCancel);
    connect(m_ui.checkBoxCustomScanFolder, &QCheckBox::stateChanged, this, [this]()
        {
            bool customFolderState = m_ui.checkBoxCustomScanFolder->isChecked();
            m_ui.lineEditCustomScanFolder->setEnabled(customFolderState);
            m_ui.toolButtonCustomScanFolder->setEnabled(customFolderState);
        });
    connect(m_ui.toolButtonCustomScanFolder, &QPushButton::clicked, this, &DialogProjectCreation::launchFileBrowserCustomScanFolder);

    adjustSize();

    this->setAttribute(Qt::WA_DeleteOnClose);
}

DialogProjectCreation::~DialogProjectCreation()
{
}

void DialogProjectCreation::setDefaultValues(const std::filesystem::path& folderPath, std::wstring default_name, std::wstring default_company)
{
    if (folderPath.empty())
        m_openPath = QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory);
    else
        m_openPath = QString::fromStdWString(folderPath.wstring());

    m_ui.lineEditPath->setText(m_openPath);

    if (!default_name.empty())
        m_ui.lineEditName->setText(QString::fromStdWString(default_name));

    if (!default_company.empty())
        m_ui.lineEditBusiness->setText(QString::fromStdWString(default_company));
}

void DialogProjectCreation::setAdditionalTemplatesPath(const std::vector<std::filesystem::path>& templates_path)
{
    for (const std::filesystem::path& p : templates_path)
        m_ui.comboBoxProjectTemplate->addItem(QString::fromStdWString(p.filename().wstring()), QVariant(QString::fromStdWString(p.wstring())));
}

void DialogProjectCreation::informData(IGuiData* keyValue)
{}

void DialogProjectCreation::createProject()
{
    if (m_ui.lineEditPath->text() == "" || m_ui.lineEditName->text() == "")
    {
        QMessageBox::warning(this, TEXT_PROJECT_ERROR_TITLE, TEXT_PROJECT_ERROR_NOT_EMPTY);
        return;
    }

    ProjectInfos infos;
    std::filesystem::path folderPath(m_ui.lineEditPath->text().toStdWString());
    std::wstring projectName = m_ui.lineEditName->text().toStdWString();
    //On enlève les espaces à la fin car les nom de dossiers ne peuvent pas avoir d'espaces à la fin
    projectName.erase(projectName.find_last_not_of(L' ') + 1);
    Utils::System::formatFilename(projectName);
    infos.m_projectName = projectName;
    infos.m_company = m_ui.lineEditBusiness->text().toStdWString();
    infos.m_location = m_ui.lineEditLocation->text().toStdWString();
    infos.m_description = m_ui.textEditDescription->toPlainText().toStdWString();
    infos.m_id = xg::newGuid();
    infos.m_customScanFolderPath = m_ui.lineEditCustomScanFolder->text().toStdWString();

    NewProjectMessage* message;
    int selectedInd = m_ui.comboBoxProjectTemplate->currentIndex();
    QVariant data = m_ui.comboBoxProjectTemplate->currentData();

    std::filesystem::path project_folder = folderPath / infos.m_projectName;

    if (selectedInd < 2)
        message = new NewProjectMessage(infos, project_folder, LanguageType(data.toInt()));
    else
        message = new NewProjectMessage(infos, project_folder, data.toString().toStdWString());

    m_dataDispatcher.sendControl(new control::function::ForwardMessage(message));

    close();
    delete(this);
}

void DialogProjectCreation::launchFileBrowser()
{
    QFileDialog dialog;
    dialog.setModal(true);

    QString qFilepath = dialog.getExistingDirectory(this, TEXT_SELECT_DIRECTORY,
        m_openPath, QFileDialog::ShowDirsOnly);

    if (qFilepath.isEmpty())
        return;

    m_openPath = qFilepath;
    m_ui.lineEditPath->setText(qFilepath);
}

void DialogProjectCreation::launchFileBrowserCustomScanFolder()
{
    QFileDialog dialog;
    dialog.setModal(true);

    QString qFilepath = dialog.getExistingDirectory(this, TEXT_SELECT_CUSTOM_SCAN_FOLDER,
        m_openPath, QFileDialog::ShowDirsOnly);

    if (qFilepath.isEmpty())
        return;

    m_openPath = qFilepath;
    m_ui.lineEditCustomScanFolder->setText(qFilepath);
}

void DialogProjectCreation::onCancel()
{
    m_dataDispatcher.sendControl(new control::function::Abort());
    close();
}
