#include "gui/Dialog/AListNameDialog.h"
#include "utils/Logger.h"

#include "gui/Texts.hpp"

AListNameDialog::AListNameDialog(IDataDispatcher& dataDispatcher, QWidget *parent)
	: ADialog(dataDispatcher, parent)
{
	m_ui.setupUi(this);
	m_ui.ListNameInfield->blockSignals(true);
	m_ui.ListNameInfield->setText(TEXT_DEFAULT);
	m_ui.ListNameInfield->blockSignals(false);

	QObject::connect(m_ui.CancelBtn, SIGNAL(clicked()), this, SLOT(cancelCreation()));
	QObject::connect(m_ui.okBtn, SIGNAL(clicked()), this, SLOT(acceptCreation()));
	GUI_LOG << "create AListNameDialog" << LOGENDL;
}

AListNameDialog::~AListNameDialog()
{
	GUI_LOG << "destroy AListNameDialog" << LOGENDL;
	m_dataDispatcher.unregisterObserver(this);
}

void AListNameDialog::informData(IGuiData *data)
{}

void AListNameDialog::show(QString text)
{
	m_ui.ListNameInfield->setText(text);
	QDialog::show();
}

void AListNameDialog::show()
{
	m_ui.ListNameInfield->setText(TEXT_DEFAULT);
	QDialog::show();
}

void AListNameDialog::cancelCreation()
{
	hide();
}