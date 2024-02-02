#ifndef PIPE_STANDARD_LIST_DIALOG_H_
#define PIPE_STANDARD_LIST_DIALOG_H_

#include "gui/Dialog/AListListDialog.h"
#include "gui/Dialog/StandardModifierDialog.h"
#include "gui/Dialog/StandardNameDialog.h"

enum class StandardType; 

class StandardListDialog : public AListListDialog
{
	Q_OBJECT
public:
	explicit StandardListDialog(IDataDispatcher &dataDispacher, const StandardType& type, QWidget *parent, const bool& deleteOnClose);
	~StandardListDialog();

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
	const StandardType	m_type;
	StandardModifierDialog m_modifDial;
	StandardNameDialog m_nameDial;

};

#endif // !PIPE_STANDARD_LIST_DIALOG_H_