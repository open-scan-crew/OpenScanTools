#include "gui/widgets/PropertyBox.h"
#include "controller/controls/ControlClippingEdition.h"
#include "controller/controls/ControlObject3DEdition.h"
#include "gui/widgets/CustomWidgets/GenericPropertiesHead.h"
#include "gui/widgets/CustomWidgets/GenericPropertiesFeet.h"
#include "controller/controls/ControlDataEdition.h"
#include "gui/widgets/FocusWatcher.h"
#include "models/3d/NodeFunctions.h"
#include "utils/math/trigo.h"

#include "models/3d/Graph/BoxNode.h"
#include "controller/Controller.h"

void PropertyBox::hideEvent(QHideEvent* event)
{
	blockAllSignals(true);

	QWidget::hideEvent(event);

	blockAllSignals(false);
}

PropertyBox::PropertyBox(Controller& controller, QWidget *parent, float guiScale)
	: APropertyGeneral(controller.getDataDispatcher(), parent)
{
	m_ui.setupUi(this);

	m_ui.genericPropsHeadWidget->setControllerInfo(controller);
	m_ui.genericPropsFeetWidget->setDataDispatcher(controller.getDataDispatcher());
	m_ui.subPropertyClipping->setDataDispatcher(&controller.getDataDispatcher());

	m_dataDispatcher.registerObserverOnKey(this, guiDType::renderValueDisplay); 
	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendAuthorsList);

	m_ui.gridBox->setVisible(false);

	// Link action
	connect(m_ui.XSizeInfield, &QLineEdit::editingFinished, this, &PropertyBox::changeSize);
	connect(m_ui.YSizeInfield, &QLineEdit::editingFinished, this, &PropertyBox::changeSize);
	connect(m_ui.ZSizeInfield, &QLineEdit::editingFinished, this, &PropertyBox::changeSize);

	m_ui.XSizeInfield->setRules(ANumericLineEdit::LineEditRules::PositiveStrict);
	m_ui.YSizeInfield->setRules(ANumericLineEdit::LineEditRules::PositiveStrict);
	m_ui.ZSizeInfield->setRules(ANumericLineEdit::LineEditRules::PositiveStrict);

	connect(m_ui.XCenterInfield, &QLineEdit::editingFinished, this, &PropertyBox::changeCenter);
	connect(m_ui.YCenterInfield, &QLineEdit::editingFinished, this, &PropertyBox::changeCenter);
	connect(m_ui.ZCenterInfield, &QLineEdit::editingFinished, this, &PropertyBox::changeCenter);
	connect(m_ui.XOrientationInfield, &QLineEdit::editingFinished, this, &PropertyBox::changeOrientation);
	connect(m_ui.YOrientationInfield, &QLineEdit::editingFinished, this, &PropertyBox::changeOrientation);
	connect(m_ui.ZOrientationInfield, &QLineEdit::editingFinished, this, &PropertyBox::changeOrientation);

	connect(m_ui.boxExpand, &QPushButton::clicked, this, &PropertyBox::onClippingExpand);
	connect(m_ui.stepModeRadioBtn, SIGNAL(pressed()), this, SLOT(onGridDivisionByStep()));
	connect(m_ui.multipleModeRadioBtn, SIGNAL(pressed()), this, SLOT(onGridDivisionByMultiple()));
	connect(m_ui.XGridInfield, &QLineEdit::editingFinished, this, &PropertyBox::changeDivision);
	connect(m_ui.YGridInfield, &QLineEdit::editingFinished, this, &PropertyBox::changeDivision);
	connect(m_ui.ZGridInfield, &QLineEdit::editingFinished, this, &PropertyBox::changeDivision);

	m_ui.XGridInfield->setRules(ANumericLineEdit::LineEditRules::PositiveStrict);
	m_ui.YGridInfield->setRules(ANumericLineEdit::LineEditRules::PositiveStrict);
	m_ui.ZGridInfield->setRules(ANumericLineEdit::LineEditRules::PositiveStrict);

	m_ui.XCenterInfield->setType(NumericType::DISTANCE);
	m_ui.YCenterInfield->setType(NumericType::DISTANCE);
	m_ui.ZCenterInfield->setType(NumericType::DISTANCE);

	m_ui.XSizeInfield->setType(NumericType::DISTANCE);
	m_ui.YSizeInfield->setType(NumericType::DISTANCE);
	m_ui.ZSizeInfield->setType(NumericType::DISTANCE);

	m_ui.XOrientationInfield->setUnit(UnitType::DEG);
	m_ui.YOrientationInfield->setUnit(UnitType::DEG);
	m_ui.ZOrientationInfield->setUnit(UnitType::DEG);

	m_ui.volumeLineEdit->setType(NumericType::VOLUME);

	//QWidget* gridX(m_ui.XGridInfield), *gridY(m_ui.YGridInfield), *gridZ(m_ui.ZGridInfield);
	//connect(m_ui.XGridInfield, &QLineEdit::textChanged, [gridX]() {gridX->setFocus(); });
	//connect(m_ui.YGridInfield, &QLineEdit::textChanged, [gridY]() {gridY->setFocus(); });
	//connect(m_ui.ZGridInfield, &QLineEdit::textChanged, [gridZ]() {gridZ->setFocus(); });
}

PropertyBox::~PropertyBox()
{
	m_dataDispatcher.unregisterObserver(this);
}

bool PropertyBox::actualizeProperty(SafePtr<AGraphNode> object)
{
	if (object)
		m_storedBox = static_pointer_cast<BoxNode>(object);

	m_ui.genericPropsHeadWidget->setObject(m_storedBox);
	m_ui.genericPropsFeetWidget->setObject(m_storedBox);
	m_ui.subPropertyClipping->setObject(m_storedBox);

	return updateBox();
}

void PropertyBox::informData(IGuiData *data)
{
	APropertyGeneral::informData(data);
	if (m_clippingMethods.find(data->getType()) != m_clippingMethods.end())
	{
		PropertyBoxMethod method = m_clippingMethods.at(data->getType());
		(this->*method)(data);
	}
}

bool PropertyBox::updateBox()
{
	blockAllSignals(true);

	m_ui.volumeLineEdit->setValue(nodeFunctions::calculateVolume(m_storedBox));

	ReadPtr<BoxNode> box = m_storedBox.cget();
	if (!box)
		return false;

	if(box->isSimpleBox())
	{
		m_ui.gridBox->setVisible(false);
		m_ui.boxExpand->setText(tr("Expand to grid"));
	}
	else
	{
		m_ui.gridBox->setVisible(true);
		m_ui.boxExpand->setText(tr("Downcast to box"));
	}

	m_ui.XGridInfield->setValue(box->getGridDivision().x);

	m_ui.YGridInfield->setValue(box->getGridDivision().y);

	m_ui.ZGridInfield->setValue(box->getGridDivision().z);

	m_ui.multipleModeRadioBtn->setChecked(box->getGridType() == GridType::ByMultiple ? true : false);
	m_ui.stepModeRadioBtn->setChecked(box->getGridType() == GridType::ByStep ? true : false);

	if (box->getGridType() == GridType::ByStep)
	{
		m_ui.YGridInfield->setType(NumericType::DISTANCE);
		m_ui.ZGridInfield->setType(NumericType::DISTANCE);
		m_ui.XGridInfield->setType(NumericType::DISTANCE);
	}
	else
	{
		m_ui.YGridInfield->resetUnit();
		m_ui.ZGridInfield->resetUnit();
		m_ui.XGridInfield->resetUnit();
	}

	m_eulers = tls::math::quat_to_euler_zyx_deg(box->getRotation(true));

	m_ui.XOrientationInfield->setValue(m_eulers.x);
	m_ui.YOrientationInfield->setValue(m_eulers.y);
	m_ui.ZOrientationInfield->setValue(m_eulers.z);

	m_ui.XCenterInfield->setValue(box->getTranslation(true).x);
	m_ui.YCenterInfield->setValue(box->getTranslation(true).y);
	m_ui.ZCenterInfield->setValue(box->getTranslation(true).z);

	m_ui.XSizeInfield->setValue(box->getScale(true).x * 2.0);
	m_ui.YSizeInfield->setValue(box->getScale(true).y * 2.0);
	m_ui.ZSizeInfield->setValue(box->getScale(true).z * 2.0);

	blockAllSignals(false);

	return true;
}

void PropertyBox::changeSize()
{
	glm::dvec3 size(m_ui.XSizeInfield->getValue() / 2.,
					m_ui.YSizeInfield->getValue() / 2.,
					m_ui.ZSizeInfield->getValue() / 2.);
	m_dataDispatcher.sendControl(new control::object3DEdition::SetSize(m_storedBox, size));
}

void PropertyBox::onSizeXEdit()
{
	ReadPtr<BoxNode> box = m_storedBox.cget();
	if (!box)
		return;

	if (box->getScale().x * 2.0 == m_ui.XSizeInfield->getValue())
		return;
	changeSize();
}

void PropertyBox::onSizeYEdit()
{
	ReadPtr<BoxNode> box = m_storedBox.cget();
	if (!box)
		return;

	if (box->getScale().y * 2.0 == m_ui.YSizeInfield->getValue())
		return;
	changeSize();
}

void PropertyBox::onSizeZEdit()
{
	ReadPtr<BoxNode> box = m_storedBox.cget();
	if (!box)
		return;

	if (box->getScale().z * 2.0 == m_ui.ZSizeInfield->getValue())
		return;
	changeSize();
}

void PropertyBox::changeOrientation()
{
	glm::dvec3 orientation(m_ui.XOrientationInfield->getValue(), m_ui.YOrientationInfield->getValue(), m_ui.ZOrientationInfield->getValue());
	m_dataDispatcher.sendControl(new control::object3DEdition::SetRotation(m_storedBox, tls::math::euler_deg_to_quat(orientation)));
}

void PropertyBox::onOrientXEdit()
{
	if (m_eulers.x == m_ui.XOrientationInfield->getValue())
		return;
	changeOrientation();
}

void PropertyBox::onOrientYEdit()
{
	if (m_eulers.y == m_ui.YOrientationInfield->getValue())
		return;
	changeOrientation();
}

void PropertyBox::onOrientZEdit()
{
	if (m_eulers.z == m_ui.ZOrientationInfield->getValue())
		return;
	changeOrientation();
}

void  PropertyBox::changeCenter()
{
	glm::dvec3 position(m_ui.XCenterInfield->getValue(),
						m_ui.YCenterInfield->getValue(),
						m_ui.ZCenterInfield->getValue());
	m_dataDispatcher.sendControl(new control::object3DEdition::SetCenter(m_storedBox, position));
}

void PropertyBox::onCenterXEdit()
{
	ReadPtr<BoxNode> box = m_storedBox.cget();
	if (!box)
		return;

	if (box->getCenter().x == m_ui.XCenterInfield->getValue())
		return;
	changeCenter();
}

void PropertyBox::onCenterYEdit()
{
	ReadPtr<BoxNode> box = m_storedBox.cget();
	if (!box)
		return;

	if (box->getCenter().y == m_ui.YCenterInfield->getValue())
		return;
	changeCenter();
}

void PropertyBox::onCenterZEdit()
{
	ReadPtr<BoxNode> box = m_storedBox.cget();
	if (!box)
		return;

	if (box->getCenter().z == m_ui.ZCenterInfield->getValue())
		return;
	changeCenter();
}

void PropertyBox::onClippingExpand()
{
	ReadPtr<BoxNode> box = m_storedBox.cget();
	if (!box)
		return;

	m_dataDispatcher.sendControl(new control::clippingEdition::SwitchBoxGrid(m_storedBox, !box->isSimpleBox()));
}

void PropertyBox::onGridDivisionByStep()
{
	m_dataDispatcher.sendControl(new control::clippingEdition::GridDivisionType(m_storedBox, GridType::ByStep));
}

void PropertyBox::onGridDivisionByMultiple()
{
	m_dataDispatcher.sendControl(new control::clippingEdition::GridDivisionType(m_storedBox, GridType::ByMultiple));
}

void PropertyBox::changeDivision()
{
	glm::vec3 division(m_ui.XGridInfield->getValue(),
						m_ui.YGridInfield->getValue(), 
						m_ui.ZGridInfield->getValue());

	m_dataDispatcher.sendControl(new control::clippingEdition::GridChangeValue(m_storedBox, division));
}

void PropertyBox::blockGridSignals(bool value)
{
	m_ui.stepModeRadioBtn->blockSignals(value);
	m_ui.multipleModeRadioBtn->blockSignals(value);
	m_ui.XGridInfield->blockSignals(value);
	m_ui.YGridInfield->blockSignals(value);
	m_ui.ZGridInfield->blockSignals(value);
}

void PropertyBox::blockClippingSignals(bool value)
{
	m_ui.XCenterInfield->blockSignals(value);
	m_ui.YCenterInfield->blockSignals(value);
	m_ui.ZCenterInfield->blockSignals(value);
	m_ui.XSizeInfield->blockSignals(value);
	m_ui.YSizeInfield->blockSignals(value);
	m_ui.ZSizeInfield->blockSignals(value);
	m_ui.XOrientationInfield->blockSignals(value);
	m_ui.YOrientationInfield->blockSignals(value);
	m_ui.ZOrientationInfield->blockSignals(value);
}

void PropertyBox::blockAllSignals(bool value)
{
	blockClippingSignals(value);
	blockGridSignals(value);
}

void PropertyBox::onGridXEdit()
{
	changeDivision();
}

void PropertyBox::onGridYEdit()
{
	changeDivision();
}

void PropertyBox::onGridZEdit()
{
	changeDivision();
}
