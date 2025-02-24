#include "gui/toolBars/ToolBarImportScantra.h"

#include "controller/controls/ControlIO.h"
#include "controller/controls/ControlApplication.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataIO.h"

#include "gui/Texts.hpp"

#include <QtWidgets/qfiledialog.h>
#include <QtCore/qstandardpaths.h>

ToolBarImportScantra::ToolBarImportScantra(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
{
	m_ui.setupUi(this);
	setEnabled(false);

	m_openPath = QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory);

	connect(m_ui.importScanTraButton, &QPushButton::released, this, &ToolBarImportScantra::slotImportScantra);
	connect(m_ui.switch_connexion_btn, &QPushButton::released, this, &ToolBarImportScantra::slotSwitchConnexion);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectPath);
}

ToolBarImportScantra::~ToolBarImportScantra()
{
    m_dataDispatcher.unregisterObserver(this);
}

void ToolBarImportScantra::informData(IGuiData *data)
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

void ToolBarImportScantra::onProjectLoad(IGuiData* data)
{
	GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(plData->m_isProjectLoad);
}

void ToolBarImportScantra::slotImportScantra()
{
	QFileDialog dialog;
	dialog.setModal(true);
	QString qFilepath = dialog.getOpenFileName(this, TEXT_IMPORT_SCANTRA, m_openPath, TEXT_IMPORT_SCANTRA_FILES, nullptr);

	std::filesystem::path filePath = qFilepath.toStdWString();
	m_openPath = qFilepath;

	m_dataDispatcher.sendControl(new control::io::ImportScantraModifications(filePath));
}

void ToolBarImportScantra::slotSwitchConnexion()
{
	m_dataDispatcher.sendControl(new control::application::SwitchScantraConnexion(!m_interprocess_started));
	m_interprocess_started = !m_interprocess_started;

	m_ui.switch_connexion_btn->setText(m_interprocess_started ?
		"Disconnect with Scantra" :
		"Connect with Scantra");
}
