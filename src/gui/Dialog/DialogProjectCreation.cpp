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

#define TEMPLATE_BASE_EN "English"
#define TEMPLATE_BASE_FR L"Français"

DialogProjectCreation::DialogProjectCreation(IDataDispatcher& dataDispatcher, QString folderPath, const std::vector<std::filesystem::path>& templates, const std::unordered_map<LangageType, ProjectTemplate>& projectTemplates, QWidget* parent)
    : ADialog(dataDispatcher, parent)
{
    // Init Ui
    m_ui.setupUi(this);

    // Line Edit
    m_ui.lineEditBuisness->setRegExp(QRegExpEdit::AlphaNumericWithSeparator);
    m_ui.lineEditLocation->setRegExp(QRegExpEdit::AlphaNumericWithSeparator);
    m_ui.lineEditPath->setRegExp(QRegExpEdit::Path);
    m_ui.lineEditName->setRegExp(QRegExpEdit::AlphaNumericWithSeparator);
    m_ui.lineEditCustomScanFolder->setRegExp(QRegExpEdit::Path);

    m_ui.lineEditCustomScanFolder->setEnabled(false);
    m_ui.toolButtonCustomScanFolder->setEnabled(false);

    m_ui.checkCentralProject->setVisible(false);


    if (folderPath.isEmpty())
        m_openPath = QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory);
    else
        m_openPath = folderPath;

    m_ui.lineEditPath->setText(m_openPath);

    m_ui.comboBoxProjectTemplate->addItem(QString(), QVariant(""));
    m_ui.comboBoxProjectTemplate->addItem(QString(TEMPLATE_BASE_EN), QVariant(LangageType::English));
    m_ui.comboBoxProjectTemplate->addItem(QString::fromStdWString(TEMPLATE_BASE_FR), QVariant(LangageType::Francais));
    for (const std::filesystem::path& p : templates)
        m_ui.comboBoxProjectTemplate->addItem(QString::fromStdWString(p.filename().wstring()), QVariant(QString::fromStdWString(p.wstring())));

    m_projectTemplates = projectTemplates;
    
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
    infos.m_company = m_ui.lineEditBuisness->text().toStdWString();
    infos.m_location = m_ui.lineEditLocation->text().toStdWString();
    infos.m_description = m_ui.textEditDescription->toPlainText().toStdWString();
    
    //new 
    infos.m_id = xg::newGuid();
    infos.m_centralProjectPath = std::filesystem::path();
    infos.m_centralId = xg::Guid();
    infos.m_isCentral = false;
    //infos.m_isCentral = (m_ui.checkCentralProject->checkState() == Qt::CheckState::Checked);

    infos.m_customScanFolderPath = m_ui.lineEditCustomScanFolder->text().toStdWString();

    NewProjectMessage* message;
    int selectedInd = m_ui.comboBoxProjectTemplate->currentIndex();
    if (selectedInd == 1 || selectedInd == 2)
        message = new NewProjectMessage(infos, folderPath / infos.m_projectName, m_projectTemplates[LangageType(selectedInd - 1)]);
    else
        message = new NewProjectMessage(infos, folderPath / infos.m_projectName, m_ui.comboBoxProjectTemplate->currentData().toString().toStdWString());

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
