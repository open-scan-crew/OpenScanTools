#include "gui/toolBars/ToolBarSphereGroup.h"
#include "gui/Dialog/StandardListDialog.h"
#include "controller/controls/ControlMeasure.h"
#include "controller/controls/ControlStandards.h"
#include "gui/GuiData/GuiDataGeneralProject.h"

ToolBarSphereGroup::ToolBarSphereGroup(IDataDispatcher& dataDispatcher, QWidget* parent, float scale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
{
	m_ui.setupUi(this);
	setEnabled(false);
	m_ui.sphereToolButton->setIconSize(QSize(20, 20) * scale);
	m_ui.robustToolButton->setIconSize(QSize(20, 20) * scale);

	connect(m_ui.sphereToolButton, &QToolButton::released, this, &ToolBarSphereGroup::initSphereDetection);
	connect(m_ui.robustToolButton, &QToolButton::released, this, &ToolBarSphereGroup::initLargeSphereDetection);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::activatedFunctions);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendListsStandards);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_methods.insert({ guiDType::activatedFunctions, &ToolBarSphereGroup::onActivateFunction });
	m_methods.insert({ guiDType::projectLoaded, &ToolBarSphereGroup::onProjectLoad });
}

ToolBarSphereGroup::~ToolBarSphereGroup()
{
	m_dataDispatcher.unregisterObserver(this);
}

void ToolBarSphereGroup::informData(IGuiData* data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		SphereGroupMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarSphereGroup::onProjectLoad(IGuiData* data)
{
	auto* function = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(function->m_isProjectLoad);
}

void ToolBarSphereGroup::onActivateFunction(IGuiData* data)
{
	auto* function = static_cast<GuiDataActivatedFunctions*>(data);

	m_ui.sphereToolButton->blockSignals(true);
	m_ui.robustToolButton->blockSignals(true);

	if (function->type == ContextType::Sphere)
	{
		m_ui.sphereToolButton->setChecked(true);
		m_ui.robustToolButton->setChecked(false);
	}
    else if (function->type == ContextType::ClicsSphere4)
	{
		m_ui.sphereToolButton->setChecked(false);
		m_ui.robustToolButton->setChecked(true);
	}
	else
	{
		m_ui.sphereToolButton->setChecked(false);
		m_ui.robustToolButton->setChecked(false);
	}

	m_ui.sphereToolButton->blockSignals(false);
	m_ui.robustToolButton->blockSignals(false);
}

void ToolBarSphereGroup::initSphereDetection()
{
	m_dataDispatcher.sendControl(new control::measure::ActivateSphere());	
}

void ToolBarSphereGroup::initLargeSphereDetection()
{
	m_dataDispatcher.sendControl(new control::measure::Activate4ClicsSphere());
}

void ToolBarSphereGroup::initSphereStandardManagement()
{
	StandardListDialog* dialog(new StandardListDialog(m_dataDispatcher, StandardType::Sphere, this->parentWidget(), true)); 
	m_dataDispatcher.sendControl(new control::standards::SendStandards(StandardType::Sphere));
	dialog->show();
}