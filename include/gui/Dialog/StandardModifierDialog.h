#ifndef PIPE_STANDARD_MODIFIER_DIALOG_H_
#define PIPE_STANDARD_MODIFIER_DIALOG_H_

#include "gui/Dialog/AListModifierDialog.h"
#include "models/application/List.h"
#include "utils/safe_ptr.h"

class StandardModifierDialog : public AListModifierDialog
{
	Q_OBJECT

public:
	explicit StandardModifierDialog(IDataDispatcher &dataDispacher, const StandardType& type, QDialog *parent = 0);
	~StandardModifierDialog();

	void getList(IGuiData *data) override;

public slots:

	void addNewElem();
	void showElemMenu(QPoint p);

	void renameElem();
	void deleteElem();

	void clearList();

	void renameListViewElem(QStandardItem *item);

private:
	StandardType m_type;
	SafePtr<StandardList> m_list;
};

#endif // !PIPE_STANDARD_MODIFIER_DIALOG_H_