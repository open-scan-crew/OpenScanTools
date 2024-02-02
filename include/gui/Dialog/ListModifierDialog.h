#ifndef LIST_MODIFIER_DIALOG_H_
#define LIST_MODIFIER_DIALOG_H_

#include "gui/Dialog/AListModifierDialog.h"
#include "models/application/List.h"
#include "models/OpenScanToolsModelEssentials.h"

class ListModifierDialog : public AListModifierDialog
{
	Q_OBJECT

public:
	explicit ListModifierDialog(IDataDispatcher &dataDispacher, QDialog *parent = 0);
	~ListModifierDialog();

	void getList(IGuiData *data) override;

public slots:

	void addNewElem();
	void showElemMenu(QPoint p);

	void renameElem();
	void deleteElem();

	void clearList();

	void renameListViewElem(QStandardItem *item);

private:
	SafePtr<UserList> m_list;
};

#endif // !LIST_MODIFIER_DIALOG_H_