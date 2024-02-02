#include "gui/Dialog/DialogImportMeshObject.h"
#include "controller/messages/ImportMeshObjectMessage.h"
#include "gui/widgets/CustomWidgets/qdoubleedit.h"
#include "gui/widgets/CustomWidgets/regexpedit.h"


#include "controller/controls/ControlFunction.h"
#include "controller/controls/ControlMeshObject.h"

#include "gui/GuiData/GuiDataRendering.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataIO.h"
#include "gui/texts/FileTypeTexts.hpp"
#include "gui/Texts.hpp"

#include <QtWidgets/QPushButton>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtCore/QStandardPaths>

#include <utils/QtUtils.h>

static const std::list<std::pair<QVariant, QString>> DirectionList = {{QVariant((uint32_t)Selection::X), "X"}
                                                                    ,{QVariant((uint32_t)Selection::_X), "-X"}
                                                                    ,{QVariant((uint32_t)Selection::Y), "Y"}
                                                                    ,{QVariant((uint32_t)Selection::_Y), "-Y"}
                                                                    ,{QVariant((uint32_t)Selection::Z), "Z"}
                                                                    ,{QVariant((uint32_t)Selection::_Z), "-Z"} };

DialogImportMeshObject::DialogImportMeshObject(IDataDispatcher& dataDispatcher, QWidget *parent)
	: ADialog(dataDispatcher, parent)
	, m_openPath(QString())
	, m_storedPosition(glm::vec3(0.f, 0.f, 0.f))
    , m_storedScanTruncateCoor(glm::dvec3(0., 0., 0.))
	, m_dialogStepSimplification(dataDispatcher, parent)
    , m_forward(Selection::Y)
    , m_up(Selection::Z)
{
    m_ui.setupUi(this);
    m_ui.scaleLineEdit->setRules(ANumericLineEdit::LineEditRules::PositiveStrict);
    m_ui.fileLineEdit->setRegExp(QRegExpEdit::FilePath);

    m_ui.xPosLineEdit->setType(NumericType::DISTANCE);
    m_ui.yPosLineEdit->setType(NumericType::DISTANCE);
    m_ui.zPosLineEdit->setType(NumericType::DISTANCE);

    m_ui.scaleLineEdit->getValue();
    m_ui.scaleLineEdit->setType(NumericType::DISTANCE);


    initComboBoxes();
    m_dataDispatcher.registerObserverOnKey(this, guiDType::projectPath);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::conversionOptionsDisplay);
    m_dataDispatcher.registerObserverOnKey(this, guiDType::projectDataPropertiesNoOpen);

    m_ui.toSimplificateCheckBox->hide();
	m_ui.coordinatesGroup->hide();
    m_ui.LODBox->hide();
    m_ui.scaleFrame->hide(); 
    m_ui.truncateCoordinatesCheckBox->hide();
    m_ui.overrideScaleCheckBox->setEnabled(false);

    connect(m_ui.fileToolButton, &QPushButton::clicked, this, &DialogImportMeshObject::onFileBrowser);
    connect(m_ui.okButton, &QPushButton::clicked, this, &DialogImportMeshObject::onOk);
    connect(m_ui.cancelButton, &QPushButton::clicked, this, &DialogImportMeshObject::onCancel);
	connect(m_ui.insertionRadioButton, &QRadioButton::released, this, [this]() { m_ui.truncateCoordinatesCheckBox->hide();
                                                                                 m_ui.truncateCoordinatesCheckBox->setChecked(false);
                                                                                 resetCoordinates();
                                                                                 m_ui.coordinatesGroup->show();
                                                                                 adjustSize(); });
    connect(m_ui.keepModelRadioButton, &QRadioButton::released, this, [this]() { m_ui.truncateCoordinatesCheckBox->show();  
                                                                                 m_ui.coordinatesGroup->hide();
                                                                                 adjustSize(); });
	connect(m_ui.placeManuallyRadioButton, &QRadioButton::released, this, [this]() { m_ui.truncateCoordinatesCheckBox->setChecked(false);
                                                                                     m_ui.truncateCoordinatesCheckBox->hide();
                                                                                     resetCoordinates();
                                                                                     m_ui.coordinatesGroup->hide();
                                                                                     adjustSize(); });
    connect(m_ui.overrideScaleCheckBox, &QCheckBox::stateChanged, this, &DialogImportMeshObject::onOverrideScaleUpdate);
    connect(m_ui.truncateCoordinatesCheckBox, &QCheckBox::stateChanged, this, &DialogImportMeshObject::onTruncateCoorOnImport);

    connect(m_ui.upDirectionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) { onComboBoxChanged(0, index); });
    connect(m_ui.forwardDirectionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) { onComboBoxChanged(1, index); });
	
	connect(m_ui.xPosLineEdit, &QDoubleEdit::editingFinished, this, &DialogImportMeshObject::onPositionEdit);
	connect(m_ui.yPosLineEdit, &QDoubleEdit::editingFinished, this, &DialogImportMeshObject::onPositionEdit);
	connect(m_ui.zPosLineEdit, &QDoubleEdit::editingFinished, this, &DialogImportMeshObject::onPositionEdit);

  
    adjustSize();
}

DialogImportMeshObject::~DialogImportMeshObject()
{}


void DialogImportMeshObject::informData(IGuiData *data)
{
    if (data->getType() == guiDType::projectPath)
        onProjectPath(data);
    if (data->getType() == guiDType::conversionOptionsDisplay)
        setTruncCoor(static_cast<GuiDataConversionOptionsDisplay*>(data)->m_translation);
    if (data->getType() == guiDType::projectDataPropertiesNoOpen)
        setTruncCoor(static_cast<GuiDataProjectProperties*>(data)->m_projectInfo.m_importScanTranslation);

}

void DialogImportMeshObject::closeEvent(QCloseEvent* close)
{
    onCancel();
}

void DialogImportMeshObject::onCancel()
{
    m_dataDispatcher.updateInformation(new GuiDataActivatedFunctions(ContextType::none));
    hide();
}

void DialogImportMeshObject::onComboBoxChanged(int comboBox, int index)
{
    QComboBox* changed = comboBox == 0 ? m_ui.upDirectionComboBox : m_ui.forwardDirectionComboBox;
    QComboBox* other = comboBox == 1 ? m_ui.upDirectionComboBox : m_ui.forwardDirectionComboBox;

    Selection selPreChanged = comboBox == 0 ? m_up : m_forward;
    int indPreChanged = 0;
    for (auto dir : DirectionList)
    {
        if ((Selection)dir.first.toInt() == selPreChanged)
            break;
        indPreChanged++;
    }

    Selection selChanged = (Selection)changed->currentData().toInt();
    Selection selOther = (Selection)other->currentData().toInt();

    std::set<Selection> incomp = getIncompatibleAxes(selChanged);
    if (incomp.find(selOther) != incomp.end())
        other->setCurrentIndex(indPreChanged);

    m_up = (Selection)m_ui.upDirectionComboBox->currentData().toInt();
    m_forward = (Selection)m_ui.forwardDirectionComboBox->currentData().toInt();
}

void DialogImportMeshObject::onOverrideScaleUpdate()
{
    if (m_ui.overrideScaleCheckBox->isChecked() && m_storedExtension != FileType::MAX_ENUM || m_storedExtension == FileType::OBJ) 
        m_ui.scaleFrame->show();
    else 
        m_ui.scaleFrame->hide();

    adjustSize();
}

void DialogImportMeshObject::onTruncateCoorOnImport()
{
    if (m_ui.truncateCoordinatesCheckBox->isChecked())
    {
        m_ui.coordinatesGroup->setEnabled(false);
        m_ui.xPosLineEdit->setValue(m_storedScanTruncateCoor.x);
        m_ui.yPosLineEdit->setValue(m_storedScanTruncateCoor.y);
        m_ui.zPosLineEdit->setValue(m_storedScanTruncateCoor.z);
        m_ui.coordinatesGroup->show();
    }
    else
        m_ui.coordinatesGroup->hide();

    adjustSize();
}

void DialogImportMeshObject::onOk()
{
    if (m_ui.scaleFrame->isVisible() && m_ui.scaleLineEdit->getValue() <= 0)
    {
        QMessageBox modal(QMessageBox::Icon::Information, TEXT_DIALOG_TITLE_IMPORT_WAVEFRONT, TEXT_DIALOG_ERROR_SCALING, QMessageBox::StandardButton::Ok);
        modal.exec();
        return;
    }

    Selection up((Selection)m_ui.upDirectionComboBox->currentData().toInt()), fw((Selection)m_ui.forwardDirectionComboBox->currentData().toInt());
    if (!checkDirs(up, fw))
    {
        QMessageBox modal(QMessageBox::Icon::Information, TEXT_DIALOG_TITLE_IMPORT_WAVEFRONT, TEXT_DIALOG_ERROR_DIRECTION, QMessageBox::StandardButton::Ok);
        modal.exec();
        return;
    }
    if(m_ui.fileLineEdit->text().isEmpty() || m_storedExtension == FileType::MAX_ENUM)
    {
        QMessageBox modal(QMessageBox::Icon::Information, TEXT_DIALOG_ERROR_FILENAME, TEXT_DIALOG_ERROR_FILENAME, QMessageBox::StandardButton::Ok);
        modal.exec();
        return;
    }
	
	PositionOptions posOp = m_ui.keepModelRadioButton->isChecked() ? PositionOptions::KeepModel : (m_ui.placeManuallyRadioButton->isChecked() ? PositionOptions::ClickPosition : PositionOptions::GivenCoordinates);
	
	FileInputData data;
	data.file = m_storedPath;
	data.extension = m_storedExtension;

	data.scale = m_ui.scaleFrame->isHidden() ? -1. : m_ui.scaleLineEdit->getValue();
    data.lod = m_ui.LODSlider->value();
	data.isMerge = m_ui.mergeCheckBox->isChecked();
	data.up = up;
	data.forward = fw;
	data.posOption = posOp;
	data.position = m_storedPosition;
    data.truncateCoordinatesAsTheScans = m_ui.truncateCoordinatesCheckBox->isChecked();
   
	if (data.extension == FileType::STEP && m_ui.toSimplificateCheckBox->isChecked())
    {
		m_dialogStepSimplification.setImportInputData(data);
		m_dialogStepSimplification.show();
	}
	else
		m_dataDispatcher.sendControl(new control::meshObject::CreateMeshObjectFromFile(data));
		
    hide();
}

void DialogImportMeshObject::onPositionEdit()
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

void DialogImportMeshObject::resetCoordinates()
{
    m_ui.coordinatesGroup->setEnabled(true);
    m_ui.xPosLineEdit->setValue(0.);
    m_ui.yPosLineEdit->setValue(0.);
    m_ui.zPosLineEdit->setValue(0.);
}

void DialogImportMeshObject::onFileBrowser()
{
    
    QString fileTypes = TEXT_FILE_TYPE_ALL_OBJECTS;
    m_ui.fileLineEdit->setText(QFileDialog::getOpenFileName(this, TEXT_DIALOG_BROWSER_TITLE_IMPORT_WAVEFRONT, m_openPath, fileTypes, nullptr));

    m_storedPath = m_ui.fileLineEdit->text().toStdWString();
    m_storedExtension = getFileType(m_storedPath.extension());

    if(!m_storedPath.empty())
        m_openPath = QString::fromStdWString(m_storedPath.parent_path().wstring());

    updateFileType();

}

void DialogImportMeshObject::onProjectPath(IGuiData* data)
{
    GuiDataProjectPath* dataType = static_cast<GuiDataProjectPath*>(data);
    m_openPath = QString::fromStdWString(dataType->m_path.wstring());
}

void DialogImportMeshObject::setTruncCoor(const glm::dvec3& truncCoor)
{
    m_storedScanTruncateCoor = truncCoor;
}

void DialogImportMeshObject::initComboBoxes()
{
    m_ui.upDirectionComboBox->blockSignals(true);
    m_ui.forwardDirectionComboBox->blockSignals(true);
    m_ui.upDirectionComboBox->clear();
    m_ui.forwardDirectionComboBox->clear();
    int ind = 0;
    for (const std::pair<QVariant, QString>& iterator : DirectionList)
    {
        m_ui.upDirectionComboBox->addItem(iterator.second, iterator.first);
        m_ui.forwardDirectionComboBox->addItem(iterator.second, iterator.first);
        if ((Selection)iterator.first.toInt() == m_forward)
            m_ui.forwardDirectionComboBox->setCurrentIndex(ind);
        if ((Selection)iterator.first.toInt() == m_up)
            m_ui.upDirectionComboBox->setCurrentIndex(ind);
        ind++;
    }
    m_ui.upDirectionComboBox->blockSignals(false);
    m_ui.forwardDirectionComboBox->blockSignals(false);
}

bool DialogImportMeshObject::checkDirs(const Selection& up, const Selection& fw) const
{
    std::set<Selection> incomp = getIncompatibleAxes(up);
    return incomp.find(fw) == incomp.end();
}

std::set<Selection> DialogImportMeshObject::getIncompatibleAxes(const Selection& dir) const
{
    std::set<Selection> incompatibleAxes;
    incompatibleAxes.insert(dir);
    switch (dir)
    {
    case Selection::X:
        incompatibleAxes.insert(Selection::_X);
        break;
    case Selection::_X:
        incompatibleAxes.insert(Selection::X);
        break;
    case Selection::Y:
        incompatibleAxes.insert(Selection::_Y);
        break;
    case Selection::_Y:
        incompatibleAxes.insert(Selection::Y);
        break;
    case Selection::Z:
        incompatibleAxes.insert(Selection::_Z);
        break;
    case Selection::_Z:
        incompatibleAxes.insert(Selection::Z);
        break;
    }

    return incompatibleAxes;
}

void DialogImportMeshObject::updateFileType()
{
    switch (m_storedExtension)
    {
        case FileType::STEP:
        {
            m_ui.LODBox->show();
            m_ui.toSimplificateCheckBox->show();
            m_ui.overrideScaleCheckBox->setEnabled(true);
        }
        break;
        case FileType::IFC:
        {
            m_ui.LODBox->show();
            m_ui.toSimplificateCheckBox->hide();
            m_ui.overrideScaleCheckBox->setEnabled(true);
        }
        break;
        case FileType::FBX:
        {
            m_ui.LODBox->hide();
            m_ui.toSimplificateCheckBox->hide();
            m_ui.overrideScaleCheckBox->setEnabled(true);
        }
        break;
        case FileType::OBJ:
        {
            m_ui.LODBox->hide();
            m_ui.toSimplificateCheckBox->hide();
            m_ui.overrideScaleCheckBox->setEnabled(false);
        }
        break;
        case FileType::DXF:
        {
            m_ui.LODBox->hide();
            m_ui.toSimplificateCheckBox->hide();
            m_ui.overrideScaleCheckBox->setEnabled(true);
        }
        break;
        default:
        {
            m_ui.LODBox->hide();
            m_ui.toSimplificateCheckBox->hide();
            m_ui.overrideScaleCheckBox->setEnabled(false);
            /*QMessageBox modal(QMessageBox::Icon::Warning, TEXT_DIALOG_TITLE_IMPORT_WAVEFRONT, TEXT_DIALOG_ERROR_FORMAT, QMessageBox::StandardButton::Ok);
            modal.exec();*/
        }
        break;
    }
    onOverrideScaleUpdate();
    adjustSize();
}
