#include "gui/Dialog/ABlockingDialog.h"
#include <qevent.h>

ABlockingDialog::ABlockingDialog(IDataDispatcher& dataDispacher, QWidget* parent)
	: ADialog(dataDispacher, parent)
{}

ABlockingDialog::~ABlockingDialog()
{}

void ABlockingDialog::keyPressEvent(QKeyEvent* e)
{
	if (e->key() == Qt::Key_Escape)
		e->ignore();
	else
		QDialog::keyPressEvent(e);
}

void ABlockingDialog::closeEvent(QCloseEvent* e)
{
	e->setAccepted(!e->spontaneous());
}