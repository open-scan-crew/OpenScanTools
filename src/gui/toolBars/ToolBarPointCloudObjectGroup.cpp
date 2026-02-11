#include "controller/controls/ControlPCObject.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataClipping.h"
#include "gui/GuiData/GuiDataIO.h"
#include "gui/toolBars/ToolBarPointCloudObjectGroup.h"
#include "controller/controls/ControlFunction.h"

#include "gui/Texts.hpp"

#include <QtCore/qstandardpaths.h>

ToolBarPointCloudObjectGroup::ToolBarPointCloudObjectGroup(IDataDispatcher& dataDispatcher, QWidget* parent, float guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
	, m_openPath(QStandardPaths::locate(QStandardPaths::DocumentsLocation, QString(), QStandardPaths::LocateDirectory))
	, m_dialog(dataDispatcher, parent)
{
	m_ui.setupUi(this);
	setEnabled(false);
	m_ui.importFromBoxBtn->setIconSize(QSize(20, 20) * guiScale);
	m_ui.creationSettingsBtn->setIconSize(QSize(20, 20) * guiScale);
	m_ui.copyPCOBtn->setIconSize(QSize(20, 20) * guiScale);
	m_ui.importFromFileBtn->setIconSize(QSize(20, 20) * guiScale);

	//m_ui.importFromBoxBtn->setEnabled(false);
	//m_ui.creationSettingsBtn->setEnabled(false);
	//m_ui.importFromFileBtn->setEnabled(false);
	//m_ui.copyPCOBtn->setEnabled(false);

	QObject::connect(m_ui.importFromBoxBtn, &QToolButton::clicked, this, &ToolBarPointCloudObjectGroup::clickFromBox);
	QObject::connect(m_ui.creationSettingsBtn, &QToolButton::clicked, this, &ToolBarPointCloudObjectGroup::clickPointCloudObjectProperties);
	QObject::connect(m_ui.copyPCOBtn, &QToolButton::clicked, this, &ToolBarPointCloudObjectGroup::clickCopy);
	QObject::connect(m_ui.importFromFileBtn, &QToolButton::clicked, this, &ToolBarPointCloudObjectGroup::clickFromFile);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::activatedFunctions);

	m_methods.insert({ guiDType::activatedFunctions, &ToolBarPointCloudObjectGroup::onFunctionActived });
	m_methods.insert({ guiDType::projectLoaded, &ToolBarPointCloudObjectGroup::onProjectLoad });
	m_methods.insert({ guiDType::projectPath, &ToolBarPointCloudObjectGroup::onProjectPath });

}

void ToolBarPointCloudObjectGroup::informData(IGuiData* data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		PCOGroupMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarPointCloudObjectGroup::onProjectLoad(IGuiData* data)
{
	GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(plData->m_isProjectLoad);
	m_ui.importFromBoxBtn->setEnabled(true);
	m_ui.copyPCOBtn->setEnabled(true);
	m_ui.importFromFileBtn->setEnabled(true);
}

void  ToolBarPointCloudObjectGroup::onFunctionActived(IGuiData* data)
{
	auto function = static_cast<GuiDataActivatedFunctions*>(data);

	m_ui.importFromBoxBtn->blockSignals(true);
	m_ui.copyPCOBtn->blockSignals(true);
	m_ui.importFromFileBtn->blockSignals(true);

	switch (function->type)
	{
	case ContextType::pointCloudObjectCreation:
		m_ui.importFromBoxBtn->setChecked(true);
		m_ui.copyPCOBtn->setChecked(false);
		m_ui.importFromFileBtn->setChecked(false);
		break;
		
	case ContextType::pointCloudObjectDuplication:
		m_ui.importFromBoxBtn->setChecked(false);
		m_ui.copyPCOBtn->setChecked(true);
		m_ui.importFromFileBtn->setChecked(false);
		break;
	default:
		m_ui.importFromBoxBtn->setChecked(false);
		m_ui.copyPCOBtn->setChecked(false);
		m_ui.importFromFileBtn->setChecked(false);
	}

	m_ui.importFromFileBtn->blockSignals(false);
	m_ui.copyPCOBtn->blockSignals(false);
	m_ui.importFromBoxBtn->blockSignals(false);
}

void ToolBarPointCloudObjectGroup::onProjectPath(IGuiData* data)
{
	auto dataType = static_cast<GuiDataProjectPath*>(data);
	m_openPath = QString::fromStdString(dataType->m_path.string());
}

void ToolBarPointCloudObjectGroup::clickPointCloudObjectProperties()
{
	m_dataDispatcher.updateInformation(new GuiDataClippingSettingsProperties(TEXT_PCO_SETTINGS_PROPERTIES_NAME, true));
}

void ToolBarPointCloudObjectGroup::clickFromBox()
{
	if (m_ui.importFromBoxBtn->isChecked())
		m_dataDispatcher.sendControl(new control::pcObject::CreatePCObjectFromBoxActivate());
	else
		m_dataDispatcher.sendControl(new control::function::Abort());
}

void ToolBarPointCloudObjectGroup::clickFromFile()
{
	m_dialog.show();
	m_ui.importFromFileBtn->setChecked(false);
}

void ToolBarPointCloudObjectGroup::clickCopy()
{
	if (m_ui.copyPCOBtn->isChecked())
		m_dataDispatcher.sendControl(new control::pcObject::ActivateDuplicate());
	else
		m_dataDispatcher.sendControl(new control::function::Abort());
}

ToolBarPointCloudObjectGroup::~ToolBarPointCloudObjectGroup()
{
	m_dataDispatcher.unregisterObserver(this);
}