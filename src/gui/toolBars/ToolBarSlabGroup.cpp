#include "gui/toolBars/ToolBarSlabGroup.h"
#include "controller/controls/ControlMeasure.h"
#include "controller/controls/ControlFunction.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
//#include <QtWidgets/qpushbutton.h>

ToolBarSlabGroup::ToolBarSlabGroup(IDataDispatcher& dataDispatcher, QWidget* parent, float scale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
{
	m_ui.setupUi(this);
	setEnabled(false);

	connect(m_ui.oneClickToolButton, &QToolButton::released, this, &ToolBarSlabGroup::init1Click);
	connect(m_ui.threeClickToolButton, &QToolButton::released, this, &ToolBarSlabGroup::init2Click);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::activatedFunctions);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);

	m_methods.insert({ guiDType::activatedFunctions, &ToolBarSlabGroup::onActivateFunction });
	m_methods.insert({ guiDType::projectLoaded, &ToolBarSlabGroup::onProjectLoad });

}

ToolBarSlabGroup::~ToolBarSlabGroup()
{
	m_dataDispatcher.unregisterObserver(this);
}

void ToolBarSlabGroup::informData(IGuiData* data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		SlabGroupMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarSlabGroup::onProjectLoad(IGuiData* data)
{
	auto* function = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(function->m_isProjectLoad);
}

void ToolBarSlabGroup::onActivateFunction(IGuiData* data)
{
	auto* function = static_cast<GuiDataActivatedFunctions*>(data);

	m_ui.oneClickToolButton->blockSignals(true);
	m_ui.threeClickToolButton->blockSignals(true);
	switch (function->type) {
		case ContextType::Slab2Click:
		{
			m_ui.oneClickToolButton->setChecked(false);
			m_ui.threeClickToolButton->setChecked(true);
		}
		break;
		case ContextType::Slab1Click:
		{
			m_ui.oneClickToolButton->setChecked(true);
			m_ui.threeClickToolButton->setChecked(false);
		}
		break;
		default:
		{
			m_ui.oneClickToolButton->setChecked(false);
			m_ui.threeClickToolButton->setChecked(false);
		}
		break;
	}

	m_ui.oneClickToolButton->blockSignals(false);
	m_ui.threeClickToolButton->blockSignals(false);
}

void ToolBarSlabGroup::init1Click()
{
	if (m_ui.oneClickToolButton->isChecked())
		m_dataDispatcher.sendControl(new control::measure::ActivateSlabMeasure(true, false));
	else
		m_dataDispatcher.sendControl(new control::function::Abort());
}

void ToolBarSlabGroup::init2Click()
{
	if (m_ui.threeClickToolButton->isChecked())
		m_dataDispatcher.sendControl(new control::measure::ActivateSlabMeasure(true, true));
	else
		m_dataDispatcher.sendControl(new control::function::Abort());
}
