#ifndef LIST_NAME_DIALOG_H_
#define LIST_NAME_DIALOG_H_

#include "gui/Dialog/AListNameDialog.h"

class ListNameDialog : public AListNameDialog
{
	Q_OBJECT

public:
	explicit ListNameDialog(IDataDispatcher &dataDispacher, QWidget *parent = 0);
	~ListNameDialog();

public slots:
	void acceptCreation();

signals:
	void sendName(const QString& name);
};

#endif // !LIST_NAME_DIALOG_H_