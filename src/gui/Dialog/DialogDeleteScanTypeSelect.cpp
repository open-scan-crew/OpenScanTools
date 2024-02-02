#include <QtWidgets/QMenu>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtCore/QStandardPaths>

#include "controller/controls/ControlSpecial.h"

#include "gui/Dialog/DialogDeleteScanTypeSelect.h"
#include "gui/Texts.hpp"


DialogDeleteScanTypeSelect::DialogDeleteScanTypeSelect(IDataDispatcher& dataDispatcher,const std::unordered_map<SafePtr<AGraphNode>, std::pair<QString, QString>>& importantData,const std::unordered_set<SafePtr<AGraphNode>> & otherData, QWidget *parent)
	: QDialog(parent)
	, m_importantData(importantData)
	, m_otherData(otherData)
	, m_dataDispatcher(dataDispatcher)
	, m_waitUser(true)
{
	m_ui.setupUi(this);
	setModal(true);

	// ----- Important | Window Flags -----
	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::MSWindowsFixedSizeDialogHint;
	flags ^= Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);
	// ------------------------------------
	GUILOG << "create DialogDeleteScanTypeSelect" << LOGENDL;

	m_ui.tableWidget->setRowCount((int)m_importantData.size());
	m_ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
	int row = 0;
	for (auto importantData : m_importantData)
	{
		m_setImportantData.insert(importantData.first);
		QTableWidgetItem* filePath = new QTableWidgetItem(QDir::toNativeSeparators(importantData.second.second));
		filePath->setToolTip(QDir::toNativeSeparators(importantData.second.second));
		m_ui.tableWidget->setItem(row, 0, new QTableWidgetItem(importantData.second.first));
		m_ui.tableWidget->setItem(row, 1, filePath);
		row++;
	}

	if(m_importantData.empty())
	{
		if(m_otherData.size() > 0)
			m_dataDispatcher.sendControl(new control::special::DeleteElement(m_otherData, true));
		m_waitUser = false;
	}

	QObject::connect(m_ui.DeleteFileBtn, SIGNAL(clicked()), this, SLOT(HardDelete()));
	QObject::connect(m_ui.keepFileBtn, SIGNAL(clicked()), this, SLOT(SoftDelete()));
	QObject::connect(m_ui.CancelBtn, SIGNAL(clicked()), this, SLOT(Cancel()));

	m_ui.tableWidget->resizeColumnToContents(0);
	m_ui.tableWidget->resizeColumnToContents(1);
	m_ui.tableWidget->adjustSize();
	adjustSize();

	this->setAttribute(Qt::WA_DeleteOnClose);
	//setWindowFlags(Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint);
}

DialogDeleteScanTypeSelect::~DialogDeleteScanTypeSelect()
{
	GUILOG << "destroy DialogDeleteScanTypeSelect" << LOGENDL;
}


bool DialogDeleteScanTypeSelect::getWaitUser()
{
	return (m_waitUser); 
}

void DialogDeleteScanTypeSelect::HardDelete()
{
	QMessageBox messageBox(this);
	messageBox.setIcon(QMessageBox::Icon::Question);
	messageBox.setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
	messageBox.setWindowTitle(TEXT_IMPORTANT_DATA_REMOVAL_TITLE);
	messageBox.setText(TEXT_IMPORTANT_SCAN_REMOVAL_MESSAGE);
	QPushButton* yesButton = messageBox.addButton(TEXT_DIALOG_YES, QMessageBox::YesRole);
	messageBox.addButton(TEXT_DIALOG_NO, QMessageBox::NoRole);
	QPushButton* cancelButton = messageBox.addButton(TEXT_DIALOG_CANCEL, QMessageBox::RejectRole);
	messageBox.setDefaultButton(cancelButton);
	messageBox.exec();

	if (messageBox.clickedButton() == yesButton)
	{
		m_dataDispatcher.sendControl(new control::special::DeleteTotalData(m_setImportantData));
		if (m_otherData.size() > 0)
			m_dataDispatcher.sendControl(new control::special::DeleteElement(m_otherData, true));
	}

	close();
}

void DialogDeleteScanTypeSelect::SoftDelete()
{
	m_otherData.insert(m_setImportantData.begin(), m_setImportantData.end());
	if(m_otherData.size() > 0)
		m_dataDispatcher.sendControl(new control::special::DeleteElement(m_otherData, true));
	close();
}

void DialogDeleteScanTypeSelect::Cancel()
{
	close();
}