#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>

#include "gui/Dialog/ListModifierDialog.h"
#include "gui/widgets/ListsNode.h"
#include "gui/GuiData/GuiDataList.h"
#include "controller/controls/ControlUserList.h"
#include "gui/texts/ListTexts.hpp"
#include "gui/widgets/CustomWidgets/qdoubleedit.h"
#include "utils/QtUtils.h"

ListModifierDialog::ListModifierDialog(IDataDispatcher& dataDispatcher, QDialog *parent)
	: AListModifierDialog(dataDispatcher, parent)
{
	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendUserList);
	m_methods.insert({ guiDType::sendUserList, &AListModifierDialog::getList });
}

ListModifierDialog::~ListModifierDialog()
{
	PANELLOG << "destroy ListModifierDialog" << LOGENDL;
}

void ListModifierDialog::getList(IGuiData *data)
{
	if (model != nullptr)
		model->blockSignals(true);
	GuiDataSendUserlist *lData = static_cast<GuiDataSendUserlist*>(data);

	m_list = lData->_list;

	ReadPtr<UserList> rList = m_list.cget();
	if (!rList)
	{
		close();
		return;
	}

	m_ui.NameLineEdit->blockSignals(true);
	m_ui.NameLineEdit->setText(QString::fromStdWString(rList->getName()));
	m_ui.NameLineEdit->blockSignals(false);
	int i = 0;

	if(model)
		delete model;
	model = new QStandardItemModel(0, 0);
	model->blockSignals(true);

	QObject::connect(model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(renameListViewElem(QStandardItem*)));
	m_ui.listView->setDragDropMode(QAbstractItemView::DragDropMode::NoDragDrop);
	m_ui.listView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
	m_ui.listView->setDragEnabled(false);
	m_ui.listView->setAcceptDrops(false);
	m_ui.listView->setDropIndicatorShown(false);

	std::unordered_map<std::wstring, std::wstring> map;
	for (const std::wstring& data : rList->clist())
		map.insert(std::pair<std::wstring, std::wstring>(data, data));

	for (auto it = map.begin(); it != map.end(); it++)
	{
		QStandardItem *item = new ItemNode(QString::fromStdWString(it->second), it->second);
		model->setItem(i++, 0, item);
	}

	m_ui.listView->setModel(model);
	model->blockSignals(false);
	PANELLOG << "userlist : " << rList->getName() << " with " << rList->clist().size() << " elems is shown" << LOGENDL;
}

void ListModifierDialog::showElemMenu(QPoint p)
{
	PANELLOG << "show elem menu" << LOGENDL;
	QMenu *menu = new QMenu(this);

	QAction *deleteAct = new QAction(TEXT_DELETE_ELEMENT, this);
	menu->addAction(deleteAct);
	QObject::connect(deleteAct, SIGNAL(triggered()), this, SLOT(deleteElem()));

	menu->popup(m_ui.listView->viewport()->mapToGlobal(p));
}

void ListModifierDialog::addNewElem()
{
	m_ui.NewElemLineEdit->blockSignals(true);
	if (!m_ui.NewElemLineEdit->text().isEmpty())
	{
		m_dataDispatcher.sendControl(new control::userlist::AddItemToUserList(m_list, m_ui.NewElemLineEdit->text().toStdWString()));
	}
	m_ui.NewElemLineEdit->setText("");
	m_ui.NewElemLineEdit->blockSignals(false);
	PANELLOG << "add a new elem" << LOGENDL;
}

void ListModifierDialog::renameListViewElem(QStandardItem *item)
{
	ItemNode *list = static_cast<ItemNode*>(item);

	m_dataDispatcher.sendControl(new control::userlist::RenameItemFromList(m_list, list->getWStrData(), item->text().toStdWString()));
	PANELLOG << "rename elem " << list->text().toStdString() << LOGENDL;
}

void ListModifierDialog::renameElem()
{
	ReadPtr<UserList> rList = m_list.cget();
	if (!rList)
		return;

	if (m_ui.NameLineEdit->text() != "" && m_ui.NameLineEdit->text().toStdWString() != rList->getName())
		m_dataDispatcher.sendControl(new control::userlist::RenameUserList(m_list, m_ui.NameLineEdit->text().toStdWString()));
	else
	{
		m_ui.NameLineEdit->blockSignals(true);
		m_ui.NameLineEdit->setText(QString::fromStdWString(rList->getName()));
		m_ui.NameLineEdit->blockSignals(false);
	}
	PANELLOG << "rename elem" << LOGENDL;
}

void ListModifierDialog::deleteElem()
{
	std::wstringstream ss;
	QModelIndexList list = m_ui.listView->selectionModel()->selectedIndexes();

	int i = 0;

	foreach(const QModelIndex &index, list)
	{
		ss << ((i == 0) ? "" : ", ");
		ItemNode *element = static_cast<ItemNode*>(model->itemFromIndex(index));
		ss << element->getWStrData();
		m_dataDispatcher.sendControl(new control::userlist::RemoveItemFromList(m_list, element->getWStrData()));
	}
	PANELLOG << "delete " << ss.str() << LOGENDL;
}

void ListModifierDialog::clearList()
{
	PANELLOG << "clear items of list " << LOGENDL;
	QMessageBox::StandardButton reply;

	reply = QMessageBox::question(this, TEXT_TITLE_CLEAN_LIST_BOX, TEXT_TITLE_CLEAN_LIST_BOX, QMessageBox::Yes | QMessageBox::No);
	if (reply == QMessageBox::Yes)
	{
		m_dataDispatcher.sendControl(new control::userlist::ClearItemFromList(m_list));
	}
	else
		PANELLOG << "click no to clear list" << LOGENDL;
}