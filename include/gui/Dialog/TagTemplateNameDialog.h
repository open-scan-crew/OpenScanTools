#ifndef LIST_NAME_DIALOG___H_
#define LIST_NAME_DIALOG___H_

#include <QtWidgets/QWidget>
#include "gui/IPanel.h"
#include "gui/IDataDispatcher.h"
#include "ui_TagTemplateNameDialog.h"

class TagTemplateNameDialog : public QDialog, public IPanel
{
	Q_OBJECT

public:
	explicit TagTemplateNameDialog(IDataDispatcher &dataDispacher, QWidget *parent = 0);
	~TagTemplateNameDialog();

	// from IPanel
	void informData(IGuiData *keyValue) override;


public slots:

	void acceptCreation();
	void cancelCreation();
private:

	Ui::TagTemplateDialog *ui;
	IDataDispatcher& m_dataDispatcher;
};

#endif // !LIST_NAME_DIALOG_H_