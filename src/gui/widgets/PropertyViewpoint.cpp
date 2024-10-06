#include "gui/widgets/PropertyViewpoint.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "controller/Controller.h"
#include "controller/controls/ControlObject3DEdition.h"
#include "controller/controls/ControlClippingEdition.h"
#include "gui/widgets/CustomWidgets/GenericPropertiesHead.h"
#include "gui/widgets/CustomWidgets/GenericPropertiesFeet.h"
#include "controller/controls/ControlDataEdition.h"
#include "controller/controls/ControlViewPoint.h"
#include "gui/GuiData/GuiData3dObjects.h"
#include "models/3d/ManipulationTypes.h"
#include "utils/math/trigo.h"

#include "models/graph/ViewPointNode.h"

PropertyViewpoint::PropertyViewpoint(Controller& controller, QWidget* parent, float guiScale)
	: APropertyGeneral(controller.getDataDispatcher(), parent)
{
	m_ui.setupUi(this);

	m_ui.genericPropsHeadWidget->setControllerInfo(controller);
	m_ui.genericPropsFeetWidget->setDataDispatcher(controller.getDataDispatcher());

	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendAuthorsList);

	// Link action
	connect(m_ui.XCenterInfield, &QLineEdit::editingFinished, this, &PropertyViewpoint::onCenterXEdit);
	connect(m_ui.YCenterInfield, &QLineEdit::editingFinished, this, &PropertyViewpoint::onCenterYEdit);
	connect(m_ui.ZCenterInfield, &QLineEdit::editingFinished, this, &PropertyViewpoint::onCenterZEdit);
	connect(m_ui.XOrientInfield, &QLineEdit::editingFinished, this, &PropertyViewpoint::onOrientXEdit);
	connect(m_ui.YOrientInfield, &QLineEdit::editingFinished, this, &PropertyViewpoint::onOrientYEdit);
	connect(m_ui.ZOrientInfield, &QLineEdit::editingFinished, this, &PropertyViewpoint::onOrientZEdit);
	connect(m_ui.updateToolButton, &QToolButton::released, this, &PropertyViewpoint::onUpdateToolButton);


	m_ui.XCenterInfield->setType(NumericType::DISTANCE);
	m_ui.YCenterInfield->setType(NumericType::DISTANCE);
	m_ui.ZCenterInfield->setType(NumericType::DISTANCE);

	m_ui.XOrientInfield->setUnit(UnitType::DEG);
	m_ui.YOrientInfield->setUnit(UnitType::DEG);
	m_ui.ZOrientInfield->setUnit(UnitType::DEG);
}

PropertyViewpoint::~PropertyViewpoint()
{
	m_dataDispatcher.unregisterObserver(this);
}

bool PropertyViewpoint::actualizeProperty(SafePtr<AGraphNode> object )
{
	if (object)
		m_object = static_pointer_cast<ViewPointNode>(object);

	m_ui.genericPropsHeadWidget->setObject(m_object);
	m_ui.genericPropsFeetWidget->setObject(m_object);

	return setViewpointParameters();
}

void PropertyViewpoint::hideEvent(QHideEvent* event)
{
	m_ui.XCenterInfield->blockSignals(true);
	m_ui.YCenterInfield->blockSignals(true);
	m_ui.ZCenterInfield->blockSignals(true);
	m_ui.XOrientInfield->blockSignals(true);
	m_ui.YOrientInfield->blockSignals(true);
	m_ui.ZOrientInfield->blockSignals(true);

	QWidget::hideEvent(event);

	m_ui.XCenterInfield->blockSignals(false);
	m_ui.YCenterInfield->blockSignals(false);
	m_ui.ZCenterInfield->blockSignals(false);
	m_ui.XOrientInfield->blockSignals(false);
	m_ui.YOrientInfield->blockSignals(false);
	m_ui.ZOrientInfield->blockSignals(false);
}

void PropertyViewpoint::informData(IGuiData* data)
{
	APropertyGeneral::informData(data);
	if (m_viewpointMethods.find(data->getType()) != m_viewpointMethods.end())
	{
		PropertyViewpointMethod method = m_viewpointMethods.at(data->getType());
		(this->*method)(data);
	}
}

void PropertyViewpoint::setObject3DParameters(const TransformationModule& data)
{
	m_ui.XCenterInfield->blockSignals(true);
	m_ui.YCenterInfield->blockSignals(true);
	m_ui.ZCenterInfield->blockSignals(true);
	m_ui.XOrientInfield->blockSignals(true);
	m_ui.YOrientInfield->blockSignals(true);
	m_ui.ZOrientInfield->blockSignals(true);

	m_eulers = tls::math::quat_to_euler_zyx_deg(data.getOrientation());

	m_ui.XCenterInfield->setValue(data.getCenter().x);
	m_ui.YCenterInfield->setValue(data.getCenter().y);
	m_ui.ZCenterInfield->setValue(data.getCenter().z);

	m_ui.XOrientInfield->setValue(m_eulers.x);
	m_ui.YOrientInfield->setValue(m_eulers.y);
	m_ui.ZOrientInfield->setValue(m_eulers.z);


	m_ui.XCenterInfield->blockSignals(false);
	m_ui.YCenterInfield->blockSignals(false);
	m_ui.ZCenterInfield->blockSignals(false);
	m_ui.XOrientInfield->blockSignals(false);
	m_ui.YOrientInfield->blockSignals(false);
	m_ui.ZOrientInfield->blockSignals(false);
}

bool PropertyViewpoint::setViewpointParameters()
{
	ReadPtr<ViewPointNode> node = m_object.cget();
	if (!node)
		return false;

	setObject3DParameters(*&node);

	return true;
}

void PropertyViewpoint::changeOrientation()
{
	glm::dvec3 orientation(m_ui.XOrientInfield->getValue(), m_ui.YOrientInfield->getValue(), m_ui.ZOrientInfield->getValue());
	m_dataDispatcher.sendControl(new control::object3DEdition::SetRotation(m_object, tls::math::euler_deg_to_quat(orientation)));
}

void PropertyViewpoint::onOrientXEdit()
{
	if (m_eulers.x == m_ui.XOrientInfield->getValue())
		return;
	changeOrientation();
}

void PropertyViewpoint::onOrientYEdit()
{
	if (m_eulers.y == m_ui.YOrientInfield->getValue())
		return;
	changeOrientation();
}

void PropertyViewpoint::onOrientZEdit()
{
	if (m_eulers.z == m_ui.ZOrientInfield->getValue())
		return;
	changeOrientation();
}

void  PropertyViewpoint::changeCenter()
{
	glm::dvec3 position(m_ui.XCenterInfield->getValue(), 
						m_ui.YCenterInfield->getValue(), 
						m_ui.ZCenterInfield->getValue());
	m_dataDispatcher.sendControl(new control::object3DEdition::SetCenter(m_object, position));
}

void PropertyViewpoint::onCenterXEdit()
{
	changeCenter();
}

void PropertyViewpoint::onCenterYEdit()
{
	changeCenter();
}

void PropertyViewpoint::onCenterZEdit()
{
	changeCenter();
}

void PropertyViewpoint::onUpdateToolButton()
{
	m_dataDispatcher.sendControl(new control::viewpoint::LaunchUpdateContext(m_object));
}
