#include "gui/widgets/PropertiesProjectPanel.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/widgets/FocusWatcher.h"


#include "controller/controls/ControlProject.h"
#include "controller/controls/ControlIO.h"

#include <fmt/format.h>

#include <QtWidgets/qfiledialog.h>

#include "gui/Texts.hpp"

PropertiesProjectPanel::PropertiesProjectPanel(IDataDispatcher& dataDispatcher, QWidget* parent)
	: APropertyGeneral(dataDispatcher, parent)
{
	m_ui.setupUi(this);

	m_ui.lineEditCustomScanFolder->setEnabled(false);

	// Link action
	QObject::connect(m_ui.CompanyInfield, &QLineEdit::editingFinished, this, &PropertiesProjectPanel::slotEditCompany);
	QObject::connect(m_ui.LocationInfield, &QLineEdit::editingFinished, this, &PropertiesProjectPanel::slotEditLocation);
	QObject::connect(new FocusWatcher(m_ui.DescInfield), &FocusWatcher::focusOut, this, &PropertiesProjectPanel::slotEditDescription);

	connect(m_ui.pushButtonCustomScanFolder, &QPushButton::released, this, &PropertiesProjectPanel::launchFileBrowserCustomScanFolder);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectDataProperties);

	m_ui.xTruncValue->setType(NumericType::DISTANCE);
	m_ui.yTruncValue->setType(NumericType::DISTANCE);
	m_ui.zTruncValue->setType(NumericType::DISTANCE);
}

PropertiesProjectPanel::~PropertiesProjectPanel()
{
	m_dataDispatcher.unregisterObserver(this);
}

void PropertiesProjectPanel::hideEvent(QHideEvent* event)
{
	m_ui.ProjectNameInfield->blockSignals(true);
	m_ui.AuthorInfield->blockSignals(true);
	m_ui.CompanyInfield->blockSignals(true);
	m_ui.LocationInfield->blockSignals(true);
	m_ui.DescInfield->blockSignals(true);
	//m_ui.ProjectIdInfield->blockSignals(true);

	QWidget::hideEvent(event);

	m_ui.ProjectNameInfield->blockSignals(false);
	m_ui.AuthorInfield->blockSignals(false);
	m_ui.CompanyInfield->blockSignals(false);
	m_ui.LocationInfield->blockSignals(false);
	m_ui.DescInfield->blockSignals(false);
	//m_ui.ProjectIdInfield->blockSignals(false);
}

void PropertiesProjectPanel::onProjectProperties(IGuiData * data)
{
	m_ui.ProjectNameInfield->blockSignals(true);
	m_ui.AuthorInfield->blockSignals(true);
	m_ui.CompanyInfield->blockSignals(true);
	m_ui.LocationInfield->blockSignals(true);
	m_ui.DescInfield->blockSignals(true);
	//m_ui.ProjectIdInfield->blockSignals(true);

	GuiDataProjectProperties *PPdata = static_cast<GuiDataProjectProperties*>(data);

	m_ui.ProjectNameInfield->setText(QString::fromStdWString(PPdata->m_projectInfo.m_projectName));

	std::wstring authName = L"NO_AUTHOR";
	ReadPtr<Author> rAuth = PPdata->m_projectInfo.m_author.cget();
	if (rAuth)
		authName = rAuth->getName();

	m_ui.AuthorInfield->setText(QString::fromStdWString(authName));
	m_ui.CompanyInfield->setText(QString::fromStdWString(PPdata->m_projectInfo.m_company));
	m_ui.LocationInfield->setText(QString::fromStdWString(PPdata->m_projectInfo.m_location));
	//m_ui.ProjectIdInfield->setText(QString::fromStdString(PPdata->projectInfo.m_id));
	if (m_ui.DescInfield->toPlainText().toStdWString() != PPdata->m_projectInfo.m_description)
		m_ui.DescInfield->setPlainText(QString::fromStdWString(PPdata->m_projectInfo.m_description));

	m_ui.xTruncValue->setValue(PPdata->m_projectInfo.m_importScanTranslation.x);
	m_ui.yTruncValue->setValue(PPdata->m_projectInfo.m_importScanTranslation.y);
	m_ui.zTruncValue->setValue(PPdata->m_projectInfo.m_importScanTranslation.z);

	m_ui.PointsCountField->setText(QString::fromStdString(fmt::format(std::locale("en_US.UTF-8"), "{:L}", PPdata->m_projectPointsCount)));
	m_ui.ScansCountField->setText(QString("%1").arg(PPdata->m_projectScansCount));
	m_ui.ScansActiveCountField->setText(QString("%1").arg(PPdata->m_projectActiveScansCount));

	m_ui.lineEditCustomScanFolder->setText(QString::fromStdWString(PPdata->m_projectInfo.m_customScanFolderPath.wstring()));
	m_openPath = QString::fromStdWString(PPdata->m_projectInfo.m_customScanFolderPath.wstring());

	storedProjectInfo = PPdata->m_projectInfo;


	m_ui.ProjectNameInfield->blockSignals(false);
	m_ui.AuthorInfield->blockSignals(false);
	m_ui.CompanyInfield->blockSignals(false);
	m_ui.LocationInfield->blockSignals(false);
	m_ui.DescInfield->blockSignals(false);
	//m_ui.ProjectIdInfield->blockSignals(false);
}

void PropertiesProjectPanel::slotEditCompany()
{
	if (m_ui.CompanyInfield->text().toStdWString() != storedProjectInfo.m_company)
	{
		storedProjectInfo.m_company = m_ui.CompanyInfield->text().toStdWString();
		sendAEditProjectControl();
	}
}

void PropertiesProjectPanel::slotEditLocation()
{
	if (m_ui.LocationInfield->text().toStdWString() != storedProjectInfo.m_location)
	{
		storedProjectInfo.m_location = m_ui.LocationInfield->text().toStdWString();
		sendAEditProjectControl();
	}
}

void PropertiesProjectPanel::slotEditDescription()
{
	if (m_ui.DescInfield->toPlainText().toStdWString() != storedProjectInfo.m_description)
	{
		storedProjectInfo.m_description = m_ui.DescInfield->toPlainText().toStdWString();
		sendAEditProjectControl();
	}
}

void PropertiesProjectPanel::sendAEditProjectControl()
{
	m_dataDispatcher.sendControl(new control::project::Edit(storedProjectInfo));
}

void PropertiesProjectPanel::informData(IGuiData *data)
{
	if (data->getType() == guiDType::projectDataProperties)
		onProjectProperties(data);
}

bool PropertiesProjectPanel::actualizeProperty(SafePtr<AGraphNode> object)
{
	return false;
}

void PropertiesProjectPanel::launchFileBrowserCustomScanFolder()
{
	QFileDialog dialog;
	dialog.setModal(true);

	QString qFilepath = dialog.getExistingDirectory(this, TEXT_SELECT_CUSTOM_SCAN_FOLDER,
		m_openPath, QFileDialog::ShowDirsOnly);

	if (qFilepath.isEmpty())
		return;

	m_openPath = qFilepath;
	storedProjectInfo.m_customScanFolderPath = qFilepath.toStdWString();
	sendAEditProjectControl();
	m_dataDispatcher.sendControl(new control::io::RefreshScanLink());
}