#include "gui/Dialog/AuthorListDialog.h"
#include "gui/Dialog/AuthorCreateDialog.h"
#include "gui/widgets/AuthorListNode.h"
#include "controller/controls/ControlAuthor.h"
#include "gui/GuiData/GuiDataAuthor.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/AuthorTexts.hpp"
#include "utils/Logger.h"

#include <QtWidgets/qmessagebox.h>


AuthorListDialog::AuthorListDialog(IDataDispatcher& dataDispatcher, QWidget *parent)
	: ADialog(dataDispatcher, parent)
{
	m_ui.setupUi(this);
	this->show();
	
	GUI_LOG << "create AuthorListDialog" << LOGENDL;

	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendAuthorsList);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::closeAuthorList);

	this->setAttribute(Qt::WA_DeleteOnClose);
	
	QObject::connect(m_ui.AddNewBtn, SIGNAL(clicked()), this, SLOT(addNewAuthor()));
	QObject::connect(m_ui.OkBtn, SIGNAL(clicked()), this, SLOT(closeDialog()));
	QObject::connect(m_ui.RemoveUserBtn, SIGNAL(clicked()), this, SLOT(deleteAuthor()));

	m_dataDispatcher.sendControl(new control::author::SendAuthorList());
}

AuthorListDialog::~AuthorListDialog()
{
	GUI_LOG << "destroy AuthorListDialog" << LOGENDL;
	m_dataDispatcher.sendControl(new control::author::SaveAuthors());
	m_dataDispatcher.unregisterObserver(this);
}

void AuthorListDialog::informData(IGuiData *data)
{
	if (data->getType() == guiDType::sendAuthorsList)
		receiveAuthorList(data);
}

void AuthorListDialog::show()
{
	m_ui.OkBtn->setFocus();
	QDialog::show();
}

void AuthorListDialog::receiveAuthorList(IGuiData *data)
{
	GuiDataSendAuthorsList *lData = static_cast<GuiDataSendAuthorsList*>(data);

	if (lData->is_project_scope_ == true)
		return;
	if (model)
		delete model;
	model = new QStandardItemModel(0, 0);
	m_ui.AuthorListView->setDragDropMode(QAbstractItemView::DragDropMode::NoDragDrop);
	m_ui.AuthorListView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
	m_ui.AuthorListView->setDragEnabled(false);
	m_ui.AuthorListView->setAcceptDrops(false);
	m_ui.AuthorListView->setDropIndicatorShown(false);

	int select_row = 0;
	for (auto it = lData->authors_.begin(); it != lData->authors_.end(); it++)
	{
		SafePtr<Author> auth = (*it);
		ReadPtr<Author> rAuth = auth.cget();
		if (!rAuth)
			continue;

		QString item_str = QString::fromStdWString(rAuth->getName());
		if (auth == lData->active_author_)
		{
			item_str += L" [*]";
			select_row = model->rowCount();
		}
		QStandardItem * item = new AuthorListNode(item_str, auth);
		item->setFlags(item->flags() & ~Qt::ItemIsEditable);
		model->appendRow(item);
	}
	m_ui.AuthorListView->setModel(model);
	
	QModelIndex index = model->index(select_row, 0);
	m_ui.AuthorListView->setCurrentIndex(index);

	m_ui.AuthorListView->setFocus();
	GUI_LOG << "receive list of author of " << lData->authors_.size() << "elems" << LOGENDL;
}

void AuthorListDialog::addNewAuthor()
{
	QDialog * dial = new AuthorCreateDialog(m_dataDispatcher, this);
	dial->show();
}

void AuthorListDialog::deleteAuthor()
{
	QMessageBox::StandardButton reply;

	reply = QMessageBox::question(this, TEXT_DELETE_AUTHOR_TITLE, TEXT_DELETE_AUTHOR_QUESTION, QMessageBox::Yes | QMessageBox::No);
	if (reply == QMessageBox::Yes)
	{
		std::wstringstream ss;
		QModelIndexList list = m_ui.AuthorListView->selectionModel()->selectedIndexes();

		int i = 0;

		foreach (const QModelIndex &index, list)
		{
			ss << ((i == 0) ? L"" : L", ");
			AuthorListNode *list = static_cast<AuthorListNode*>(model->itemFromIndex(index));
			ss << list->text().toStdWString();
			m_dataDispatcher.sendControl(new control::author::DeleteAuthor(list->getAuthor()));
		}
		GUI_LOG << "delete " << ss.str() << LOGENDL;
	}
	else
		GUI_LOG << "list not deleted" << LOGENDL;
}

bool AuthorListDialog::selectAuthor()
{
	if (m_ui.AuthorListView->selectionModel()->selectedIndexes().size() != 1)
	{
		m_dataDispatcher.updateInformation(new GuiDataWarning(TEXT_WARNING_AUTHOR_SELECT_ONE));
		return false;
	}

	QModelIndex index = m_ui.AuthorListView->selectionModel()->currentIndex();
	AuthorListNode* list = static_cast<AuthorListNode*>(model->itemFromIndex(index));
	m_dataDispatcher.sendControl(new control::author::SelectAuthor(list->getAuthor()));
	return true;
}

void AuthorListDialog::closeDialog()
{
	if (selectAuthor())
		delete(this);
}