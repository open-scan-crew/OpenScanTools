#include "gui/toolBars/ToolBarConvertImage.h"
#include "gui/GuiData/GuiDataGeneralProject.h"


ToolBarConvertImage::ToolBarConvertImage(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
	, m_dialog(new DialogImportImage(dataDispatcher, parent))
{
	m_ui.setupUi(this);
	setEnabled(false);

	connect(m_ui.convertImageButton, &QPushButton::clicked, this, &ToolBarConvertImage::slotConvertImage);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
}

ToolBarConvertImage::~ToolBarConvertImage()
{
	m_dialog->close();
    m_dataDispatcher.unregisterObserver(this);
}

void ToolBarConvertImage::informData(IGuiData *data)
{
	switch (data->getType())
	{
		case guiDType::projectLoaded:
		{
			onProjectLoad(data);
		}
		break;
	}
}

void ToolBarConvertImage::onProjectLoad(IGuiData* data)
{
	GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(plData->m_isProjectLoad);
}

void ToolBarConvertImage::slotConvertImage()
{
	m_dialog->show();
}