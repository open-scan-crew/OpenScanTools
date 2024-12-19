#include "gui\toolBars\ToolBarManipulateObjects.h"
#include "gui/GuiData/GuiDataGeneralProject.h"

#include "controller/controls/ControlObject3DEdition.h"


ToolBarManipulateObjects::ToolBarManipulateObjects(IDataDispatcher &dataDispatcher, QWidget *parent, float guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
{
	m_ui.setupUi(this);

	setEnabled(false);

	connect(m_ui.translate, &QPushButton::released, this, &ToolBarManipulateObjects::translate);
	connect(m_ui.translateLockZ, &QPushButton::released, this, &ToolBarManipulateObjects::translateLockZ);
	connect(m_ui.translateAlongZ, &QPushButton::released, this, &ToolBarManipulateObjects::translateAlongZ);
	connect(m_ui.rotate2points, &QPushButton::released, this, &ToolBarManipulateObjects::rotate2points);
	connect(m_ui.rotate3points, &QPushButton::released, this, &ToolBarManipulateObjects::rotate3points);
	
	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::activatedFunctions);

	m_methods.insert({ guiDType::activatedFunctions, &ToolBarManipulateObjects::onActivateFunction });
	m_methods.insert({ guiDType::projectLoaded, &ToolBarManipulateObjects::onProjectLoad });
}

ToolBarManipulateObjects::~ToolBarManipulateObjects()
{
	m_dataDispatcher.unregisterObserver(this);
}

void ToolBarManipulateObjects::onProjectLoad(IGuiData* data)
{
	auto* pldata = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(pldata->m_isProjectLoad);
}

void ToolBarManipulateObjects::onActivateFunction(IGuiData* data)
{
	auto* function = static_cast<GuiDataActivatedFunctions*>(data);
	if (function->type == ContextType::manipulateObjects)
		return;

	m_ui.translate->blockSignals(true);
	m_ui.translateLockZ->blockSignals(true);
	m_ui.rotate2points->blockSignals(true);
	m_ui.rotate3points->blockSignals(true);

	m_ui.translate->setChecked(false);
	m_ui.translateLockZ->setChecked(false);
	m_ui.rotate2points->setChecked(false);
	m_ui.rotate3points->setChecked(false);

	m_ui.translate->blockSignals(false);
	m_ui.translateLockZ->blockSignals(false);
	m_ui.rotate2points->blockSignals(false);
	m_ui.rotate3points->blockSignals(false);
}

void ToolBarManipulateObjects::translate()
{
	m_dataDispatcher.sendControl(new control::object3DEdition::LaunchManipulateContext(false, ZMovement::Default));
}

void ToolBarManipulateObjects::translateLockZ()
{
	m_dataDispatcher.sendControl(new control::object3DEdition::LaunchManipulateContext(false, ZMovement::Lock));
}

void ToolBarManipulateObjects::translateAlongZ()
{
	m_dataDispatcher.sendControl(new control::object3DEdition::LaunchManipulateContext(false, ZMovement::Along));
}

void ToolBarManipulateObjects::rotate2points()
{
	m_dataDispatcher.sendControl(new control::object3DEdition::LaunchManipulateContext(true, ZMovement::Lock));
}

void ToolBarManipulateObjects::rotate3points()
{
	m_dataDispatcher.sendControl(new control::object3DEdition::LaunchManipulateContext(true, ZMovement::Default));
}

void ToolBarManipulateObjects::informData(IGuiData *data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		constraintToolBarMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}