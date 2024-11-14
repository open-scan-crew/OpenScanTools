#ifndef TEMPLATE_MANAGER_DIALOG_H_
#define TEMPLATE_MANAGER_DIALOG_H_

#include "gui/Dialog/ADialog.h"
#include "ui_TemplateManagerDialog.h"
#include "gui/Dialog/TagTemplateNameDialog.h"
#include "gui/Dialog/TemplateEditorDialog.h"

#include <QtGui/qstandarditemmodel.h>

class TemplateManagerDialog : public ADialog
{
	Q_OBJECT

public:
	explicit TemplateManagerDialog(IDataDispatcher &dataDispacher, QWidget *parent, const bool& deleteOnClose);
	~TemplateManagerDialog();

	// from IPanel
	void informData(IGuiData *keyValue) override;

	void receiveTemplateList(IGuiData *data);

public slots:

	void showTreeMenu(QPoint p);

	void deleteTemplate();
	void newTemplate();
	void duplicateTemplate();
	void editSelectedTemplate();
	void editTemplate(const QModelIndex& index);
	void exportTemplate();
	void importTemplate();
	void clickOnItem(const QModelIndex& id);

	void FinishDialog();
private:
	QModelIndex m_idSaved;

	Ui::TemplateManager *m_ui;

	QStandardItemModel *m_model = nullptr;
	TagTemplateNameDialog m_templateNameDialog;
	TemplateEditorDialog  m_templateEditorDialog;
};

#endif // !TEMPLATE_MANAGER_DIALOG_H_