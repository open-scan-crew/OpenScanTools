#include <QtWidgets/QMenu>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtCore/QStandardPaths>

#include "gui/Dialog/ListListDialog.h"
#include "gui/GuiData/GuiDataList.h"
#include "gui/Dialog/ListNameDialog.h"
#include "gui/Dialog/ListModifierDialog.h"
#include "gui/widgets/ListsNode.h"
#include "controller/controls/ControlUserList.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ListTexts.hpp"
#include "utils/Logger.h"

ListListDialog::ListListDialog(IDataDispatcher& dataDispatcher, QWidget *parent, const bool& deleteOnClose)
	: AListListDialog(dataDispatcher, parent, deleteOnClose)
	, m_modifDial(dataDispatcher, this)
	, m_nameDial(dataDispatcher, parent)
{
	m_nameDial.hide();
	m_modifDial.hide();

	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendListsList);
	m_methods.insert({ guiDType::sendListsList, &AListListDialog::receiveListList });
}

ListListDialog::~ListListDialog()
{
	saveList();
}

void ListListDialog::saveList()
{
	m_dataDispatcher.sendControl(new control::userlist::SaveLists());
}

void ListListDialog::receiveListList(IGuiData *data)
{
	GuiDataSendListsList *lData = static_cast<GuiDataSendListsList*>(data);
	int i = 0;

	if(m_model)
		delete m_model;
	m_model = new QStandardItemModel(0, 0);
	m_ui.listListView->setDragDropMode(QAbstractItemView::DragDropMode::NoDragDrop);
	m_ui.listListView->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	m_ui.listListView->setDragEnabled(false);
	m_ui.listListView->setAcceptDrops(false);
	m_ui.listListView->setDropIndicatorShown(false);

	for (SafePtr<UserList> list : lData->_lists)
	{
		ReadPtr<UserList> rList = list.cget();
		if (rList)
		{
			QStandardItem* item = new ListNode(QString::fromStdWString(rList->getName()), list, rList->getOrigin());
			m_model->setItem(i++, 0, item);
		}
	}

	m_ui.listListView->setModel(m_model);
	connect(m_ui.listListView->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(clickOnItem(const QModelIndex &)));

	m_ui.RemoveBtn->setEnabled(false);
	m_ui.EditBtn->setEnabled(false);
	m_ui.DuplicateBtn->setEnabled(false);
	m_ui.ExportBtn->setEnabled(false);

	GUI_LOG << "List list Dialog : " << m_model->rowCount() << " elements" << LOGENDL;
}

void ListListDialog::clickOnItem(const QModelIndex &idx)
{
	m_idSaved = idx;
	ListNode<UserList>* list = static_cast<ListNode<UserList>*>(m_model->itemFromIndex(m_ui.listListView->selectionModel()->currentIndex()));

	if (list->isOriginList() == false)
	{
		m_ui.RemoveBtn->setEnabled(true);
		m_ui.EditBtn->setEnabled(true);
		m_ui.DuplicateBtn->setEnabled(true);
		m_ui.ExportBtn->setEnabled(true);
	}
	else
	{
		m_ui.RemoveBtn->setEnabled(false);
		m_ui.EditBtn->setEnabled(true);
		m_ui.DuplicateBtn->setEnabled(true);
		m_ui.ExportBtn->setEnabled(true);
	}
	GUI_LOG << "click on item" << LOGENDL;
}

void ListListDialog::addNewList()
{
	GUI_LOG << "add new list" << LOGENDL;
	m_nameDial.show();
}

void ListListDialog::deleteList()
{
	QMessageBox::StandardButton reply;

	reply = QMessageBox::question(this, TEXT_DELETE_LIST_TITLE, TEXT_DELETE_LIST_QUESTION, QMessageBox::Yes | QMessageBox::No);
	if (reply == QMessageBox::Yes)
	{
		std::stringstream ss;
		QModelIndexList list = m_ui.listListView->selectionModel()->selectedIndexes();

		int i = 0;

		foreach(const QModelIndex &index, list)
		{
			ListNode<UserList>* list = static_cast<ListNode<UserList>*>(m_model->itemFromIndex(index));
			m_dataDispatcher.sendControl(new control::userlist::DeleteUserList(list->getList()));
		}
	}
	else
		GUI_LOG << "list not deleted" << LOGENDL;
}

void ListListDialog::listViewSelect()
{
	if (m_ui.listListView->selectionModel()->selectedIndexes().size() != 1)
	{
		m_dataDispatcher.updateInformation(new GuiDataWarning(TEXT_WARNING_SELECT_ONE_ELEMENT));
		return;
	}
	foreach (const QModelIndex &index, m_ui.listListView->selectionModel()->selectedIndexes())
	{
		ListNode<UserList>* list = static_cast<ListNode<UserList>*>(m_model->itemFromIndex(m_ui.listListView->selectionModel()->currentIndex()));
		//m_modifDial = new ListModifierDialog(m_dataDispatcher, this);
		//m_modifDial->show();
		m_modifDial.show();
		m_dataDispatcher.sendControl(new control::userlist::SendInfoUserList(list->getList()));
	}
}

void ListListDialog::importNewList()
{
	QString fileName = QFileDialog::getOpenFileName(this, TEXT_IMPORT_LIST,
		m_openPath, TEXT_FILE_LIST_IMPORT, nullptr);
	if (fileName.isEmpty())
		return;

	m_openPath = QString::fromStdWString(std::filesystem::path(fileName.toStdWString()).parent_path().wstring());
	m_dataDispatcher.sendControl(new control::userlist::ImportList(fileName.toStdWString()));
}

void ListListDialog::exportList()
{
	QString fileName = QFileDialog::getExistingDirectory(this, TEXT_EXPORT_LIST,
		m_openPath, QFileDialog::ShowDirsOnly);

	if (fileName.isEmpty())
		return;
	QModelIndex index = m_idSaved;
	m_openPath = fileName;
	ListNode<UserList>* list = static_cast<ListNode<UserList>*>(m_model->itemFromIndex(index));
	m_dataDispatcher.sendControl(new control::userlist::ExportList(fileName.toStdWString(), list->getList()));
}

void ListListDialog::duplicateList()
{
	if (m_ui.listListView->selectionModel()->selectedIndexes().size() != 1)
	{
		return;
		m_dataDispatcher.updateInformation(new GuiDataWarning(TEXT_WARNING_SELECT_ONE_ELEMENT));
	}
	QModelIndex index = m_idSaved;

	foreach (const QModelIndex &index, m_ui.listListView->selectionModel()->selectedIndexes())
	{
		ListNode<UserList>* list = static_cast<ListNode<UserList>*>(m_model->itemFromIndex(index));
		m_dataDispatcher.sendControl(new control::userlist::DuplicateUserList(list->getList()));
	}
}

void ListListDialog::FinishDialog()
{
	saveList();
	AListListDialog::FinishDialog();
}