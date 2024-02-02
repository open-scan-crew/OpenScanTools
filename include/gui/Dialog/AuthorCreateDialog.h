#ifndef CREATE_AUTHOR_DIALOG_H_
#define CREATE_AUTHOR_DIALOG_H_

#include <QtWidgets/QWidget>
#include "gui/Dialog/ADialog.h"
#include "ui_AuthorCreateDialog.h"

class AuthorCreateDialog : public ADialog
{
	Q_OBJECT

public:
	explicit AuthorCreateDialog(IDataDispatcher &dataDispacher, QWidget *parent = 0);
	~AuthorCreateDialog();

	// from IPanel
	void informData(IGuiData *keyValue) override;

public slots:

	void acceptCreation();
	void cancelCreation();
private:

	Ui::AuthorCreateDialog *ui;
};

#endif // !CREATE_AUTHOR_DIALOG_H_