#include "gui/widgets/PropertySphere.h"
#include "gui/widgets/CustomWidgets/GenericPropertiesHead.h"
#include "gui/widgets/CustomWidgets/GenericPropertiesFeet.h"
#include "controller/Controller.h"
#include "controller/controls/ControlClippingEdition.h"
#include "controller/controls/ControlObject3DEdition.h"
#include "models/3d/NodeFunctions.h"
#include "gui/GuiData/GuiDataList.h"
#include "utils/math/trigo.h"
#include <QRegExpValidator>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QtConcurrent/qtconcurrentrun.h>
#include <cctype>

#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>

#include "models/3d/Graph/SphereNode.h"

PropertySphere::PropertySphere(Controller& controller, QWidget* parent, float guiScale)
	: APropertyGeneral(controller.getDataDispatcher(), parent)
	, m_sphere()
{
	m_ui.setupUi(this);

	m_ui.genericPropsHeadWidget->setControllerInfo(controller);
	m_ui.genericPropsFeetWidget->setDataDispatcher(controller.getDataDispatcher());
	m_ui.subPropertyClipping->setDataDispatcher(&controller.getDataDispatcher());

	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendListsStandards);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendAuthorsList);

	connect(m_ui.diameterInputField, &QDoubleEdit::editingFinished, this, &PropertySphere::onDiameterChange);

	m_ui.XCenterInfield->setType(NumericType::DISTANCE);
	m_ui.YCenterInfield->setType(NumericType::DISTANCE);
	m_ui.ZCenterInfield->setType(NumericType::DISTANCE);

	m_ui.diameterInputField->setType(NumericType::DIAMETER);

	m_ui.volumeDoubleEdit->setType(NumericType::VOLUME);
}

PropertySphere::~PropertySphere()
{
	m_dataDispatcher.unregisterObserver(this);
}

bool PropertySphere::actualizeProperty(SafePtr<AGraphNode> object )
{
	if (object)
		m_sphere = static_pointer_cast<SphereNode>(object);

	m_ui.genericPropsHeadWidget->setObject(m_sphere);
	m_ui.genericPropsFeetWidget->setObject(m_sphere);
	m_ui.subPropertyClipping->setObject(m_sphere);

	return updateSphere();
}

void PropertySphere::informData(IGuiData* data)
{
	APropertyGeneral::informData(data);
	if (m_sphereMethods.find(data->getType()) != m_sphereMethods.end())
	{
		PropertySphereMethod method = m_sphereMethods.at(data->getType());
		(this->*method)(data);
	}
}

bool PropertySphere::updateSphere()
{
	m_ui.volumeDoubleEdit->setValue(nodeFunctions::calculateVolume(m_sphere));

	ReadPtr<SphereNode> sphere = m_sphere.cget();
	if (!sphere)
		return false;

	setObject3DParameters(*&sphere);
	m_ui.diameterInputField->setValue(sphere->getRadius() * 2.0);

	return true;
}

void PropertySphere::setObject3DParameters(const TransformationModule& data)
{
	m_ui.XCenterInfield->setValue(data.getCenter().x);
	m_ui.YCenterInfield->setValue(data.getCenter().y);
	m_ui.ZCenterInfield->setValue(data.getCenter().z);
}

void PropertySphere::onDiameterChange()
{
	PANELLOG << "on diameter edit" << LOGENDL;
	double diameter(m_ui.diameterInputField->getValue());
	m_dataDispatcher.sendControl(new control::object3DEdition::SetSphereRadius(m_sphere, diameter / 2.0));
}
