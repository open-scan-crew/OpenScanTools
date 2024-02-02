#include <QtWidgets/QMenu>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtCore/QStandardPaths>

#include "gui/texts/ProjectTemplateTexts.hpp"

#include "gui/Dialog/ProjectTemplateListDialog.h"
#include "gui/GuiData/GuiDataList.h"
#include "gui/Dialog/ListNameDialog.h"
#include "gui/Dialog/ListModifierDialog.h"
#include "gui/widgets/ListsNode.h"
#include "controller/controls/ControlProjectTemplate.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ListTexts.hpp"

#include "utils/System.h"
#include "utils/Utils.h"

ProjectTemplateListDialog::ProjectTemplateListDialog(IDataDispatcher& dataDispatcher, QWidget *parent)
	: ADialog(dataDispatcher, parent)
	, m_nameDial(dataDispatcher, parent)
	, m_isProjectLoad(false)
{
	m_ui.setupUi(this);
	m_nameDial.hide();

	m_ui.NewListBtn->setEnabled(false);
	m_ui.updateBtn->setEnabled(false);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectTemplateList);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_methods.insert({ guiDType::projectTemplateList, &ProjectTemplateListDialog::receiveProjectTemplateList });
	m_methods.insert({ guiDType::projectLoaded, &ProjectTemplateListDialog::onProjectLoaded });

	QObject::connect(m_ui.NewListBtn, &QPushButton::clicked, this, &ProjectTemplateListDialog::addNewTemplate);
	QObject::connect(m_ui.FinishBtn, &QPushButton::clicked, this, &ProjectTemplateListDialog::finishDialog);
	QObject::connect(m_ui.updateBtn, &QPushButton::clicked, this, &ProjectTemplateListDialog::updateTemplate);
	QObject::connect(m_ui.RemoveBtn, &QPushButton::clicked, this, &ProjectTemplateListDialog::deleteTemplate);
	QObject::connect(m_ui.RenameBtn, &QPushButton::clicked, this, &ProjectTemplateListDialog::renameTemplate);
	QObject::connect(&m_nameDial, &ProjectTemplateNameDialog::sendName, this, &ProjectTemplateListDialog::onNameRecieved);

	// Creation of the contextual menu
	/*m_contextualMenu = new QMenu(this);
	QAction* deleteAct = new QAction(TEXT_ACTION_DELETE_LIST, this);
	m_contextualMenu->addAction(deleteAct);
	QAction* duplicateAct = new QAction(TEXT_ACTION_DUPLICATE_LIST, this);
	m_contextualMenu->addAction(duplicateAct);
	QObject::connect(deleteAct, SIGNAL(triggered()), this, SLOT(deleteTemplate()));
	QObject::connect(duplicateAct, SIGNAL(triggered()), this, SLOT(duplicateTemplate()));*/
}

ProjectTemplateListDialog::~ProjectTemplateListDialog()
{}

void ProjectTemplateListDialog::informData(IGuiData* keyValue)
{
	if (m_methods.find(keyValue->getType()) != m_methods.end())
	{
		ProjectTemplateListMethod method = m_methods.at(keyValue->getType());
		(this->*method)(keyValue);
	}
}

void ProjectTemplateListDialog::receiveProjectTemplateList(IGuiData *data)
{
	GuiDataProjectTemplateList*lData = static_cast<GuiDataProjectTemplateList*>(data);
	uint64_t count(0);

	if(m_model)
		delete m_model;
	m_model = new QStandardItemModel(0, 0);


	for (const std::filesystem::path& name : lData->m_templates)
	{
		QStandardItem* item = new ItemNode(QString::fromStdWString(name.filename().wstring()), name.filename().wstring());
		m_model->setItem(count++, 0, item);
	}

	m_ui.listListView->setModel(m_model);
	m_ui.listListView->setDragDropMode(QAbstractItemView::DragDropMode::NoDragDrop);
	m_ui.listListView->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	m_ui.listListView->setDragEnabled(false);
	m_ui.listListView->setAcceptDrops(false);
	m_ui.listListView->setDropIndicatorShown(false);

	connect(m_ui.listListView->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(clickOnItem(const QModelIndex &)));

	m_ui.RemoveBtn->setEnabled(false);
	m_ui.updateBtn->setEnabled(false);

	PANELLOG << "receive list of project templates of " << lData->m_templates.size() << "elems" << LOGENDL;
}

void ProjectTemplateListDialog::onProjectLoaded(IGuiData* data)
{
	GuiDataProjectLoaded* projectLoad = static_cast<GuiDataProjectLoaded*>(data);
	m_isProjectLoad = projectLoad->m_isProjectLoad;
	m_projectName = projectLoad->m_projectName;

	m_ui.NewListBtn->setEnabled(m_isProjectLoad);
}

void ProjectTemplateListDialog::clickOnItem(const QModelIndex &idx)
{
	m_idSaved = idx;
	ItemNode* item = static_cast<ItemNode*>(m_model->itemFromIndex(m_ui.listListView->selectionModel()->currentIndex()));

	if (item)
	{
		m_ui.RemoveBtn->setEnabled(true);
		m_ui.updateBtn->setEnabled(m_isProjectLoad);
		m_ui.RenameBtn->setEnabled(true);
	}
	else
	{
		m_ui.RemoveBtn->setEnabled(false);
		m_ui.updateBtn->setEnabled(false);
		m_ui.RenameBtn->setEnabled(false);
	}
	PANELLOG << "click on item" << LOGENDL;
}


void ProjectTemplateListDialog::onNameRecieved(const std::wstring& name)
{
	switch (m_waitFor)
	{
	case WaitForName::Creation:
		m_dataDispatcher.sendControl(new control::projectTemplate::CreateTemplate(name, false));
		break;
	case WaitForName::Renaming:
		{
			QModelIndex index = m_idSaved;
			foreach(const QModelIndex & index, m_ui.listListView->selectionModel()->selectedIndexes())
			{
				ItemNode* item = static_cast<ItemNode*>(m_model->itemFromIndex(m_ui.listListView->selectionModel()->currentIndex()));
				m_dataDispatcher.sendControl(new control::projectTemplate::RenameTemplate(item->getWStrData(), name));
			}
		}
		break;
	}
	m_waitFor = WaitForName::Nope;
}

void ProjectTemplateListDialog::addNewTemplate()
{
	PANELLOG << "add new list" << LOGENDL;
	m_waitFor = WaitForName::Creation;
	m_nameDial.show(QString::fromStdWString(m_projectName));
}

void ProjectTemplateListDialog::deleteTemplate()
{
	QMessageBox::StandardButton reply;

	reply = QMessageBox::question(this, TEXT_DELETE_LIST_TITLE, TEXT_DELETE_LIST_QUESTION, QMessageBox::Yes | QMessageBox::No);
	if (reply == QMessageBox::Yes)
	{
		QModelIndexList list = m_ui.listListView->selectionModel()->selectedIndexes();
		foreach(const QModelIndex &index, list)
		{
			ItemNode* item = static_cast<ItemNode*>(m_model->itemFromIndex(index));
			m_dataDispatcher.sendControl(new control::projectTemplate::DeleteTemplate(item->getWStrData()));
		}
		PANELLOG << "delete " << list.size() << " templates." << LOGENDL;
	}
	else
		PANELLOG << "templates not deleted" << LOGENDL;
}

void ProjectTemplateListDialog::updateTemplate()
{
	QModelIndexList list = m_ui.listListView->selectionModel()->selectedIndexes();
	foreach(const QModelIndex & index, list)
	{
		ItemNode* item = static_cast<ItemNode*>(m_model->itemFromIndex(index));
		m_dataDispatcher.sendControl(new control::projectTemplate::CreateTemplate(item->getWStrData(), true));
	}
}

void ProjectTemplateListDialog::renameTemplate()
{
	checkSelectedBeforeName(WaitForName::Renaming);
}

void ProjectTemplateListDialog::checkSelectedBeforeName(const WaitForName& wait)
{
	if (m_ui.listListView->selectionModel()->selectedIndexes().size() != 1)
	{
		m_dataDispatcher.updateInformation(new GuiDataWarning(TEXT_WARNING_SELECT_ONE_ELEMENT));
		return;
	}
	ItemNode* item = static_cast<ItemNode*>(m_model->itemFromIndex(m_ui.listListView->selectionModel()->currentIndex()));

	m_waitFor = wait;
	m_nameDial.show(QString::fromStdWString(item->getWStrData()));
}

void ProjectTemplateListDialog::finishDialog()
{
	hide();
}

