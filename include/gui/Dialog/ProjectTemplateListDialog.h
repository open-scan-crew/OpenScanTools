#ifndef PROJECTTEMPLTAES_LIST_DIALOG_H_
#define PROJECTTEMPLTAES_LIST_DIALOG_H_

#include "gui/Dialog/ProjectTemplateNameDialog.h"

#include "gui/Dialog/ADialog.h"
#include "ui_DialogProjectTemplate.h"

#include <qstandarditemmodel.h>

class ProjectTemplateListDialog;

typedef void (ProjectTemplateListDialog::* ProjectTemplateListMethod)(IGuiData*);

class ProjectTemplateListDialog : public ADialog
{
	Q_OBJECT

public:
	explicit ProjectTemplateListDialog(IDataDispatcher &dataDispacher, QWidget *parent = 0);
	~ProjectTemplateListDialog();

	// from IPanel

	void receiveProjectTemplateList(IGuiData *data);
	void onProjectLoaded(IGuiData* data);

	void informData(IGuiData* keyValue);

public slots:
	void addNewTemplate();
	void deleteTemplate();
	//void selectTemplate();
	//void duplicateTemplate();
	void renameTemplate();
	//void importNewTemplate();
	void updateTemplate();
	void clickOnItem(const QModelIndex& id);
	void finishDialog();
	void onNameRecieved(const std::wstring& name);


private:
	enum class WaitForName{Nope, Creation, Import, Duplication, Renaming};

private:
	void checkSelectedBeforeName(const WaitForName& wait);

private:
	ProjectTemplateNameDialog	m_nameDial;
	std::unordered_map<guiDType, ProjectTemplateListMethod> m_methods;
	QModelIndex m_idSaved;
	WaitForName m_waitFor = WaitForName::Nope;

	bool m_isProjectLoad;
	std::wstring m_projectName;

	Ui::ProjectTemplate m_ui;

	QStandardItemModel* m_model = nullptr;
	QMenu* m_contextualMenu = nullptr;
};

#endif // !PROJECTTEMPLTAES_LIST_DIALOG_H_