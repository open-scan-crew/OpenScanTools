#include "gui/Dialog/DialogImportFileObject.h"
#include "gui/GuiData/GuiDataIO.h"

#include "controller/controls/ControlModal.h"
#include "controller/controls/ControlFunction.h"
#include "controller/messages/FilesMessage.h"

#include "gui/texts/FileSystemTexts.hpp"

#include "models/graph/MeshObjectNode.h"
#include "models/graph/PointCloudNode.h"


#include <QtWidgets/qfiledialog.h>
#include <QtCore/qstandardpaths.h>

DialogImportFileObject::DialogImportFileObject(IDataDispatcher& dataDispatcher, QWidget* parent)
	: ADialog(dataDispatcher, parent)
{
	m_ui.setupUi(this);
	setModal(true);

	m_openPath = QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectPath);

	QObject::connect(m_ui.FileFolderBtn, SIGNAL(clicked()), this, SLOT(ImportFiles()));
	QObject::connect(m_ui.IgnoreBtn, SIGNAL(clicked()), this, SLOT(Ignore()));
	QObject::connect(m_ui.CancelBtn, SIGNAL(clicked()), this, SLOT(Cancel()));
}

DialogImportFileObject::~DialogImportFileObject()
{}

void DialogImportFileObject::setInfoFileObjects(const std::unordered_set<SafePtr<AGraphNode>>& notFoundFileObjects, bool isOnlyLink)
{
	m_ui.tableWidget->clear();
	m_ui.tableWidget->setRowCount((int)notFoundFileObjects.size());
	m_ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
	int row = 0;
	for (const SafePtr<AGraphNode>& infoPtr : notFoundFileObjects)
	{
		QString objectName;
		QString qFileName;
		ElementType type;
		{
			ReadPtr<AGraphNode> readInfo = infoPtr.cget();
			if (!readInfo)
				continue;
			objectName = QString::fromStdWString(readInfo->getName());
			type = readInfo->getType();
		}

		switch (type)
		{
			case ElementType::Scan:
			case ElementType::PCO:
			{
				ReadPtr<PointCloudNode> readInfo = static_pointer_cast<PointCloudNode>(infoPtr).cget();
				if (!readInfo)
					continue;
				qFileName = QDir::toNativeSeparators(QString::fromStdWString(readInfo->getScanPath().wstring()));
			}
			case ElementType::MeshObject:
			{
				ReadPtr<MeshObjectNode> readInfo = static_pointer_cast<MeshObjectNode>(infoPtr).cget();
				if (!readInfo)
					continue;
				qFileName = QDir::toNativeSeparators(QString::fromStdWString(readInfo->getFilePath().wstring()));
			}
		}

		QTableWidgetItem* fileNameItem = new QTableWidgetItem(qFileName);
		fileNameItem->setToolTip(qFileName);
		m_ui.tableWidget->setItem(row, 0, new QTableWidgetItem(objectName));
		m_ui.tableWidget->setItem(row, 1, fileNameItem);
		row++;
	}

	m_ui.tableWidget->resizeColumnToContents(0);
	m_ui.tableWidget->resizeColumnToContents(1);
	m_ui.tableWidget->adjustSize();

	if (isOnlyLink)
	{
		m_ui.IgnoreBtn->hide();
	}
	else
	{
		m_ui.IgnoreBtn->show();
	}

	adjustSize();
}

void DialogImportFileObject::informData(IGuiData * data)
{
	switch (data->getType())
	{
		case guiDType::projectPath:
		{
			auto dataType = static_cast<GuiDataProjectPath*>(data);
			m_openPath = QString::fromStdWString(dataType->m_path.wstring());
		}
		break;
	}
}

void DialogImportFileObject::ImportFiles()
{
	QString folderPath = QFileDialog::getExistingDirectory(this, TEXT_SELECT_DIRECTORY, m_openPath, QFileDialog::Option());

	if (folderPath != "")
	{
		m_openPath = folderPath;
		std::filesystem::path folder = folderPath.toStdWString();
		m_dataDispatcher.sendControl(new control::function::ForwardMessage(new FilesMessage({ folder }, 1)));
		hide();
	}
}

void DialogImportFileObject::Ignore()
{
	m_dataDispatcher.sendControl(new control::modal::ModalReturnValue(0));
	hide();
}

void DialogImportFileObject::Cancel()
{
	m_dataDispatcher.sendControl(new control::function::Abort());
	hide();
}
