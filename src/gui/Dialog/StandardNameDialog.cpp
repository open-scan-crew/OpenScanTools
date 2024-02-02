#include "StandardNameDialog.h"
#include "controller/controls/ControlStandards.h"


StandardNameDialog::StandardNameDialog(IDataDispatcher& dataDispatcher, const StandardType& type, QWidget *parent)
	: AListNameDialog(dataDispatcher, parent)
	, m_type(type)
{
	switch (type)
	{
		case StandardType::Pipe:
			setWindowTitle(QObject::tr("Create pipes standard"));
			break;
		case StandardType::Sphere:
			setWindowTitle(tr("Create spheres standard"));
			break;
	}
}

StandardNameDialog::~StandardNameDialog()
{}

void StandardNameDialog::acceptCreation()
{
	m_ui.ListNameInfield->blockSignals(true);
	if (m_ui.ListNameInfield->text() != "")
	{
		m_dataDispatcher.sendControl(new control::standards::CreateStandard(m_ui.ListNameInfield->text().toStdWString(), m_type));
		hide();
	}
	m_ui.ListNameInfield->blockSignals(false);
}