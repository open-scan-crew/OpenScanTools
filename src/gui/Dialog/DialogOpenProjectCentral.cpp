#include "gui/Dialog/DialogOpenProjectCentral.h"
#include "controller/controls/ControlModal.h"

#include "gui/texts/FileSystemTexts.hpp"

#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qfiledialog.h>
#include <QtCore/qstandardpaths.h>

DialogOpenProjectCentral::DialogOpenProjectCentral(IDataDispatcher& dataDispatcher, QWidget* parent)
    : ADialog(dataDispatcher, parent)
{
    m_ui.setupUi(this);
    m_openPath = QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory);

   connect(m_ui.buttonBox->button(QDialogButtonBox::StandardButton::Ok), &QPushButton::clicked, this, &DialogOpenProjectCentral::openProjectCentral);
   connect(m_ui.buttonBox->button(QDialogButtonBox::StandardButton::Cancel), &QPushButton::clicked, this, &DialogOpenProjectCentral::onCancel);
   connect(m_ui.pushButton, &QPushButton::clicked, this, &DialogOpenProjectCentral::openProjectLocal);
}

DialogOpenProjectCentral::~DialogOpenProjectCentral()
{
}

void DialogOpenProjectCentral::setCentralProjectPath(const std::filesystem::path& centralPath)
{
    m_centralPath = centralPath;
}

void DialogOpenProjectCentral::openProjectCentral()
{
    m_dataDispatcher.sendControl(new control::modal::ModalReturnValue(1));
    hide();
}

void DialogOpenProjectCentral::onCancel()
{

    m_dataDispatcher.sendControl(new control::modal::ModalReturnValue(0));
    hide();
}

void DialogOpenProjectCentral::openProjectLocal()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, TEXT_SELECT_DIRECTORY, m_openPath, QFileDialog::Option());

    if (folderPath != "")
    {
        m_openPath = folderPath;
        std::filesystem::path folder = folderPath.toStdWString();
        m_dataDispatcher.sendControl(new control::modal::ModalReturnValue(0));
        hide();

        //A faire : création du projet local à partir du projet central
    }

}

void DialogOpenProjectCentral::informData(IGuiData* keyValue)
{}

