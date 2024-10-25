#include <QtWidgets/QMenu>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtCore/QStandardPaths>

#include "gui/Dialog/AuthorListDialog.h"
#include "gui/widgets/AuthorListNode.h"
#include "controller/controls/ControlApplication.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/Texts.hpp"
#include "utils/Logger.h"

AuthorListDialog::AuthorListDialog(IDataDispatcher& dataDispatcher, QWidget *parent)
	: ADialog(dataDispatcher, parent)
	, m_haveToQuit(false)
{
	m_ui.setupUi(this);
	this->show();
	
	GUI_LOG << "create AuthorListDialog" << LOGENDL;

	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendAuthorsList);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::closeAuthorList);

	this->setAttribute(Qt::WA_DeleteOnClose);
	
	QObject::connect(m_ui.AddNewBtn, SIGNAL(clicked()), this, SLOT(addNewAuthor()));
	QObject::connect(m_ui.OkBtn, SIGNAL(clicked()), this, SLOT(FinishDialog()));
	QObject::connect(m_ui.RemoveUserBtn, SIGNAL(clicked()), this, SLOT(deleteAuthor()));
}

AuthorListDialog::~AuthorListDialog()
{
	GUI_LOG << "destroy AuthorListDialog" << LOGENDL;
	m_dataDispatcher.sendControl(new control::application::author::SaveAndQuitAuthors());
	m_dataDispatcher.unregisterObserver(this);
}

void AuthorListDialog::informData(IGuiData *data)
{
	if (data->getType() == guiDType::sendAuthorsList)
		receiveAuthorList(data);
	else if (data->getType() == guiDType::closeAuthorList)
		receiveCloseAuthorDialog(data);
}

void AuthorListDialog::show()
{
	m_ui.OkBtn->setFocus();
	QDialog::show();
}

void AuthorListDialog::receiveAuthorList(IGuiData *data)
{
	GuiDataSendAuthorsList *lData = static_cast<GuiDataSendAuthorsList*>(data);
	int i = 0;

	if (lData->m_isProjectScope == true)
		return;
	if (model)
		delete model;
	model = new QStandardItemModel(0, 0);
	m_ui.AuthorListView->setDragDropMode(QAbstractItemView::DragDropMode::NoDragDrop);
	m_ui.AuthorListView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
	m_ui.AuthorListView->setDragEnabled(false);
	m_ui.AuthorListView->setAcceptDrops(false);
	m_ui.AuthorListView->setDropIndicatorShown(false);

	for (auto it = lData->m_authors.begin(); it != lData->m_authors.end(); it++)
	{
		SafePtr<Author> auth = (*it);
		ReadPtr<Author> rAuth = auth.cget();
		if (!rAuth)
			continue;
		if (!i)
			m_dataDispatcher.sendControl(new control::application::author::SelectAuthor(auth));

		QStandardItem * item = new AuthorListNode(QString::fromStdWString(rAuth->getName()), auth);
		item->setFlags(item->flags() & ~Qt::ItemIsEditable);
		model->setItem(i++, 0, item);
	}
	m_ui.AuthorListView->setModel(model);
	
	if (i > 0)
	{
		m_ui.AuthorListView->selectAll();
		m_ui.AuthorListView->setFocus();

		QModelIndexList list = m_ui.AuthorListView->selectionModel()->selectedIndexes();

		auto iterator(list.begin());
		for (int counter(0); counter < lData->m_selectedAuthor; counter++)
			iterator++;
		m_ui.AuthorListView->setCurrentIndex(*iterator);
		m_ui.AuthorListView->setFocus();

		QMetaObject::invokeMethod(m_ui.AuthorListView, "clicked", Q_ARG(QModelIndex, *list.begin()));
	}
	m_haveToQuit = (i != 0) ? true : false;

	m_ui.AuthorListView->setModel(model);
	ListConnect = QObject::connect(m_ui.AuthorListView, &QListView::clicked, this, &AuthorListDialog::authorViewSelect);
	GUI_LOG << "receive list of author of " << lData->m_authors.size() << "elems" << LOGENDL;
}

void AuthorListDialog::receiveCloseAuthorDialog(IGuiData * data)
{
	if (m_haveToQuit == true)
	{
		this->close();
		delete (this);
	}
}

void AuthorListDialog::addNewAuthor()
{
	GUI_LOG << "add new list" << LOGENDL;
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
			m_dataDispatcher.sendControl(new control::application::author::DeleteAuthor(list->getAuthor()));
		}
		m_haveToQuit = (i != 0) ? true : false;
		GUI_LOG << "delete " << ss.str() << LOGENDL;
	}
	else
		GUI_LOG << "list not deleted" << LOGENDL;
}

void AuthorListDialog::authorViewSelect()
{
	if (m_ui.AuthorListView->selectionModel()->selectedIndexes().size() != 1)
	{
		m_dataDispatcher.updateInformation(new GuiDataWarning(TEXT_WARNING_AUTHOR_SELECT_ONE));
		return;
	}
	foreach (const QModelIndex &index, m_ui.AuthorListView->selectionModel()->selectedIndexes())
	{
		AuthorListNode *list = static_cast<AuthorListNode*>(model->itemFromIndex(m_ui.AuthorListView->selectionModel()->currentIndex()));
		m_dataDispatcher.sendControl(new control::application::author::SelectAuthor(list->getAuthor()));
	}
}

void AuthorListDialog::FinishDialog()
{
	if (m_haveToQuit)
	{
		QObject::disconnect(ListConnect);
		m_dataDispatcher.sendControl(new control::application::author::SaveAndQuitAuthors());
		delete(this);
	}
	else
		addNewAuthor();
}