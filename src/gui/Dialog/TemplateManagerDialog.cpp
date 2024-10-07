#include <QtWidgets/QMenu>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtCore/QStandardPaths>
#include "gui/Dialog/TemplateManagerDialog.h"
#include "gui/Dialog/TemplateEditorDialog.h"
#include "gui/GuiData/GuiDataTemplate.h"
#include "gui/widgets/TemplatesNode.h"
#include "controller/controls/ControlTemplateEdit.h"
#include "controller/controls/ControlFunction.h"
#include "controller/controls/ControlIO.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/FileTypeTexts.hpp"
#include "gui/Texts.hpp"
#include "utils/Logger.h"

TemplateManagerDialog::TemplateManagerDialog(IDataDispatcher &dataDispacher, QWidget *parent, const bool& deleteOnClose)
	: ADialog(dataDispacher, parent)
	, m_ui(new Ui::TemplateManager)
	, m_templateEditorDialog(dataDispacher, parent)
	, m_templateNameDialog(dataDispacher, this)
{
	m_ui->setupUi(this);
	m_templateEditorDialog.hide();
	m_templateNameDialog.hide();
	GUI_LOG << "create TemplateManagerDialog" << LOGENDL;

	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendTemplateList);
	m_ui->templateList->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

	//ui->templateList->dou
	QObject::connect(m_ui->templateList, SIGNAL(doubleClicked(const QModelIndex&)), SLOT(editTemplate(const QModelIndex&)));
	QObject::connect(m_ui->templateList, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showTreeMenu(QPoint)));
	QObject::connect(m_ui->DeleteBtn, SIGNAL(clicked()), this, SLOT(deleteTemplate()));
	QObject::connect(m_ui->DuplicateBtn, SIGNAL(clicked()), this, SLOT(duplicateTemplate()));
	QObject::connect(m_ui->EditBtn, SIGNAL(clicked()), this, SLOT(editSelectedTemplate()));
	QObject::connect(m_ui->ExportBtn, SIGNAL(clicked()), this, SLOT(exportTemplate()));
	QObject::connect(m_ui->ImportBtn, SIGNAL(clicked()), this, SLOT(importTemplate()));
	QObject::connect(m_ui->FinishBtn, SIGNAL(clicked()), this, SLOT(FinishDialog()));
	QObject::connect(m_ui->NewBtn, SIGNAL(clicked()), this, SLOT(newTemplate()));

	if(deleteOnClose)
		setAttribute(Qt::WA_DeleteOnClose);
}

TemplateManagerDialog::~TemplateManagerDialog()
{
	GUI_LOG << "delete TemplateManagerDialog" << LOGENDL;
	m_dataDispatcher.sendControl(new control::tagTemplate::SaveTemplates);
	m_dataDispatcher.unregisterObserver(this);
}

void TemplateManagerDialog::informData(IGuiData *data)
{
	if (data->getType() == guiDType::sendTemplateList)
		receiveTemplateList(data);
}

void TemplateManagerDialog::clickOnItem(const QModelIndex &idx)
{
	m_idSaved = idx;
	TemplateListNode *tempNode = static_cast<TemplateListNode*>(m_model->itemFromIndex(m_ui->templateList->selectionModel()->currentIndex()));

	if (tempNode->getOriginTemplate() == false)
	{
		m_ui->DeleteBtn->setEnabled(true);
		m_ui->EditBtn->setEnabled(true);
		m_ui->DuplicateBtn->setEnabled(true);
		m_ui->ExportBtn->setEnabled(true);
	}
	else
	{
		m_ui->DeleteBtn->setEnabled(false);
		m_ui->EditBtn->setEnabled(false);
		m_ui->DuplicateBtn->setEnabled(true);
		m_ui->ExportBtn->setEnabled(false);
	}

	GUI_LOG << "click on item " << tempNode->text().toStdWString() << LOGENDL;
}

void TemplateManagerDialog::receiveTemplateList(IGuiData * data)
{
	GuiDataSendTemplateList *lData = static_cast<GuiDataSendTemplateList*>(data);
	int i = 0;

	if(m_model)
		delete m_model;
	m_model = new QStandardItemModel(0, 0);
	m_ui->templateList->setDragDropMode(QAbstractItemView::DragDropMode::NoDragDrop);
	m_ui->templateList->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	m_ui->templateList->setDragEnabled(false);
	m_ui->templateList->setAcceptDrops(false);
	m_ui->templateList->setDropIndicatorShown(false);

	for (SafePtr<sma::TagTemplate> temp : lData->m_templates)
	{
		TemplateListNode * item = new TemplateListNode(temp);
		m_model->setItem(i++, 0, item);
	}

	m_ui->templateList->setModel(m_model);
	connect(m_ui->templateList->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(clickOnItem(const QModelIndex &)));

	m_ui->DeleteBtn->setEnabled(false);
	m_ui->EditBtn->setEnabled(false);
	m_ui->DuplicateBtn->setEnabled(false);
	m_ui->ExportBtn->setEnabled(false);

	GUI_LOG << "Template manager dialog : " << lData->m_templates.size() << " elements received" << LOGENDL;
}

void TemplateManagerDialog::deleteTemplate()
{
	QMessageBox::StandardButton reply;

	reply = QMessageBox::question(this, TEXT_TITLE_DELETE_TEMPLATE_BOX, TEXT_MESSAGE_DELETE_TEMPLATE_BOX, QMessageBox::Yes | QMessageBox::No);
	if (reply == QMessageBox::Yes)
	{
		QModelIndexList list = m_ui->templateList->selectionModel()->selectedIndexes();

		int i = 0;
		m_dataDispatcher.sendControl(new control::function::Abort());

		foreach(const QModelIndex &index, list)
		{
			TemplateListNode *list = static_cast<TemplateListNode*>(m_model->itemFromIndex(index));
			m_dataDispatcher.sendControl(new control::tagTemplate::DeleteTagTemplate(list->getTemplate()));
		}
	}
	else
		GUI_LOG << "template not deleted" << LOGENDL;
}

void TemplateManagerDialog::showTreeMenu(QPoint p)
{
	GUI_LOG << "show Tree Menu" << LOGENDL;

	m_idSaved = m_ui->templateList->indexAt(p);
	if (m_idSaved.isValid() == false)
		return;
	QMenu *menu = new QMenu(this);

	QAction *deleteAct = new QAction(TEXT_DELETE_TEMPLATE_BUTTON, this);
	menu->addAction(deleteAct);
	QAction *exportAct = new QAction(TEXT_EXPORT_TEMPLATE_BOX, this);
	menu->addAction(exportAct);
	QAction *duplicateAct = new QAction(TEXT_DUPLICATE_TEMPLATE_BOX, this);
	menu->addAction(duplicateAct);
	QAction *editAct = new QAction(TEXT_EDIT_TEMPLATE_BOX, this);
	menu->addAction(editAct);
	QObject::connect(deleteAct, SIGNAL(triggered()), this, SLOT(deleteTemplate()));
	QObject::connect(exportAct, SIGNAL(triggered()), this, SLOT(exportTemplate()));
	QObject::connect(duplicateAct, SIGNAL(triggered()), this, SLOT(duplicateTemplate()));
	QObject::connect(editAct, SIGNAL(triggered()), this, SLOT(editTemplate()));

	menu->popup(m_ui->templateList->viewport()->mapToGlobal(p));
}

void TemplateManagerDialog::newTemplate()
{
	GUI_LOG << "add new template" << LOGENDL;
	m_templateNameDialog.show();
}

void TemplateManagerDialog::duplicateTemplate()
{
	if (m_ui->templateList->selectionModel()->selectedIndexes().size() != 1)
	{
		m_dataDispatcher.updateInformation(new GuiDataWarning(TEXT_TEMPLATE_WARNING_SELECT_ONE));
		return;
	}
	QModelIndex index = m_idSaved;

	foreach (const QModelIndex &index, m_ui->templateList->selectionModel()->selectedIndexes())
	{
		TemplateListNode *list = static_cast<TemplateListNode*>(m_model->itemFromIndex(index));
		m_dataDispatcher.sendControl(new control::tagTemplate::DuplicateTagTemplate(list->getTemplate()));
	}
}

void TemplateManagerDialog::editSelectedTemplate()
{
	if (m_ui->templateList->selectionModel()->selectedIndexes().size() != 1)
	{
		m_dataDispatcher.updateInformation(new GuiDataWarning(TEXT_TEMPLATE_WARNING_SELECT_ONE));
		return;
	}
	foreach (const QModelIndex &index, m_ui->templateList->selectionModel()->selectedIndexes())
	{
		editTemplate(index);
	}
}

void TemplateManagerDialog::editTemplate(const QModelIndex& index)
{
	TemplateListNode *list = static_cast<TemplateListNode*>(m_model->itemFromIndex(index));
	m_templateEditorDialog.show();
	m_dataDispatcher.sendControl(new control::tagTemplate::SendTagTemplate(list->getTemplate()));
}

void TemplateManagerDialog::exportTemplate()
{
	if (m_ui->templateList->selectionModel()->selectedIndexes().size() < 1)
	{
		m_dataDispatcher.updateInformation(new GuiDataWarning(TEXT_TEMPLATE_WARNING_SELECT_MULTIPLE));
		return;
	}
	QString fileName = QFileDialog::getExistingDirectory(this, TEXT_EXPORT_TEMPLATE,
		QString(), QFileDialog::ShowDirsOnly);

	if (fileName.isEmpty())
		return;

	std::unordered_set<SafePtr<sma::TagTemplate>> ids;
	foreach(const QModelIndex &index, m_ui->templateList->selectionModel()->selectedIndexes())
	{
		TemplateListNode *temp = static_cast<TemplateListNode*>(m_model->itemFromIndex(index));
		ids.insert(temp->getTemplate());
	}
	if (ids.size() > 0)
		m_dataDispatcher.sendControl(new control::io::ExportTemplate(fileName.toStdWString(), ids));
}

void TemplateManagerDialog::importTemplate()
{
	QString fileName = QFileDialog::getOpenFileName(this, TEXT_IMPORT_TEMPLATE,
		QString(), TEXT_FILE_TYPE_TLT, nullptr);
	if (fileName.isEmpty())
		return;

	m_dataDispatcher.sendControl(new control::io::ImportTemplate(fileName.toStdWString()));
}

void TemplateManagerDialog::FinishDialog()
{
	m_dataDispatcher.sendControl(new control::tagTemplate::SaveTemplates);
	hide();
}
