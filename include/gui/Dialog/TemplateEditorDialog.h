#ifndef TEMPLATE_EDITOR_DIALOG_H_
#define TEMPLATE_EDITOR_DIALOG_H_

#include "gui/Dialog/ADialog.h"
#include "gui/IDataDispatcher.h"
#include "ui_TemplateEditorDialog.h"
#include "models/application/TagTemplate.h"
#include "models/application/List.h"

class TemplateEditorDialog;

typedef QWidget *(TemplateEditorDialog::*defaultValueWidgetGenerator)(sma::tField& field, int line);
typedef std::wstring(TemplateEditorDialog::*defaultValueGatherer)(int);

class TemplateEditorDialog : public ADialog
{
	Q_OBJECT

public:
	explicit TemplateEditorDialog(IDataDispatcher &dataDispacher, QWidget *parent = 0);
	~TemplateEditorDialog();

	// from IPanel
	void informData(IGuiData *keyValue) override;

	void receiveTagTemplate(IGuiData *data);

	void createCreationField();

	QWidget *generateStringWidget(sma::tField& field, int line);
	QWidget *generateMultiLineWidget(sma::tField& field, int line);
	QWidget *generateDateWidget(sma::tField& field, int line);
	QWidget *generateNumberWidget(sma::tField& field, int line);
	QWidget *generateHyperlinkWidget(sma::tField& field, int line);
	QWidget *generateListWidget(sma::tField& field, int line);
	QWidget *generateNoneWidget(sma::tField& field, int line);

	std::wstring gatherStringWidget(int line);
	std::wstring gatherMultiLineWidget(int line);
	std::wstring gatherDateWidget(int line);
	std::wstring gatherNumberWidget(int line);
	std::wstring gatherHyperlinkWidget(int line);
	std::wstring gatherListWidget(int line);
	std::wstring gatherNoneWidget(int line);

public slots:

	void renameTemplate();
	void CancelDialog();
	void FinishDialog();
	void TryToAddNewLine();

	void controlNumberDefaultField(int line);

	void verifNameModif(int line);
	void modifyName(int line);
	void modifyType(int line);
	void modifyRef(int line);
	void modifyDefaultValue(int line);
	void deleteField(int line);

	void setFilter(const int& header);
	
private:
	int					m_sortedColumn;
	Qt::SortOrder		m_sortOrder;
	SafePtr<sma::TagTemplate>	m_template;

	Ui::TemplateEditDialog *ui;

	std::vector<sma::tFieldType> m_typesVec;
	std::vector<SafePtr<UserList>> m_listLink;
	std::unordered_map<sma::tFieldType, std::pair<defaultValueWidgetGenerator, defaultValueGatherer>> m_defaultValueManagers;

	const std::map<sma::tFieldType, QString> m_fieldTypeToString;
};

#endif // !TEMPLATE_EDITOR_DIALOG_H_