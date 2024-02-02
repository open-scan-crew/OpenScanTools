#include <QtWidgets/QMenu>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtCore/QStandardPaths>

#include "controller/controls/ControlModal.h"
#include "controller/controls/ControlFunction.h"

#include "gui/Dialog/DialogExportFileObject.h"


DialogExportFileObject::DialogExportFileObject(IDataDispatcher& dataDispatcher, std::unordered_map<std::wstring, std::wstring> infoFileObjects, QWidget* parent)
	: QDialog(parent)
	, m_dataDispatcher(dataDispatcher)
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

	m_ui.tableWidget->setRowCount((int)infoFileObjects.size());
	m_ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
	int row = 0;
	for (auto info : infoFileObjects)
	{
		QString objectName = QString::fromStdWString(info.first);
		QString filePath = QString::fromStdWString(info.second);

		QTableWidgetItem* filePathItem = new QTableWidgetItem(filePath);
		filePathItem->setToolTip(filePath);
		m_ui.tableWidget->setItem(row, 0, new QTableWidgetItem(objectName));
		m_ui.tableWidget->setItem(row, 1, filePathItem);
		row++;
	}

	QObject::connect(m_ui.okBtn, SIGNAL(clicked()), this, SLOT(Ok()));
	QObject::connect(m_ui.CancelBtn, SIGNAL(clicked()), this, SLOT(Cancel()));

	m_ui.tableWidget->resizeColumnToContents(0);
	m_ui.tableWidget->resizeColumnToContents(1);
	m_ui.tableWidget->adjustSize();
	adjustSize();

	this->setAttribute(Qt::WA_DeleteOnClose);
}

DialogExportFileObject::~DialogExportFileObject()
{
	GUILOG << "destroy DialogDeleteScanTypeSelect" << LOGENDL;
}

void DialogExportFileObject::Ok()
{
	m_dataDispatcher.sendControl(new control::modal::ModalReturnValue(0));
	close();
}

void DialogExportFileObject::Cancel()
{
	m_dataDispatcher.sendControl(new control::function::Abort());
	close();
}