#include "gui/widgets/PropertyPoint.h"
#include "controller/Controls/ControlObject3DEdition.h"
#include "gui/widgets/CustomWidgets/GenericPropertiesHead.h"
#include "gui/widgets/CustomWidgets/GenericPropertiesFeet.h"
#include "controller/Controller.h"
#include "controller/controls/ControlDataEdition.h"
#include "controller/controls/ControlClippingEdition.h"
#include "controller/controls/ControlAnimation.h"
#include "gui/GuiData/GuiData3dObjects.h"
#include "utils/math/trigo.h"
#include <QRegExpValidator>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QtConcurrent/qtconcurrentrun.h>
#include <cctype>

#include "models/graph/PointNode.h"

PropertyPoint::PropertyPoint(Controller& controller, QWidget* parent, float guiScale)
	: APropertyGeneral(controller.getDataDispatcher(), parent)
{
	m_ui.setupUi(this);
	m_ui.genericPropsHeadWidget->setControllerInfo(controller);
	m_ui.genericPropsFeetWidget->setDataDispatcher(controller.getDataDispatcher());
	m_ui.subPropertyClipping->setDataDispatcher(&controller.getDataDispatcher());

	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendAuthorsList);

	connect(m_ui.addAsKeyPoint, SIGNAL(pressed()), this, SLOT(addAsAnimationViewPoint()));

	// Link action
	connect(m_ui.XLineEdit, &QLineEdit::editingFinished, this, &PropertyPoint::onXEdit);
	connect(m_ui.YLineEdit, &QLineEdit::editingFinished, this, &PropertyPoint::onYEdit);
	connect(m_ui.ZLineEdit, &QLineEdit::editingFinished, this, &PropertyPoint::onZEdit);

	m_ui.XLineEdit->setType(NumericType::DISTANCE);
	m_ui.YLineEdit->setType(NumericType::DISTANCE);
	m_ui.ZLineEdit->setType(NumericType::DISTANCE);


	m_ui.addAsKeyPoint->setVisible(false);


}

PropertyPoint::~PropertyPoint()
{
	m_dataDispatcher.unregisterObserver(this);
}

bool PropertyPoint::actualizeProperty(SafePtr<AGraphNode> object )
{
	if (object)
		m_point = static_pointer_cast<PointNode>(object);

	m_ui.genericPropsHeadWidget->setObject(m_point);
	m_ui.genericPropsFeetWidget->setObject(m_point);
	m_ui.subPropertyClipping->setObject(m_point);

	return updatePoint();
}

void PropertyPoint::hideEvent(QHideEvent* event)
{
	m_ui.XLineEdit->blockSignals(true);
	m_ui.YLineEdit->blockSignals(true);
	m_ui.ZLineEdit->blockSignals(true);

	QWidget::hideEvent(event);

	m_ui.XLineEdit->blockSignals(false);
	m_ui.YLineEdit->blockSignals(false);
	m_ui.ZLineEdit->blockSignals(false);
	
}

void PropertyPoint::informData(IGuiData* data)
{
	APropertyGeneral::informData(data);
	if (m_measureMethods.find(data->getType()) != m_measureMethods.end())
	{
		PropertyPointMethod method = m_measureMethods.at(data->getType());
		(this->*method)(data);
	}
}

bool PropertyPoint::updatePoint()
{
	ReadPtr<PointNode> point = m_point.cget();
	if (!point)
		return false;

	m_ui.XLineEdit->blockSignals(true);
	m_ui.YLineEdit->blockSignals(true);
	m_ui.ZLineEdit->blockSignals(true);

	m_ui.XLineEdit->setValue(point->getCenter().x);
	m_ui.YLineEdit->setValue(point->getCenter().y);
	m_ui.ZLineEdit->setValue(point->getCenter().z);

	m_ui.XLineEdit->blockSignals(false);
	m_ui.YLineEdit->blockSignals(false);
	m_ui.ZLineEdit->blockSignals(false);

	return true;
}

void PropertyPoint::changePosition()
{
	glm::dvec3 position(m_ui.XLineEdit->getValue(),
						m_ui.YLineEdit->getValue(),
						m_ui.ZLineEdit->getValue());
	m_dataDispatcher.sendControl(new control::object3DEdition::SetCenter(m_point, position));
}

void PropertyPoint::onXEdit()
{
	changePosition();
}

void PropertyPoint::onYEdit()
{
	changePosition();
}

void PropertyPoint::onZEdit()
{
	changePosition();
}

void PropertyPoint::addAsAnimationViewPoint()
{
	m_dataDispatcher.sendControl(new control::animation::AddViewPoint(m_point));
}
