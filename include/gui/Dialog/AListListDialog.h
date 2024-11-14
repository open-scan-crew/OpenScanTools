#ifndef ALIST_LIST_DIALOG_H_
#define ALIST_LIST_DIALOG_H_

#include <QtGui/qstandarditemmodel.h>
#include "gui/Dialog/ADialog.h"
#include "ui_ListListDialog.h"

class AListListDialog;

typedef void (AListListDialog::* ListListMethod)(IGuiData*);

class AListListDialog : public ADialog
{
	Q_OBJECT

public:
	AListListDialog(IDataDispatcher &dataDispacher, QWidget *parent, const bool& deleteOnClose);
	~AListListDialog();

	// from IPanel
	virtual void informData(IGuiData *keyValue) override;

	virtual void receiveListList(IGuiData* data) = 0;
	virtual void onProjectPath(IGuiData* data);

public slots:
	void showTreeMenu(QPoint p);
	virtual void addNewList() = 0;
	virtual void deleteList() = 0;
	virtual void listViewSelect() = 0;
	virtual void importNewList() = 0;
	virtual void exportList() = 0;
	virtual void duplicateList() = 0;
	virtual void clickOnItem(const QModelIndex& id) = 0;


	virtual void FinishDialog();
protected:
	std::unordered_map<guiDType, ListListMethod> m_methods;
	QModelIndex m_idSaved;

	Ui::ListListDialog m_ui;
	QString m_openPath;

	QStandardItemModel *m_model = nullptr;
	QMenu* m_contextualMenu = nullptr;
};

#endif // !LIST_LIST_DIALOG_H_