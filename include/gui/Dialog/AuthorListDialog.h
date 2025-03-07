#ifndef AUTHOR_LIST_DIALOG_H_
#define AUTHOR_LIST_DIALOG_H_

#include "gui/Dialog/ADialog.h"
#include "gui/IDataDispatcher.h"
#include "ui_AuthorDialog.h"

// This DialogBox MUST be created each time otherwise it doesn't work.
class AuthorListDialog : public ADialog
{
	Q_OBJECT

public:
	explicit AuthorListDialog(IDataDispatcher &dataDispacher, QWidget *parent = 0);
	~AuthorListDialog();

	// from IPanel
	void informData(IGuiData *keyValue) override;

	void receiveAuthorList(IGuiData *data);
	void show();

public slots:

	void addNewAuthor();
	void deleteAuthor();
	bool selectAuthor();
	void closeDialog();

private:
	QModelIndex idSaved;

	Ui::AuthorListDialog m_ui;

	QMenu* m_contextualMenu = nullptr;
};

#endif // !AUTHOR_LIST_DIALOG_H_