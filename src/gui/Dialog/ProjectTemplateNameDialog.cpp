#include "gui/Dialog/ProjectTemplateNameDialog.h"
#include "gui/texts/ProjectTemplateTexts.hpp"

ProjectTemplateNameDialog::ProjectTemplateNameDialog(IDataDispatcher& dataDispatcher, QWidget *parent)
	: AListNameDialog(dataDispatcher, parent)	
{
	setWindowTitle(TEXT_PROJECT_TEMPLATE_DIALOG_NAME_TITLE);
}

ProjectTemplateNameDialog::~ProjectTemplateNameDialog()
{}

void ProjectTemplateNameDialog::acceptCreation()
{
	m_ui.ListNameInfield->blockSignals(true);
	if (m_ui.ListNameInfield->text() != "")
	{
		emit sendName(m_ui.ListNameInfield->text().toStdWString());
		hide();
	}
	m_ui.ListNameInfield->blockSignals(false);
}