#include "gui/Dialog/DialogImportPCObject.h"
#include "gui/widgets/CustomWidgets/regexpedit.h"

#include "controller/controls/ControlProject.h"

#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataIO.h"
#include "gui/texts/FileTypeTexts.hpp"
#include "gui/Texts.hpp"
#include "gui/texts/FileSystemTexts.hpp"

#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qmessagebox.h>

#include "gui\Dialog\DialogImportAsciiPC.h"


DialogImportPCObject::DialogImportPCObject(IDataDispatcher& dataDispatcher, QWidget *parent)
	: ADialog(dataDispatcher, parent)
	, m_openPath(QString())
	, m_storedPosition(glm::vec3(0.f, 0.f, 0.f))
{
    m_ui.setupUi(this);
    m_ui.fileLineEdit->setRegExp(QRegExpEdit::FilePath);

    m_ui.xPosLineEdit->setType(NumericType::DISTANCE);
    m_ui.yPosLineEdit->setType(NumericType::DISTANCE);
    m_ui.zPosLineEdit->setType(NumericType::DISTANCE);

    m_dataDispatcher.registerObserverOnKey(this, guiDType::projectPath);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::conversionOptionsDisplay);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::projectDataPropertiesNoOpen);

	m_ui.coordinatesGroup->hide();
    m_ui.scaleFrame->hide(); 

    connect(m_ui.fileToolButton, &QPushButton::clicked, this, &DialogImportPCObject::onFileBrowser);
    connect(m_ui.okButton, &QPushButton::clicked, this, &DialogImportPCObject::onOk);
    connect(m_ui.cancelButton, &QPushButton::clicked, this, &DialogImportPCObject::onCancel);
	connect(m_ui.insertionRadioButton, &QRadioButton::released, this, [this]() { m_ui.coordinatesGroup->show();
                                                                                 adjustSize(); });
	connect(m_ui.placeManuallyRadioButton, &QRadioButton::released, this, [this]() { m_ui.coordinatesGroup->hide();
                                                                                     adjustSize(); });

	connect(m_ui.xPosLineEdit, &QDoubleEdit::editingFinished, this, &DialogImportPCObject::onPositionEdit);
	connect(m_ui.yPosLineEdit, &QDoubleEdit::editingFinished, this, &DialogImportPCObject::onPositionEdit);
	connect(m_ui.zPosLineEdit, &QDoubleEdit::editingFinished, this, &DialogImportPCObject::onPositionEdit);

  
    adjustSize();
}

DialogImportPCObject::~DialogImportPCObject()
{}


void DialogImportPCObject::informData(IGuiData *data)
{
    if (data->getType() == guiDType::projectPath)
        onProjectPath(data);
}

void DialogImportPCObject::closeEvent(QCloseEvent* close)
{
    onCancel();
}

void DialogImportPCObject::onCancel()
{
    m_dataDispatcher.updateInformation(new GuiDataActivatedFunctions(ContextType::none));
    hide();
}

void DialogImportPCObject::onOk()
{
    if (m_ui.scaleFrame->isVisible())
    {
        QMessageBox modal(QMessageBox::Icon::Information, TEXT_DIALOG_TITLE_IMPORT_WAVEFRONT, TEXT_DIALOG_ERROR_SCALING, QMessageBox::StandardButton::Ok);
        modal.exec();
        return;
    }

    if(m_ui.fileLineEdit->text().isEmpty())
    {
        QMessageBox modal(QMessageBox::Icon::Information, TEXT_ERROR_FILENAME, TEXT_ERROR_FILENAME, QMessageBox::StandardButton::Ok);
        modal.exec();
        return;
    }

    DialogImportAsciiPC importAsciiDialog(this);
    if (importAsciiDialog.setInfoAsciiPC(m_storedPaths))
        importAsciiDialog.exec();

    std::map<std::filesystem::path, Import::AsciiInfo> mapAsciiInfo;
    if (importAsciiDialog.isFinished())
        mapAsciiInfo = importAsciiDialog.getFileImportInfo();

    Import::ScanInfo info;
    info.asObject = true;
    info.paths = m_storedPaths;
    info.mapAsciiInfo = mapAsciiInfo;
    info.positionAsObject = glm::dvec3(m_ui.xPosLineEdit->getValue(), m_ui.yPosLineEdit->getValue(), m_ui.zPosLineEdit->getValue());
    info.positionOption = m_ui.keepModelRadioButton->isChecked() ? PositionOptions::KeepModel : (m_ui.placeManuallyRadioButton->isChecked() ? PositionOptions::ClickPosition : PositionOptions::GivenCoordinates);

    m_dataDispatcher.sendControl(new control::project::ImportScan(info));
		
    hide();
}

void DialogImportPCObject::onPositionEdit()
{
	if (!m_ui.xPosLineEdit->isModified() && !m_ui.yPosLineEdit->isModified() && !m_ui.zPosLineEdit->isModified())
		return;
	m_ui.xPosLineEdit->setModified(false);
	m_ui.yPosLineEdit->setModified(false);
	m_ui.zPosLineEdit->setModified(false);

	float x = (float)m_ui.xPosLineEdit->getValue();
	float y = (float)m_ui.yPosLineEdit->getValue();
	float z = (float)m_ui.zPosLineEdit->getValue();

	m_storedPosition = glm::vec3(x, y, z);
}

void DialogImportPCObject::onFileBrowser()
{
    QStringList files = QFileDialog::getOpenFileNames(this, TEXT_SELECT_DIRECTORY, m_openPath, TEXT_FILE_TYPE_ALL_SCANS_OPEN, nullptr);

    m_storedPaths.clear();
    for (const auto& iterator : files)
        m_storedPaths.push_back(iterator.toStdWString());

    if (!m_storedPaths.empty())
        m_openPath = QString::fromStdWString(files.begin()->toStdWString());

    m_ui.fileLineEdit->setText(m_openPath);
}

void DialogImportPCObject::onProjectPath(IGuiData* data)
{
    GuiDataProjectPath* dataType = static_cast<GuiDataProjectPath*>(data);
    m_openPath = QString::fromStdWString(dataType->m_path.wstring());
}