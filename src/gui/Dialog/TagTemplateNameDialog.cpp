#include "gui/Dialog/TagTemplateNameDialog.h"
#include "controller/controls/ControlTemplateEdit.h"

TagTemplateNameDialog::TagTemplateNameDialog(IDataDispatcher& dataDispatcher, QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::TagTemplateDialog)
	, m_dataDispatcher(dataDispatcher)
{
	ui->setupUi(this);

	QObject::connect(ui->CancelBtn, SIGNAL(clicked()), this, SLOT(cancelCreation()));
	QObject::connect(ui->okBtn, SIGNAL(clicked()), this, SLOT(acceptCreation()));
	PANELLOG << "create TagTemplateNameDialog" << LOGENDL;
}

TagTemplateNameDialog::~TagTemplateNameDialog()
{
	PANELLOG << "destroy TagTemplateNameDialog" << LOGENDL;
	m_dataDispatcher.unregisterObserver(this);
}

void TagTemplateNameDialog::informData(IGuiData *data)
{}

void TagTemplateNameDialog::acceptCreation()
{
	ui->TemplateNameInfield->blockSignals(true);
	if (ui->TemplateNameInfield->text() != "")
	{
		m_dataDispatcher.sendControl(new control::tagTemplate::CreateTagTemplate(ui->TemplateNameInfield->text().toStdWString()));
		this->hide();
	}
	ui->TemplateNameInfield->blockSignals(false);
}

void TagTemplateNameDialog::cancelCreation()
{
	this->hide();
	//delete(this);
}