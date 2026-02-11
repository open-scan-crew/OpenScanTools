#include "gui/toolBars/ToolBarViewPoint.h"
#include "controller/controls/ControlViewPoint.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
//#include <QtWidgets/qpushbutton.h>

ToolBarViewPoint::ToolBarViewPoint(IDataDispatcher& dataDispatcher, QWidget* parent, float scale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
{
	m_ui.setupUi(this);
	setEnabled(false);
	m_ui.createToolButton->setIconSize(QSize(20, 20) * scale);

	connect(m_ui.createToolButton, &QToolButton::clicked, this, &ToolBarViewPoint::initViewPointCreation);
	
	m_dataDispatcher.registerObserverOnKey(this, guiDType::activatedFunctions);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendListsStandards);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_methods.insert({ guiDType::activatedFunctions, &ToolBarViewPoint::onActivateFunction });
	m_methods.insert({ guiDType::projectLoaded, &ToolBarViewPoint::onProjectLoad });
}

ToolBarViewPoint::~ToolBarViewPoint()
{
	m_dataDispatcher.unregisterObserver(this);
}

void ToolBarViewPoint::informData(IGuiData* data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		ViewPointGroupMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarViewPoint::onProjectLoad(IGuiData* data)
{
	auto* function = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(function->m_isProjectLoad);
}

void ToolBarViewPoint::onActivateFunction(IGuiData* data)
{
	auto* function = static_cast<GuiDataActivatedFunctions*>(data);
}

void ToolBarViewPoint::initViewPointCreation()
{
	m_dataDispatcher.sendControl(new control::viewpoint::LaunchCreationContext());	
}
