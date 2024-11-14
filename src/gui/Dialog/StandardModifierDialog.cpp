#include <QtWidgets/qmenu.h>
#include <QtWidgets/qmessagebox.h>

#include "gui/Dialog/StandardModifierDialog.h"
#include "gui/widgets/ListsNode.h"
#include "gui/GuiData/GuiDataList.h"
#include "controller/controls/ControlStandards.h"
#include "gui/texts/ListTexts.hpp"
#include "gui/widgets/CustomWidgets/qdoubleedit.h"
#include "utils/QtUtils.h"
#include "utils/Logger.h"

StandardModifierDialog::StandardModifierDialog(IDataDispatcher& dataDispatcher, const StandardType& type, QDialog *parent)
	: AListModifierDialog(dataDispatcher, parent)
	, m_type(type)
{
	switch (type)
	{
	case StandardType::Pipe:
		setWindowTitle(tr("Pipe standard editor"));
		break;
	case StandardType::Sphere:
		setWindowTitle(tr("Sphere standard editor"));
		break;
	}
	m_ui.indLabel->setText(TEXT_IND_UNIT_IN_METERS);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendStandardList);
	m_methods.insert({ guiDType::sendStandardList, &AListModifierDialog::getList });
	QDoubleEdit* doubleEdit(new QDoubleEdit());
	doubleEdit->setPlaceholderText(TEXT_INPUT_MASK_VALUE);
	doubleEdit->setRules(ANumericLineEdit::LineEditRules::PositiveStrict);
	utils::replace(m_ui.NewElemLineEdit, doubleEdit);
	m_ui.NewElemLineEdit = doubleEdit;
}

StandardModifierDialog::~StandardModifierDialog()
{
	GUI_LOG << "destroy StandardModifierDialog" << LOGENDL;
}

void StandardModifierDialog::getList(IGuiData *data)
{
	if (model != nullptr)
		model->blockSignals(true);
	GuiDataSendStandards*lData = static_cast<GuiDataSendStandards*>(data);
	if (lData->m_type != m_type)
		return;
	m_list = lData->m_list;

	ReadPtr<StandardList> rList = m_list.cget();
	if(!rList)
	{
		close();
		return;
	}

	m_ui.NameLineEdit->blockSignals(true);
	m_ui.NameLineEdit->setText(QString::fromStdWString(rList->getName()));
	m_ui.NameLineEdit->blockSignals(false);
	int i = 0;
	
	if (model)
		delete model;
	model = new QStandardItemModel(0, 0);
	model->blockSignals(true);

	QObject::connect(model, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(renameListViewElem(QStandardItem*)));
	m_ui.listView->setDragDropMode(QAbstractItemView::DragDropMode::NoDragDrop);
	m_ui.listView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
	m_ui.listView->setDragEnabled(false);
	m_ui.listView->setAcceptDrops(false);
	m_ui.listView->setDropIndicatorShown(false);

	std::unordered_map<double, double> map;
	for (double value : rList->clist())
		map.insert(std::pair<double, double>(value, value));

	for (auto it = map.begin(); it != map.end(); it++)
	{
		QStandardItem *item = new ItemNode(QString::number(it->second), std::to_wstring(it->second));
		model->setItem(i++, 0, item);
	}

	m_ui.listView->setModel(model);
	model->blockSignals(false);
	GUI_LOG << "userlist : " << rList->getName() << " with " << rList->clist().size() << " elems is shown" << LOGENDL;
}

void StandardModifierDialog::showElemMenu(QPoint p)
{
	GUI_LOG << "show elem menu" << LOGENDL;
	QMenu *menu = new QMenu(this);

	QAction *deleteAct = new QAction(TEXT_DELETE_ELEMENT, this);
	menu->addAction(deleteAct);
	QObject::connect(deleteAct, SIGNAL(triggered()), this, SLOT(deleteElem()));

	menu->popup(m_ui.listView->viewport()->mapToGlobal(p));
}

void StandardModifierDialog::addNewElem()
{
	m_ui.NewElemLineEdit->blockSignals(true);
	QDoubleEdit* doubleEdit = static_cast<QDoubleEdit*>(m_ui.NewElemLineEdit);
	if (doubleEdit->getValue())
		m_dataDispatcher.sendControl(new control::standards::AddItemToStandard(m_list, doubleEdit->getValue(), m_type));
	m_ui.NewElemLineEdit->clear();
	m_ui.NewElemLineEdit->blockSignals(false);
	GUI_LOG << "add a new elem" << LOGENDL;
}

void StandardModifierDialog::renameListViewElem(QStandardItem *item)
{
	ItemNode *list = static_cast<ItemNode*>(item);

	m_dataDispatcher.sendControl(new control::standards::RenameItemFromList(m_list, std::stod(list->getWStrData()), item->text().toDouble(), m_type));
	GUI_LOG << "rename elem " << list->text().toStdString() << LOGENDL;
}

void StandardModifierDialog::renameElem()
{
	QString listName;
	{
		ReadPtr<StandardList> rStand = m_list.cget();
		if (rStand)
			listName = QString::fromStdWString(rStand->getName());
	}

	if (m_ui.NameLineEdit->text() != "" && m_ui.NameLineEdit->text() != listName)
		m_dataDispatcher.sendControl(new control::standards::RenameStandard(m_list, m_ui.NameLineEdit->text().toStdWString(), m_type));
	else
	{
		ReadPtr<StandardList> rStand = m_list.cget();
		if (rStand)
		{
			m_ui.NameLineEdit->blockSignals(true);
			m_ui.NameLineEdit->setText(listName);
			m_ui.NameLineEdit->blockSignals(false);
		}
	}
	GUI_LOG << "rename elem" << LOGENDL;
}

void StandardModifierDialog::deleteElem()
{
	std::wstringstream ss;
	QModelIndexList list = m_ui.listView->selectionModel()->selectedIndexes();

	int i = 0;

	foreach(const QModelIndex &index, list)
	{
		ss << ((i == 0) ? "" : ", ");
		ItemNode *element = static_cast<ItemNode*>(model->itemFromIndex(index));
		ss << element->getWStrData();
		m_dataDispatcher.sendControl(new control::standards::RemoveItemFromList(m_list, std::stod(element->getWStrData()), m_type));
	}
	GUI_LOG << "delete " << ss.str() << LOGENDL;
}

void StandardModifierDialog::clearList()
{
	GUI_LOG << "clear items of list " << LOGENDL;
	QMessageBox::StandardButton reply;

	reply = QMessageBox::question(this, TEXT_TITLE_CLEAN_LIST_BOX, TEXT_TITLE_CLEAN_LIST_BOX, QMessageBox::Yes | QMessageBox::No);
	if (reply == QMessageBox::Yes)
	{
		m_dataDispatcher.sendControl(new control::standards::ClearItemFromList(m_list, m_type));
	}
	else
		GUI_LOG << "click no to clear list" << LOGENDL;
}