#include "gui/toolBars/ToolBarImportObjects.h"

#include "controller/controls/ControlIO.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataIO.h"

#include "gui/Texts.hpp"

#include <QtWidgets/QFileDialog>
#include <QtCore/QStandardPaths>

ToolBarImportObjects::ToolBarImportObjects(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
{
	m_ui.setupUi(this);
	setEnabled(false);

	m_openPath = QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory);

	connect(m_ui.importTagsButton, &QPushButton::released, this, &ToolBarImportObjects::slotImportObjects);
	connect(m_ui.linkMissingFilesButton, &QPushButton::released, this, &ToolBarImportObjects::slotLinkObjects);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectPath);
}

ToolBarImportObjects::~ToolBarImportObjects()
{
    m_dataDispatcher.unregisterObserver(this);
}

void ToolBarImportObjects::informData(IGuiData *data)
{
	switch (data->getType())
	{
		case guiDType::projectPath:
		{
			auto dataType = static_cast<GuiDataProjectPath*>(data);
			m_openPath = QString::fromStdWString(dataType->m_path.wstring());
		}
		break;
		case guiDType::projectLoaded:
		{
			onProjectLoad(data);
		}
		break;
	}
}

void ToolBarImportObjects::onProjectLoad(IGuiData* data)
{
	GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(plData->m_isProjectLoad);
}

void ToolBarImportObjects::slotLinkObjects()
{
	m_dataDispatcher.sendControl(new control::io::LinkOSTObjectsContext());
}

void ToolBarImportObjects::slotImportObjects()
{
	QFileDialog dialog;
	dialog.setModal(true);
	QStringList qFilespath = dialog.getOpenFileNames(this, TEXT_IMPORT_SHARED_OBJECTS, m_openPath, TEXT_IMPORT_TYPE_ALL_OBJECTS_OPEN, nullptr);

	std::vector<std::filesystem::path> filesPath;
	for (QString path : qFilespath)
		filesPath.push_back(path.toStdWString());

	if (!filesPath.empty())
		m_openPath = QString::fromStdWString(filesPath.begin()->wstring());

	m_dataDispatcher.sendControl(new control::io::ImportOSTObjects(filesPath));
}