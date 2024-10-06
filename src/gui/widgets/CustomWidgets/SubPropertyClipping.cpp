#include "gui/widgets/CustomWidgets/SubPropertyClipping.h"
#include "controller/controls/ControlClippingEdition.h"

#include "models/graph/AClippingNode.h"
#include "controller/Controller.h"

#include "models/LicenceTypes.h"

void SubPropertyClipping::hideEvent(QHideEvent* event)
{
	blockSignals(true);

	QWidget::hideEvent(event);

	blockSignals(false);
}

SubPropertyClipping::SubPropertyClipping(QWidget *parent, float guiScale)
	: QWidget(parent)
{
	m_ui.setupUi(this);

	connect(m_ui.group_clip, &QGroupBox::clicked, this, &SubPropertyClipping::onActiveClipping);
	connect(m_ui.InteriorModeRadioBtn, SIGNAL(pressed()), this, SLOT(onShowInteriorClick()));
	connect(m_ui.ExteriorModeRadioBtn, SIGNAL(pressed()), this, SLOT(onShowExteriorClick()));
	connect(m_ui.lineEdit_minClip, &QLineEdit::editingFinished, this, &SubPropertyClipping::onMinClipDistEdit);
	connect(m_ui.lineEdit_maxClip, &QLineEdit::editingFinished, this, &SubPropertyClipping::onMaxClipDistEdit);

	connect(m_ui.group_ramp, &QGroupBox::clicked, this, &SubPropertyClipping::onRampActive);
	connect(m_ui.lineEdit_minRamp, &QLineEdit::editingFinished, this, &SubPropertyClipping::onRampMin);
	connect(m_ui.lineEdit_maxRamp, &QLineEdit::editingFinished, this, &SubPropertyClipping::onRampMax);
	connect(m_ui.spinBox_stepsRamp, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &SubPropertyClipping::onRampSteps);
	connect(m_ui.checkBox_clamp, &QCheckBox::stateChanged, this, &SubPropertyClipping::onRampClamp);

	m_ui.lineEdit_minClip->setRules(ANumericLineEdit::LineEditRules::Nothing);
	m_ui.lineEdit_minClip->setType(NumericType::DISTANCE);

	m_ui.lineEdit_maxClip->setRules(ANumericLineEdit::LineEditRules::Nothing);
	m_ui.lineEdit_maxClip->setType(NumericType::DISTANCE);

	m_ui.lineEdit_minRamp->setRules(ANumericLineEdit::LineEditRules::Nothing);
	m_ui.lineEdit_minRamp->setType(NumericType::DISTANCE);

	m_ui.lineEdit_maxRamp->setRules(ANumericLineEdit::LineEditRules::Nothing);
	m_ui.lineEdit_maxRamp->setType(NumericType::DISTANCE);
}

SubPropertyClipping::~SubPropertyClipping()
{
}

void SubPropertyClipping::setObject(const SafePtr<AClippingNode>& object)
{
	if (object)
		m_storedClip = object;
	{
		ReadPtr<AClippingNode> rNode = m_storedClip.cget();
		if (rNode)
			prepareUi(rNode->getType());
	}

	update();
}

void SubPropertyClipping::setDataDispatcher(IDataDispatcher* dataDispatcher)
{
	m_dataDispatcher = dataDispatcher;
}

void SubPropertyClipping::update()
{
	blockSignals(true);

	ReadPtr<AClippingNode> rClip = m_storedClip.cget();
	if (!rClip)
		return;

	m_ui.group_clip->setChecked(rClip->isClippingActive());
	m_ui.InteriorModeRadioBtn->setChecked(rClip->getClippingMode() == ClippingMode::showInterior ? true : false);
	m_ui.ExteriorModeRadioBtn->setChecked(rClip->getClippingMode() == ClippingMode::showExterior ? true : false);

	m_ui.lineEdit_minClip->setValue(rClip->getMinClipDist());
	m_ui.lineEdit_maxClip->setValue(rClip->getMaxClipDist());

	m_ui.group_ramp->setChecked(rClip->isRampActive());
	m_ui.lineEdit_minRamp->setValue(rClip->getRampMin());
	m_ui.lineEdit_maxRamp->setValue(rClip->getRampMax());
	m_ui.spinBox_stepsRamp->setValue(rClip->getRampSteps());
	m_ui.checkBox_clamp->setChecked(rClip->isRampClamped());

	blockSignals(false);

	return;
}

void SubPropertyClipping::prepareUi(ElementType type)
{
	m_ui.InteriorModeRadioBtn->setEnabled(true);
	m_ui.ExteriorModeRadioBtn->setEnabled(true);

	bool isBoxOrGrid = (type == ElementType::Box || type == ElementType::Grid);

	m_ui.label_minClip->setVisible(!isBoxOrGrid);
	m_ui.lineEdit_minClip->setVisible(!isBoxOrGrid);
	m_ui.label_maxClip->setVisible(!isBoxOrGrid);
	m_ui.lineEdit_maxClip->setVisible(!isBoxOrGrid);

	m_ui.lineEdit_minRamp->setVisible(!isBoxOrGrid);
	m_ui.label_minRamp->setVisible(!isBoxOrGrid);
	m_ui.lineEdit_maxRamp->setVisible(!isBoxOrGrid);
	m_ui.label_maxRamp->setVisible(!isBoxOrGrid);

	m_ui.checkBox_clamp->setVisible(isBoxOrGrid);
	m_ui.checkBox_clamp->setVisible(isBoxOrGrid);

	m_ui.InteriorModeRadioBtn->setEnabled(type != ElementType::Grid && m_ui.group_clip->isChecked());
	m_ui.ExteriorModeRadioBtn->setEnabled(type != ElementType::Grid && m_ui.group_clip->isChecked());
}

void SubPropertyClipping::onShowInteriorClick()
{
	if (m_dataDispatcher)
		m_dataDispatcher->sendControl(new control::clippingEdition::SetMode(m_storedClip, ClippingMode::showInterior));
}

void SubPropertyClipping::onShowExteriorClick()
{
	if (m_dataDispatcher)
		m_dataDispatcher->sendControl(new control::clippingEdition::SetMode(m_storedClip, ClippingMode::showExterior));
}

void SubPropertyClipping::onActiveClipping()
{
	if(m_dataDispatcher)
		m_dataDispatcher->sendControl(new control::clippingEdition::SetClipActive(m_storedClip, m_ui.group_clip->isChecked()));
}

void SubPropertyClipping::onMinClipDistEdit()
{
	if (m_dataDispatcher)
		m_dataDispatcher->sendControl(new control::clippingEdition::SetMinClipDist(m_storedClip, m_ui.lineEdit_minClip->getValue()));
}

void SubPropertyClipping::onMaxClipDistEdit()
{
	if (m_dataDispatcher)
		m_dataDispatcher->sendControl(new control::clippingEdition::SetMaxClipDist(m_storedClip, m_ui.lineEdit_maxClip->getValue()));
}

void SubPropertyClipping::onRampActive(bool active)
{
	if (m_dataDispatcher)
		m_dataDispatcher->sendControl(new control::clippingEdition::SetRampActive(m_storedClip, active));
}

void SubPropertyClipping::onRampMin()
{
	if (m_dataDispatcher)
		m_dataDispatcher->sendControl(new control::clippingEdition::SetMinRamp(m_storedClip, m_ui.lineEdit_minRamp->getValue()));
}

void SubPropertyClipping::onRampMax()
{
	if (m_dataDispatcher)
		m_dataDispatcher->sendControl(new control::clippingEdition::SetMaxRamp(m_storedClip, m_ui.lineEdit_maxRamp->getValue()));
}

void SubPropertyClipping::onRampSteps(int value)
{
	if (m_dataDispatcher)
		m_dataDispatcher->sendControl(new control::clippingEdition::SetRampSteps(m_storedClip, m_ui.spinBox_stepsRamp->value()));
}

void SubPropertyClipping::onRampClamp(int value)
{
	if (m_dataDispatcher)
		m_dataDispatcher->sendControl(new control::clippingEdition::SetRampClamped(m_storedClip, m_ui.checkBox_clamp->isChecked()));
}

void SubPropertyClipping::blockSignals(bool value)
{
	m_ui.InteriorModeRadioBtn->blockSignals(value);
	m_ui.ExteriorModeRadioBtn->blockSignals(value);
	m_ui.group_clip->blockSignals(value);
	m_ui.lineEdit_minClip->blockSignals(value);
	m_ui.lineEdit_maxClip->blockSignals(value);

	m_ui.group_ramp->blockSignals(value);
	m_ui.lineEdit_minRamp->blockSignals(value);
	m_ui.lineEdit_maxRamp->blockSignals(value);
	m_ui.spinBox_stepsRamp->blockSignals(value);
	m_ui.checkBox_clamp->blockSignals(value);
}