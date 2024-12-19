#include "gui/toolBars/ToolBarTagGroup.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "controller/controls/ControlFunctionTag.h"
#include "controller/controls/ControlFunction.h"

#include <QtWidgets/qwidget.h>

ToolBarTagGroup::ToolBarTagGroup(IDataDispatcher &dataDispatcher, QWidget *parent, const float& guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
{
	m_ui.setupUi(this);
	setEnabled(false);

    m_ui.copyTagButton->setIconSize(QSize(20, 20) * guiScale);
    m_ui.createTagButton->setIconSize(QSize(20, 20) * guiScale);
    m_ui.moveTagButton->setIconSize(QSize(20, 20) * guiScale);

	QObject::connect(m_ui.createTagButton, &QToolButton::released, this, &ToolBarTagGroup::clickCreatetag);
	QObject::connect(m_ui.moveTagButton, &QToolButton::released, this, &ToolBarTagGroup::clickMoveTag);
	QObject::connect(m_ui.copyTagButton, &QToolButton::released, this, &ToolBarTagGroup::clickDuplicateTag);

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::activatedFunctions);

	methods.insert({ guiDType::projectLoaded, &ToolBarTagGroup::onProjectLoad });
	methods.insert({ guiDType::activatedFunctions, &ToolBarTagGroup::onActivatedFunctions });


}

ToolBarTagGroup::~ToolBarTagGroup()
{
    m_dataDispatcher.unregisterObserver(this);
}

void ToolBarTagGroup::informData(IGuiData *data)
{
	if (methods.find(data->getType()) != methods.end())
	{
		TagGroupMethod method = methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarTagGroup::onProjectLoad(IGuiData *data)
{
    GuiDataProjectLoaded* plData = static_cast<GuiDataProjectLoaded*>(data);
    setEnabled(plData->m_isProjectLoad);
	m_ui.createTagButton->setEnabled(true);
	m_ui.moveTagButton->setEnabled(true);
	m_ui.copyTagButton->setEnabled(true);
}

void ToolBarTagGroup::onActivatedFunctions(IGuiData *data)
{
	m_ui.createTagButton->blockSignals(true);
	m_ui.moveTagButton->blockSignals(true);
	m_ui.copyTagButton->blockSignals(true);
	GuiDataActivatedFunctions* function = static_cast<GuiDataActivatedFunctions*>(data);
	switch (function->type)
	{
	case ContextType::tagCreation:	
		m_ui.createTagButton->setChecked(true);
		m_ui.moveTagButton->setChecked(false);
		m_ui.copyTagButton->setChecked(false);
		break;
	case ContextType::tagDuplication:
		m_ui.createTagButton->setChecked(false);
		m_ui.moveTagButton->setChecked(false);
		m_ui.copyTagButton->setChecked(true);
		break;
	case ContextType::tagMove:
		m_ui.createTagButton->setChecked(false);
		m_ui.moveTagButton->setChecked(true);
		m_ui.copyTagButton->setChecked(false);
		break;
	default:	
		m_ui.createTagButton->setChecked(false);
		m_ui.moveTagButton->setChecked(false);
		m_ui.copyTagButton->setChecked(false);
	}

	m_ui.createTagButton->blockSignals(false);
	m_ui.moveTagButton->blockSignals(false);
	m_ui.copyTagButton->blockSignals(false);
}

void ToolBarTagGroup::clickCreatetag()
{
	if (m_ui.createTagButton->isChecked())
		m_dataDispatcher.sendControl(new control::function::tag::ActivateCreate());
	else
		m_dataDispatcher.sendControl(new control::function::Abort());
}

void ToolBarTagGroup::clickMoveTag()
{
	if (m_ui.moveTagButton->isChecked())
        m_dataDispatcher.sendControl(new control::function::tag::ActivateMove());
	else
		m_dataDispatcher.sendControl(new control::function::Abort());
}

void ToolBarTagGroup::clickDuplicateTag()
{
	if (m_ui.copyTagButton->isChecked())
		m_dataDispatcher.sendControl(new control::function::tag::ActivateDuplicate());
	else
        m_dataDispatcher.sendControl(new control::function::Abort());
}