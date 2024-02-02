#ifndef ALIST_MODIFIER_DIALOG_H_
#define ALIST_MODIFIER_DIALOG_H_

#include <QtWidgets/QDialog>
#include <QtWidgets/QListView>
#include <QtGui/QStandardItemModel>
#include "gui/Dialog/ADialog.h"
#include "ui_ListModifierDialog.h"

class AListModifierDialog;

typedef void (AListModifierDialog::* ListModifierMethod)(IGuiData*);

class AListModifierDialog : public ADialog
{
	Q_OBJECT

public:
	AListModifierDialog(IDataDispatcher &dataDispacher, QDialog *parent = 0);
	~AListModifierDialog();

	// from IPanel
	void informData(IGuiData *keyValue) override;

	void show();
	virtual void getList(IGuiData *data) = 0;

public slots:
	virtual void showElemMenu(QPoint p);

	virtual void addNewElem() = 0;
	virtual void renameElem() = 0;
	virtual void deleteElem() = 0;
	virtual void renameListViewElem(QStandardItem *item) = 0;
	virtual void clearList() = 0;

	void closeDialog();


protected:
	std::unordered_map<guiDType, ListModifierMethod> m_methods;
	Ui::ListModifierDialog m_ui;

	QStandardItemModel *model = nullptr;
};

#endif // !ALIST_MODIFIER_DIALOG_H_