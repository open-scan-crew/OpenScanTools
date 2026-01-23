#include "gui/Dialog/DialogSubProjectInfo.h"
#include "models/application/Author.h"
#include "models/project/ProjectInfos.h"

DialogSubProjectInfo::DialogSubProjectInfo(IDataDispatcher& dataDispatcher, QWidget* parent)
	: QDialog(parent)
{
	m_ui.setupUi(this);

	// ----- Important | Window Flags -----
	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::MSWindowsFixedSizeDialogHint;
	flags ^= Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);
	// ------------------------------------

	connect(m_ui.okButton, &QPushButton::clicked, this, &DialogSubProjectInfo::onOkButton);

}

DialogSubProjectInfo::~DialogSubProjectInfo()
{}

void DialogSubProjectInfo::setProjectInfos(const ProjectInfos& info)
{
	m_ui.ProjectNameInfield->setText(QString::fromStdWString(info.m_projectName));

	std::wstring authName = L"NO_AUTHOR";
	{
		ReadPtr<Author> rAuth = info.m_author.cget();
		if (rAuth)
			authName = rAuth->getName();
	}

	m_ui.AuthorInfield->setText(QString::fromStdWString(authName));
	m_ui.CompanyInfield->setText(QString::fromStdWString(info.m_company));
	m_ui.LocationInfield->setText(QString::fromStdWString(info.m_location));
	m_ui.DescInfield->setPlainText(QString::fromStdWString(info.m_description));

	storedInfo = info;

}

ProjectInfos DialogSubProjectInfo::getProjectInfos()
{
	ProjectInfos info = storedInfo;

	info.m_projectName = m_ui.ProjectNameInfield->text().toStdWString();
	info.m_company = m_ui.CompanyInfield->text().toStdWString();
	info.m_location = m_ui.LocationInfield->text().toStdWString();
	info.m_description = m_ui.DescInfield->toPlainText().toStdWString();

	return info;
}

void DialogSubProjectInfo::onOkButton()
{
	hide();
	QDialog::accept();
}

/*
void PropertiesProjectPanel::onProjectProperties(IGuiData * data)
{


	m_ui.ProjectNameInfield->setText(QString::fromStdWString(PPdata->projectInfo.m_projectName));
	m_ui.AuthorInfield->setText(QString::fromStdString(PPdata->projectInfo.m_author.getName()));
	m_ui.CompanyInfield->setText(QString::fromStdString(PPdata->projectInfo.m_company));
	m_ui.LocationInfield->setText(QString::fromStdString(PPdata->projectInfo.m_location));
	//m_ui.ProjectIdInfield->setText(QString::fromStdString(PPdata->projectInfo.m_id));
	if (m_ui.DescInfield->toPlainText().toStdString() != PPdata->projectInfo.m_description)
		m_ui.DescInfield->setPlainText(QString::fromStdString(PPdata->projectInfo.m_description));

	m_ui.xTruncValue->setText(QString::fromStdString(Utils::roundFloat(PPdata->projectInfo.m_importScanTranslation.x, 0)));
	m_ui.yTruncValue->setText(QString::fromStdString(Utils::roundFloat(PPdata->projectInfo.m_importScanTranslation.y, 0)));
	m_ui.zTruncValue->setText(QString::fromStdString(Utils::roundFloat(PPdata->projectInfo.m_importScanTranslation.z, 0)));

	uint64_t pointsCount = PPdata->projectPointsCount;
	QString strPoint("");
	while (pointsCount >= 1000)
	{
		std::string num = Utils::completeWithZeros(pointsCount % 1000);
		strPoint.push_front(QString(".%1").arg(num.c_str()));
		pointsCount = (pointsCount / 1000);
	}
	strPoint.push_front(QString("%1").arg(pointsCount));

	m_ui.PointsCountField->setText(strPoint);
	m_ui.ScansCountField->setText(QString("%1").arg(PPdata->projectScansCount));
	m_ui.ScansActiveCountField->setText(QString("%1").arg(PPdata->projectActiveScansCount));


	storedProjectName = QString::fromStdWString(PPdata->projectInfo.m_projectName);
	storedAuthor = PPdata->projectInfo.m_author;
	storedCompany = QString::fromStdString(PPdata->projectInfo.m_company);
	storedLocation = QString::fromStdString(PPdata->projectInfo.m_location);
	storedDesc = QString::fromStdString(PPdata->projectInfo.m_description);


}*/