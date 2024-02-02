#ifndef PROJECT_TEMPLATE_NAME_DIALOG_H_
#define PROJECT_TEMPLATE_NAME_DIALOG_H_

#include "gui/Dialog/AListNameDialog.h"

class ProjectTemplateNameDialog : public AListNameDialog
{
	Q_OBJECT

public:
	explicit ProjectTemplateNameDialog(IDataDispatcher &dataDispacher, QWidget *parent = 0);
	~ProjectTemplateNameDialog();

public slots:
	void acceptCreation();

signals:
	void sendName(const std::wstring& name);
};

#endif // !PROJECT_TEMPLATE_NAME_DIALOG_H_