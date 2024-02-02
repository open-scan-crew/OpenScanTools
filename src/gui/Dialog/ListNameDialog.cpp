#include "gui/Dialog/ListNameDialog.h"
#include "controller/controls/ControlUserList.h"

ListNameDialog::ListNameDialog(IDataDispatcher& dataDispatcher, QWidget *parent)
	: AListNameDialog(dataDispatcher, parent)	
{}

ListNameDialog::~ListNameDialog()
{}

void ListNameDialog::acceptCreation()
{
	m_ui.ListNameInfield->blockSignals(true);
	if (m_ui.ListNameInfield->text() != "")
	{
		m_dataDispatcher.sendControl(new control::userlist::CreateUserList(m_ui.ListNameInfield->text().toStdWString()));
		hide();
	}
	m_ui.ListNameInfield->blockSignals(false);
}