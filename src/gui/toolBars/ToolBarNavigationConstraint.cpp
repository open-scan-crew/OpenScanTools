#include "gui\toolBars\ToolBarNavigationConstraint.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "models/3d/NavigationTypes.h"



ToolBarNavigationConstraint::ToolBarNavigationConstraint(IDataDispatcher &dataDispatcher, QWidget *parent, float guiScale)
	: QWidget(parent)
	, m_dataDispatcher(dataDispatcher)
{
	m_ui.setupUi(this);

	m_ui.lockHorizontalRadio->setEnabled(false);
	m_ui.lockVerticalRadio->setEnabled(false);
	m_ui.lockZRadio->setEnabled(false);
	m_ui.lockYRadio->setEnabled(false);
	m_ui.lockXRadio->setEnabled(false);
	setEnabled(false);

	connect(m_ui.constraintBox, &QCheckBox::clicked, this, &ToolBarNavigationConstraint::slotApplyConstraint);
	connect(m_ui.lockHorizontalRadio, &QRadioButton::released, this, [this] { this->slotLock(NaviConstraint::LockHorizontal); });
	connect(m_ui.lockVerticalRadio, &QRadioButton::released, this, [this] { this->slotLock(NaviConstraint::LockVertical); });
	connect(m_ui.lockZRadio, &QRadioButton::released, this, [this] { this->slotLock(NaviConstraint::LockZValue); });
	connect(m_ui.lockYRadio, &QRadioButton::released, this, [this] { this->slotLock(NaviConstraint::LockYValue); });
	connect(m_ui.lockXRadio, &QRadioButton::released, this, [this] { this->slotLock(NaviConstraint::LockXValue); });

	m_dataDispatcher.registerObserverOnKey(this, guiDType::projectLoaded);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::focusViewport);

	m_methods.insert({ guiDType::projectLoaded, &ToolBarNavigationConstraint::onProjectLoad });
	m_methods.insert({ guiDType::focusViewport, &ToolBarNavigationConstraint::onFocusViewport });
}

ToolBarNavigationConstraint::~ToolBarNavigationConstraint()
{
	m_dataDispatcher.unregisterObserver(this);
}

void ToolBarNavigationConstraint::onProjectLoad(IGuiData* data)
{
	auto* pldata = static_cast<GuiDataProjectLoaded*>(data);
	setEnabled(pldata->m_isProjectLoad);
}

void ToolBarNavigationConstraint::onFocusViewport(IGuiData* data)
{
	GuiDataFocusViewport* castData = static_cast<GuiDataFocusViewport*>(data);
	if (castData->m_forceFocus && castData->m_camera)
		m_focusCamera = castData->m_camera;
}

/*
void ToolBarNavigationConstraint::onProjectionModeChanged(IGuiData * data)
{
	auto* projMode = static_cast<GuiDataRenderProjectionMode*>(data);
	m_ui.constraintBox->blockSignals(true);
	m_ui.lockHorizontalRadio->blockSignals(true);
	m_ui.lockVerticalRadio->blockSignals(true);
	m_ui.lockZRadio->blockSignals(true);
	if (projMode->m_mode == ProjectionMode::Orthographic) {
		m_ui.lockHorizontalRadio->setEnabled(false);
		m_ui.lockVerticalRadio->setEnabled(false);
		m_ui.lockZRadio->setEnabled(false);
		m_ui.constraintBox->setEnabled(false);

		m_dataDispatcher.updateInformation(new GuiDataRenderApplyNavigationConstraint(false));
	}
	else {
		m_ui.constraintBox->setEnabled(true);

		applyConstraint(m_ui.constraintBox->isChecked());
	}

	m_ui.lockHorizontalRadio->blockSignals(false);
	m_ui.lockVerticalRadio->blockSignals(false);
	m_ui.lockZRadio->blockSignals(false);
	m_ui.constraintBox->blockSignals(false);

}
*/

void ToolBarNavigationConstraint::informData(IGuiData *data)
{
	if (m_methods.find(data->getType()) != m_methods.end())
	{
		constraintToolBarMethod method = m_methods.at(data->getType());
		(this->*method)(data);
	}
}

void ToolBarNavigationConstraint::applyConstraint(bool checked) {
	m_ui.lockHorizontalRadio->blockSignals(true);
	m_ui.lockVerticalRadio->blockSignals(true);
	m_ui.lockZRadio->blockSignals(true);
	m_ui.lockYRadio->blockSignals(true);
	m_ui.lockXRadio->blockSignals(true);

	if (checked) {
		m_ui.lockHorizontalRadio->setEnabled(true);
		m_ui.lockVerticalRadio->setEnabled(true);
		m_ui.lockZRadio->setEnabled(true);
		m_ui.lockYRadio->setEnabled(true);
		m_ui.lockXRadio->setEnabled(true);
	}
	else {
		m_ui.lockHorizontalRadio->setEnabled(false);
		m_ui.lockVerticalRadio->setEnabled(false);
		m_ui.lockZRadio->setEnabled(false);
		m_ui.lockYRadio->setEnabled(false);
		m_ui.lockXRadio->setEnabled(false);
	}

	m_ui.lockHorizontalRadio->blockSignals(false);
	m_ui.lockVerticalRadio->blockSignals(false);
	m_ui.lockZRadio->blockSignals(false);
	m_ui.lockYRadio->blockSignals(false);
	m_ui.lockXRadio->blockSignals(false);

	m_dataDispatcher.updateInformation(new GuiDataRenderApplyNavigationConstraint(checked, m_focusCamera));
}

void ToolBarNavigationConstraint::slotApplyConstraint(bool checked)
{
	applyConstraint(checked);
}

void ToolBarNavigationConstraint::slotLock(NaviConstraint constraint)
{
	m_dataDispatcher.updateInformation(new GuiDataRenderNavigationConstraint(constraint, m_focusCamera));
}