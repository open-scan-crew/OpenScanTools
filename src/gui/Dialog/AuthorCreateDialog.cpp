#include "gui/Dialog/AuthorCreateDialog.h"
#include "controller/controls/ControlAuthor.h"
#include "utils/Logger.h"

AuthorCreateDialog::AuthorCreateDialog(IDataDispatcher& dataDispatcher, QWidget *parent)
	: ADialog(dataDispatcher, parent)
	, ui(new Ui::AuthorCreateDialog)
{
	ui->setupUi(this);

	ui->OkBtn->setDefault(true);

	QObject::connect(ui->CancelBtn, SIGNAL(clicked()), this, SLOT(cancelCreation()));
	QObject::connect(ui->OkBtn, SIGNAL(clicked()), this, SLOT(acceptCreation()));
	GUI_LOG << "create AuthorCreateDialog" << LOGENDL;
}

AuthorCreateDialog::~AuthorCreateDialog()
{
	GUI_LOG << "destroy AuthorCreateDialog" << LOGENDL;
	m_dataDispatcher.unregisterObserver(this);
}

void AuthorCreateDialog::informData(IGuiData *data)
{}

void AuthorCreateDialog::acceptCreation()
{
	ui->AuthorInfield->blockSignals(true);
	if (ui->AuthorInfield->text() != "")
	{
		m_dataDispatcher.sendControl(new control::author::CreateNewAuthor(ui->AuthorInfield->text().toStdWString()));
		this->close();
		delete this;
		return;
	}
	ui->AuthorInfield->blockSignals(false);
}

void AuthorCreateDialog::cancelCreation()
{
	this->close();
	delete this;
}