#include "gui/toolBars/ToolBarExportVideo.h"
#include "gui/GuiData/GuiDataGeneralProject.h"

ToolBarExportVideo::ToolBarExportVideo(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
	, m_dialog(new DialogExportVideo(dataDispatcher, this, guiScale))
{
	m_ui.setupUi(this);
	setEnabled(false);

	connect(m_ui.generateVideoPushButton, &QPushButton::clicked, this, &ToolBarExportVideo::generateVideo);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
}

ToolBarExportVideo::~ToolBarExportVideo()
{
    m_dataDispatcher.unregisterObserver(this);
}

void ToolBarExportVideo::informData(IGuiData *data)
{
	if (data->getType() == guiDType::projectLoaded)
		onProjectLoad(data);
}

void ToolBarExportVideo::onProjectLoad(IGuiData* data)
{
	GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(plData->m_isProjectLoad);
}

void ToolBarExportVideo::generateVideo()
{
	m_dialog->show();
}
