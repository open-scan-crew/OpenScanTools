#ifndef LIST_LIST_DIALOG_H_
#define LIST_LIST_DIALOG_H_

#include "gui/Dialog/AListListDialog.h"
#include "gui/Dialog/ListModifierDialog.h"
#include "gui/Dialog/ListNameDialog.h"

class ListListDialog : public AListListDialog
{
	Q_OBJECT

public:
	explicit ListListDialog(IDataDispatcher &dataDispacher, QWidget *parent, const bool& deleteOnClose);
	~ListListDialog();

	// from IPanel

	void receiveListList(IGuiData *data) override;
	void saveList();

public slots:

	void addNewList();
	void deleteList();
	void listViewSelect();
	void importNewList();
	void exportList();
	void duplicateList();
	void clickOnItem(const QModelIndex& id);

	void FinishDialog() override;
private:
	ListModifierDialog m_modifDial;
	ListNameDialog	m_nameDial;
};

#endif // !LIST_LIST_DIALOG_H_