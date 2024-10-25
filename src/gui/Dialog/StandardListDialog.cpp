#include <QtWidgets/QMenu>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtCore/QStandardPaths>

#include "gui/Dialog/StandardListDialog.h"
#include "gui/GuiData/GuiDataList.h"
#include "gui/widgets/ListsNode.h"
#include "controller/controls/ControlStandards.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ListTexts.hpp"
#include "utils/Logger.h"

StandardListDialog::StandardListDialog(IDataDispatcher& dataDispatcher, const StandardType& type, QWidget *parent, const bool& deleteOnClose)
	: AListListDialog(dataDispatcher, parent, deleteOnClose)
	, m_type(type)
	, m_modifDial(dataDispatcher, type, this)
	, m_nameDial(dataDispatcher, type, parent)
{
	m_nameDial.hide();
	m_modifDial.hide();
	switch (type)
	{
	case StandardType::Pipe:
		setWindowTitle(TEXT_PIPE_STANDARD_LIST_DIALOG_TITLE);
		break;
	case StandardType::Sphere:
		setWindowTitle(TEXT_SPHERE_STANDARD_LIST_DIALOG_TITLE);
		break;
	}

	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendListsStandards);
	m_methods.insert({ guiDType::sendListsStandards, &AListListDialog::receiveListList });

}

StandardListDialog::~StandardListDialog()
{
	saveList();
}

void StandardListDialog::saveList()
{
	m_dataDispatcher.sendControl(new control::standards::SaveLists(m_type));
}

void StandardListDialog::receiveListList(IGuiData *data)
{
	GuiDataSendListsStandards*lData = static_cast<GuiDataSendListsStandards*>(data);
	if (lData->m_type != m_type)
		return;
	int i = 0;
	
	if (m_model)
		delete m_model;
	m_model = new QStandardItemModel(0, 0);
	m_ui.listListView->setDragDropMode(QAbstractItemView::DragDropMode::NoDragDrop);
	m_ui.listListView->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	m_ui.listListView->setDragEnabled(false);
	m_ui.listListView->setAcceptDrops(false);
	m_ui.listListView->setDropIndicatorShown(false);

	for (SafePtr<StandardList> stand : lData->m_lists)
	{
		ReadPtr<StandardList> rStand = stand.cget();
		if (rStand && rStand->getOrigin() == false)
		{
			QStandardItem* item = new ListNode<StandardList>(QString::fromStdWString(rStand->getName()), stand, rStand->getOrigin());
			m_model->setItem(i++, 0, item);
		}
	}

	m_ui.listListView->setModel(m_model);
	connect(m_ui.listListView->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(clickOnItem(const QModelIndex &)));

	m_ui.RemoveBtn->setEnabled(false);
	m_ui.EditBtn->setEnabled(false);
	m_ui.DuplicateBtn->setEnabled(false);
	m_ui.ExportBtn->setEnabled(false);

	GUI_LOG << "Standard list dialog : " << m_model->rowCount() << " elements received" << LOGENDL;
}

void StandardListDialog::clickOnItem(const QModelIndex &idx)
{
	m_idSaved = idx;
	ListNode<StandardList>* list = static_cast<ListNode<StandardList>*>(m_model->itemFromIndex(m_ui.listListView->selectionModel()->currentIndex()));

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

void StandardListDialog::addNewList()
{
	GUI_LOG << "add new list" << LOGENDL;
	m_nameDial.show();
}

void StandardListDialog::deleteList()
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
			ListNode<StandardList>* list = static_cast<ListNode<StandardList>*>(m_model->itemFromIndex(index));
			m_dataDispatcher.sendControl(new control::standards::DeleteStandard(list->getList(), m_type));
		}
	}
	else
		GUI_LOG << "list not deleted" << LOGENDL;
}

void StandardListDialog::listViewSelect()
{
	if (m_ui.listListView->selectionModel()->selectedIndexes().size() != 1)
	{
		m_dataDispatcher.updateInformation(new GuiDataWarning(TEXT_WARNING_SELECT_ONE_ELEMENT));
		return;
	}
	foreach (const QModelIndex &index, m_ui.listListView->selectionModel()->selectedIndexes())
	{
		ListNode<StandardList>* list = static_cast<ListNode<StandardList>*>(m_model->itemFromIndex(m_ui.listListView->selectionModel()->currentIndex()));
		//m_modifDial = new StandardModifierDialog(m_dataDispatcher, this);
		m_modifDial.show();
		m_dataDispatcher.sendControl(new control::standards::SendInfoStandard(list->getList(), m_type));
	}
}

void StandardListDialog::importNewList()
{
	QString fileName = QFileDialog::getOpenFileName(this, TEXT_IMPORT_LIST,
		m_openPath, TEXT_FILE_LIST_IMPORT, nullptr);
	if (fileName.isEmpty())
		return;

	m_openPath = QString::fromStdWString(std::filesystem::path(fileName.toStdWString()).parent_path().wstring());
	m_dataDispatcher.sendControl(new control::standards::ImportList(fileName.toStdWString(), m_type));
}

void StandardListDialog::exportList()
{
	QString fileName = QFileDialog::getExistingDirectory(this, TEXT_EXPORT_LIST,
		m_openPath, QFileDialog::ShowDirsOnly);
	if (fileName.isEmpty() || !m_idSaved.isValid())
		return;
	QModelIndex index = m_idSaved;

	m_openPath = fileName;

	ListNode<StandardList>* list = static_cast<ListNode<StandardList>*>(m_model->itemFromIndex(index));
	m_dataDispatcher.sendControl(new control::standards::ExportList(fileName.toStdWString(), list->getList(), m_type));
}

void StandardListDialog::duplicateList()
{
	if (m_ui.listListView->selectionModel()->selectedIndexes().size() != 1)
	{
		return;
		m_dataDispatcher.updateInformation(new GuiDataWarning(TEXT_WARNING_SELECT_ONE_ELEMENT));
	}
	QModelIndex index = m_idSaved;

	foreach (const QModelIndex &index, m_ui.listListView->selectionModel()->selectedIndexes())
	{
		ListNode<StandardList> *list = static_cast<ListNode<StandardList>*>(m_model->itemFromIndex(index));
		m_dataDispatcher.sendControl(new control::standards::DuplicateStandard(list->getList(), m_type));
	}
}

void StandardListDialog::FinishDialog()
{
	saveList();
	AListListDialog::FinishDialog();
}