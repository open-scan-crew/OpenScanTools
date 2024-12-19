#include "gui/toolBars/ToolBarMeshObjectGroup.h"

#include "controller/controls/ControlFunction.h"
#include "controller/controls/ControlMeshObject.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataClipping.h"

#include "gui/Texts.hpp"

ToolBarMeshObjectGroup::ToolBarMeshObjectGroup(IDataDispatcher& dataDispatcher, QWidget* parent, float guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
	, m_dialog(dataDispatcher, parent)
{
	m_ui.setupUi(this);
	setEnabled(false);

	m_ui.creationSettingsBtn->setIconSize(QSize(20, 20) * guiScale);
	m_ui.copyBtn->setIconSize(QSize(20, 20) * guiScale);
	m_ui.importFromFileBtn->setIconSize(QSize(20, 20) * guiScale);

	QObject::connect(m_ui.creationSettingsBtn, &QToolButton::released, this, &ToolBarMeshObjectGroup::clickWavefrontProperties);
	QObject::connect(m_ui.copyBtn, &QToolButton::released, this, &ToolBarMeshObjectGroup::clickCopy);
	QObject::connect(m_ui.importFromFileBtn, &QToolButton::released, this, &ToolBarMeshObjectGroup::clickFromFile);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::activatedFunctions);

	m_methods.insert({ guiDType::activatedFunctions, &ToolBarMeshObjectGroup::onFunctionActived });
	m_methods.insert({ guiDType::projectLoaded, &ToolBarMeshObjectGroup::onProjectLoad });


}

void ToolBarMeshObjectGroup::informData(IGuiData* data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		WavefrontGroupMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarMeshObjectGroup::onProjectLoad(IGuiData* data)
{
	GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(plData->m_isProjectLoad);
}

void  ToolBarMeshObjectGroup::onFunctionActived(IGuiData* data)
{
	auto function = static_cast<GuiDataActivatedFunctions*>(data);

	m_ui.importFromFileBtn->blockSignals(true);
	m_ui.copyBtn->blockSignals(true);

	switch (function->type)
	{
	case ContextType::meshObjectCreation:
		m_ui.importFromFileBtn->setChecked(true);
		m_ui.copyBtn->setChecked(false);
		break;
	case ContextType::meshObjectDuplication:
		m_ui.importFromFileBtn->setChecked(false);
		m_ui.copyBtn->setChecked(true);
		break;
	default:
		m_ui.importFromFileBtn->setChecked(false);
		m_ui.copyBtn->setChecked(false);
	}

	m_ui.importFromFileBtn->blockSignals(false);
	m_ui.copyBtn->blockSignals(false);
}

void ToolBarMeshObjectGroup::clickWavefrontProperties()
{
	m_dataDispatcher.updateInformation(new GuiDataClippingSettingsProperties(TEXT_3DMODEL_SETTINGS_PROPERTIES_NAME, true));
}

void ToolBarMeshObjectGroup::clickFromFile()
{
	if (m_ui.importFromFileBtn->isChecked())
		m_dialog.show();
	else
		m_dataDispatcher.sendControl(new control::function::Abort());
}

void ToolBarMeshObjectGroup::clickCopy()
{
	if (m_ui.copyBtn->isChecked())
		m_dataDispatcher.sendControl(new control::meshObject::ActivateDuplicate());
	else
		m_dataDispatcher.sendControl(new control::function::Abort());
}

ToolBarMeshObjectGroup::~ToolBarMeshObjectGroup()
{
	m_dataDispatcher.unregisterObserver(this);
}