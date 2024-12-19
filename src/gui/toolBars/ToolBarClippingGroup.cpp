#include "gui/toolBars/ToolBarClippingGroup.h"

#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataClipping.h"

#include "controller/controls/ControlFunctionClipping.h"
#include "controller/controls/ControlFunction.h"
#include "gui/Texts.hpp"

ToolBarClippingGroup::ToolBarClippingGroup(IDataDispatcher& dataDispatcher, QWidget* parent, float guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
{
	m_ui.setupUi(this);
	setEnabled(false);
	m_ui.LocalBoxBtn->setIconSize(QSize(20, 20) * guiScale);
	m_ui.GlobalBoxBtn->setIconSize(QSize(20, 20) * guiScale);
	m_ui.CopyBoxBtn->setIconSize(QSize(20, 20) * guiScale);
	m_ui.AttachedBoxBtn->setIconSize(QSize(20, 20) * guiScale);
	m_ui.AttachedBox2PointsBtn->setIconSize(QSize(20, 20) * guiScale);
	m_ui.CreationSettingsBtn->setIconSize(QSize(20, 20) * guiScale);

	QObject::connect(m_ui.LocalBoxBtn, &QToolButton::released, this, &ToolBarClippingGroup::clickLocalBox);
	QObject::connect(m_ui.GlobalBoxBtn, &QToolButton::released, this, &ToolBarClippingGroup::clickGlobalBox);
	QObject::connect(m_ui.CopyBoxBtn, &QToolButton::released, this, &ToolBarClippingGroup::clickCopyBox);
	QObject::connect(m_ui.AttachedBoxBtn, &QToolButton::released, this, &ToolBarClippingGroup::clickAttachedBox);
	QObject::connect(m_ui.AttachedBox2PointsBtn, &QToolButton::released, this, &ToolBarClippingGroup::clickAttachedBox2Points);
	QObject::connect(m_ui.CreationSettingsBtn, &QToolButton::released, this, &ToolBarClippingGroup::clickClippingProperties);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::activatedFunctions);

	m_methods.insert({ guiDType::activatedFunctions, &ToolBarClippingGroup::onCopyDone });
	m_methods.insert({ guiDType::projectLoaded, &ToolBarClippingGroup::onProjectLoad });



}

void ToolBarClippingGroup::informData(IGuiData *data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		ClipGroupMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarClippingGroup::onProjectLoad(IGuiData * data)
{
	GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(plData->m_isProjectLoad);
	m_ui.LocalBoxBtn->setEnabled(true);
	m_ui.CopyBoxBtn->setEnabled(true);
	m_ui.AttachedBoxBtn->setEnabled(true);
}

void  ToolBarClippingGroup::onCopyDone(IGuiData* data)
{
	auto function = static_cast<GuiDataActivatedFunctions*>(data);
	m_ui.LocalBoxBtn->blockSignals(true);
	m_ui.CopyBoxBtn->blockSignals(true);
	m_ui.AttachedBoxBtn->blockSignals(true);
	m_ui.AttachedBox2PointsBtn->blockSignals(true);

	switch (function->type)
	{
		case ContextType::clippingBoxAttached3Points:
		{
			m_ui.LocalBoxBtn->setChecked(false);
			m_ui.CopyBoxBtn->setChecked(false);
			m_ui.AttachedBoxBtn->setChecked(true);
			m_ui.AttachedBox2PointsBtn->setChecked(false);
		}
		break;
		case ContextType::clippingBoxAttached2Points:
		{
			m_ui.LocalBoxBtn->setChecked(false);
			m_ui.CopyBoxBtn->setChecked(false);
			m_ui.AttachedBoxBtn->setChecked(false);
			m_ui.AttachedBox2PointsBtn->setChecked(true);
		}
		break;
		case ContextType::boxDuplication:
		{
			m_ui.LocalBoxBtn->setChecked(false);
			m_ui.CopyBoxBtn->setChecked(true);
			m_ui.AttachedBoxBtn->setChecked(false);
			m_ui.AttachedBox2PointsBtn->setChecked(false);
		}
		break;
		case ContextType::clippingBoxCreation:
		{
			m_ui.LocalBoxBtn->setChecked(true);
			m_ui.CopyBoxBtn->setChecked(false);
			m_ui.AttachedBoxBtn->setChecked(false);
			m_ui.AttachedBox2PointsBtn->setChecked(false);
		}
		break;
		default:
		{
			m_ui.LocalBoxBtn->setChecked(false);
			m_ui.CopyBoxBtn->setChecked(false);
			m_ui.AttachedBoxBtn->setChecked(false);
			m_ui.AttachedBox2PointsBtn->setChecked(false);
			m_ui.LocalBoxBtn->clearFocus();
			m_ui.CopyBoxBtn->clearFocus();
			m_ui.AttachedBoxBtn->clearFocus();
			m_ui.AttachedBox2PointsBtn->clearFocus();
		}
	}
	m_ui.LocalBoxBtn->blockSignals(false);
	m_ui.CopyBoxBtn->blockSignals(false);
	m_ui.AttachedBoxBtn->blockSignals(false);
	m_ui.AttachedBox2PointsBtn->blockSignals(false);
}

void ToolBarClippingGroup::clickClippingProperties()
{
	m_dataDispatcher.updateInformation(new GuiDataClippingSettingsProperties(TEXT_BOXES_SETTINGS_PROPERTIES_NAME, false));
}

void ToolBarClippingGroup::clickLocalBox()
{
	if (m_ui.LocalBoxBtn->isChecked())
		m_dataDispatcher.sendControl(new control::function::clipping::ActivateCreateLocal());
	else
		m_dataDispatcher.sendControl(new control::function::Abort());
}

void ToolBarClippingGroup::clickAttachedBox()
{
	if (m_ui.AttachedBoxBtn->isChecked())
		m_dataDispatcher.sendControl(new control::function::clipping::ActivateCreateAttached());
	else
		m_dataDispatcher.sendControl(new control::function::Abort());
}

void ToolBarClippingGroup::clickAttachedBox2Points()
{
	if (m_ui.AttachedBox2PointsBtn->isChecked())
		m_dataDispatcher.sendControl(new control::function::clipping::ActivateCreateAttached2Points());
	else
		m_dataDispatcher.sendControl(new control::function::Abort());
}

void ToolBarClippingGroup::clickGlobalBox()
{
	m_dataDispatcher.sendControl(new control::function::clipping::CreateGlobal());
}

void ToolBarClippingGroup::clickCopyBox()
{
	if (m_ui.CopyBoxBtn->isChecked())
		m_dataDispatcher.sendControl(new control::function::clipping::ActivateDuplicate());
	else
		m_dataDispatcher.sendControl(new control::function::Abort());
}

ToolBarClippingGroup::~ToolBarClippingGroup()
{
	m_dataDispatcher.unregisterObserver(this);
}