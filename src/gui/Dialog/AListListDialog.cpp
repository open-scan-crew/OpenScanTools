#include <QtWidgets/QMenu>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtCore/QStandardPaths>

#include "gui/Dialog/AListListDialog.h"
#include "gui/GuiData/GuiDataList.h"
#include "gui/GuiData/GuiDataIO.h"
#include "gui/Dialog/ListNameDialog.h"
#include "gui/Dialog/ListModifierDialog.h"
#include "gui/widgets/ListsNode.h"
#include "controller/controls/ControlUserList.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ListTexts.hpp"

AListListDialog::AListListDialog(IDataDispatcher& dataDispatcher, QWidget *parent, const bool& deleteOnClose)
	: ADialog(dataDispatcher, parent)
	, m_openPath(QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory))
{
	m_ui.setupUi(this);
	PANELLOG << "create AListListDialog" << LOGENDL;

	m_ui.RemoveBtn->setEnabled(false);
	m_ui.EditBtn->setEnabled(false);
	m_ui.DuplicateBtn->setEnabled(false);
	m_ui.ExportBtn->setEnabled(false);

	QObject::connect(m_ui.listListView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showTreeMenu(QPoint)));
	QObject::connect(m_ui.NewListBtn, SIGNAL(clicked()), this, SLOT(addNewList()));
	QObject::connect(m_ui.ExportBtn, SIGNAL(clicked()), this, SLOT(exportList()));
	QObject::connect(m_ui.listListView, &QListView::doubleClicked, this, &AListListDialog::listViewSelect);
	QObject::connect(m_ui.FinishBtn, SIGNAL(clicked()), this, SLOT(FinishDialog()));
	QObject::connect(m_ui.ImportListBtn, SIGNAL(clicked()), this, SLOT(importNewList()));
	QObject::connect(m_ui.RemoveBtn, SIGNAL(clicked()), this, SLOT(deleteList()));
	QObject::connect(m_ui.EditBtn, SIGNAL(clicked()), this, SLOT(listViewSelect()));
	QObject::connect(m_ui.DuplicateBtn, SIGNAL(clicked()), this, SLOT(duplicateList()));

	// Creation of the contextual menu
	m_contextualMenu = new QMenu(this);
	QAction* deleteAct = new QAction(TEXT_ACTION_DELETE_LIST, this);
	m_contextualMenu->addAction(deleteAct);
	QAction* exportAct = new QAction(TEXT_ACTION_EXPORT_LIST, this);
	m_contextualMenu->addAction(exportAct);
	QAction* duplicateAct = new QAction(TEXT_ACTION_DUPLICATE_LIST, this);
	m_contextualMenu->addAction(duplicateAct);
	QObject::connect(deleteAct, SIGNAL(triggered()), this, SLOT(deleteList()));
	QObject::connect(exportAct, SIGNAL(triggered()), this, SLOT(exportList()));
	QObject::connect(duplicateAct, SIGNAL(triggered()), this, SLOT(duplicateList()));

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectPath);
	m_methods.insert({ guiDType::projectPath, &AListListDialog::onProjectPath });

	if(deleteOnClose)
		setAttribute(Qt::WA_DeleteOnClose);
}

AListListDialog::~AListListDialog()
{
	PANELLOG << "destroy AListListDialog" << LOGENDL;
	m_dataDispatcher.unregisterObserver(this);
}

void AListListDialog::informData(IGuiData *data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		ListListMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void AListListDialog::onProjectPath(IGuiData* data)
{
	auto dataType = static_cast<GuiDataProjectPath*>(data);
	m_openPath = QString::fromStdWString(dataType->m_path.wstring());
}

void AListListDialog::showTreeMenu(QPoint p)
{
	//PANELLOG << "show Tree Menu" << LOGENDL;
	m_idSaved = m_ui.listListView->indexAt(p);
	if (m_idSaved.isValid() == false)
		return;

	m_contextualMenu->popup(m_ui.listListView->viewport()->mapToGlobal(p));
}

void AListListDialog::FinishDialog()
{
	close();
}