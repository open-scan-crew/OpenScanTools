#include "gui/toolBars/ToolBarRenderEnhance.h"
#include "gui/GuiData/GuiDataGeneralProject.h"

ToolBarRenderEnhance::ToolBarRenderEnhance(IDataDispatcher& dataDispatcher, QWidget* parent, float guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
{
	m_ui.setupUi(this);
	setEnabled(false);
	(void)guiScale;

	registerGuiDataFunction(guiDType::projectLoaded, &ToolBarRenderEnhance::onProjectLoad);
}

void ToolBarRenderEnhance::informData(IGuiData* data)
{
	auto it = m_methods.find(data->getType());
	if (it != m_methods.end())
	{
		GuiDataFunction method = it->second;
		(this->*method)(data);
	}
}

void ToolBarRenderEnhance::onProjectLoad(IGuiData* data)
{
	GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(plData->m_isProjectLoad);
}
