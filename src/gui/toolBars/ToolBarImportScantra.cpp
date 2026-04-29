#include "gui/toolBars/ToolBarImportScantra.h"

#include "controller/controls/ControlIO.h"
#include "controller/controls/ControlApplication.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataIO.h"

#include "gui/Texts.hpp"

#include <QtWidgets/qfiledialog.h>
#include <QtCore/qstandardpaths.h>
#include <QtCore/qsettings.h>

namespace {
    // QSettings key for the application-wide "disable auto-zoom on Scantra
    // block adjustment" preference. Stored under the same organization name
    // OST already uses for window layout (see Gui.cpp). Default = false
    // (i.e. auto-zoom remains ENABLED) to preserve legacy behavior.
    constexpr const char* kDisableAutoZoomKey = "scantra/disableAutoZoomOnAdjustment";
}

ToolBarImportScantra::ToolBarImportScantra(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
{
	m_ui.setupUi(this);
	setEnabled(false);

	m_openPath = QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory);

	connect(m_ui.importScanTraButton, &QPushButton::clicked, this, &ToolBarImportScantra::slotImportScantra);
	connect(m_ui.disableAutoZoomCheckBox, &QCheckBox::toggled, this, &ToolBarImportScantra::slotDisableAutoZoomToggled);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectPath);

	loadAutoZoomPreference();
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

void ToolBarImportScantra::slotDisableAutoZoomToggled(bool checked)
{
	// Persist the preference so it survives application restarts.
	QSettings settings;
	settings.setValue(kDisableAutoZoomKey, checked);

	// Push the new value through the controller. Note: the checkbox semantics
	// are the inverse of the underlying ScantraInterface flag ("auto zoom
	// ENABLED"), so we negate here.
	m_dataDispatcher.sendControl(new control::application::SetScantraAutoZoomOnAdjustment(!checked));
}

void ToolBarImportScantra::loadAutoZoomPreference()
{
	QSettings settings;
	const bool disableAutoZoom = settings.value(kDisableAutoZoomKey, false).toBool();

	QSignalBlocker blocker(m_ui.disableAutoZoomCheckBox);
	m_ui.disableAutoZoomCheckBox->setChecked(disableAutoZoom);

	m_dataDispatcher.sendControl(new control::application::SetScantraAutoZoomOnAdjustment(!disableAutoZoom));
}
