#ifndef ALIST_NAME_DIALOG_H_
#define ALIST_NAME_DIALOG_H_

#include <QtWidgets/qwidget.h>
#include "gui/Dialog/ADialog.h"
#include "ui_ListNameDialog.h"

class AListNameDialog : public ADialog
{
	Q_OBJECT

public:
	explicit AListNameDialog(IDataDispatcher &dataDispacher, QWidget *parent = 0);
	~AListNameDialog();

	// from IPanel
	void informData(IGuiData *keyValue) override;
	void show(QString name);
	void show();

public slots:

	virtual void acceptCreation() = 0;
	void cancelCreation();

protected:
	Ui::ListNameDialog m_ui;
};

#endif // !ALIST_NAME_DIALOG_H_