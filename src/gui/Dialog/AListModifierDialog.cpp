#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>

#include "gui/Dialog/AListModifierDialog.h"
#include "gui/widgets/ListsNode.h"
#include "gui/GuiData/GuiDataList.h"
#include "controller/controls/ControlUserList.h"
#include "gui/texts/ListTexts.hpp"
#include "utils/QtUtils.h"

AListModifierDialog::AListModifierDialog(IDataDispatcher& dataDispatcher, QDialog *parent)
	: ADialog(dataDispatcher, parent)
{
	m_ui.setupUi(this);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendUserList);
	m_ui.listView->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

	QObject::connect(m_ui.AddElemBtn, SIGNAL(clicked()), this, SLOT(addNewElem()));
	QObject::connect(m_ui.listView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showElemMenu(QPoint)));
	QObject::connect(m_ui.FinishBtn, SIGNAL(clicked()), this, SLOT(closeDialog()));
	QObject::connect(m_ui.ClearBtn, SIGNAL(clicked()), this, SLOT(clearList()));
	//connect(m_ui.lineEditIndex, SIGNAL(editingFinished()), this, SLOT(changeUserIndex()));
	QObject::connect(m_ui.NameLineEdit, SIGNAL(editingFinished()), this, SLOT(renameElem()));
	QObject::connect(m_ui.RemoveBtn, SIGNAL(clicked()), this, SLOT(deleteElem()));
	PANELLOG << "create AListModifierDialog" << LOGENDL;
}

AListModifierDialog::~AListModifierDialog()
{
	PANELLOG << "destroy AListModifierDialog" << LOGENDL;
	m_dataDispatcher.unregisterObserver(this);
}

void AListModifierDialog::informData(IGuiData *data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		ListModifierMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
	PANELLOG << "informdata ListModifier" << LOGENDL;
}

void AListModifierDialog::show()
{
	if(m_ui.listView->model() && m_ui.listView->model()->rowCount() > 0)
		m_ui.listView->model()->removeRows(0, m_ui.listView->model()->rowCount());
	QDialog::show();
}

void AListModifierDialog::showElemMenu(QPoint p)
{
	PANELLOG << "show elem menu" << LOGENDL;
	QMenu *menu = new QMenu(this);

	QAction *deleteAct = new QAction(TEXT_DELETE_ELEMENT, this);
	menu->addAction(deleteAct);
	QObject::connect(deleteAct, SIGNAL(triggered()), this, SLOT(deleteElem()));

	menu->popup(m_ui.listView->viewport()->mapToGlobal(p));
}

void AListModifierDialog::closeDialog()
{
	hide();
}