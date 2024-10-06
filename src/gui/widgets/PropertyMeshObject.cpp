#include "gui/widgets/PropertyMeshObject.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "controller/controls/ControlObject3DEdition.h"
#include "gui/widgets/CustomWidgets/GenericPropertiesHead.h"
#include "gui/widgets/CustomWidgets/GenericPropertiesFeet.h"
#include "controller/controls/ControlDataEdition.h"
#include "gui/widgets/FocusWatcher.h"
#include "utils/math/trigo.h"
#include "controller/Controller.h"

#include "models/graph/MeshObjectNode.h"

PropertyMeshObject::PropertyMeshObject(Controller& controller, QWidget* parent, float guiScale)
	: APropertyGeneral(controller.getDataDispatcher(), parent)
{
	m_ui.setupUi(this);

	m_ui.genericPropsHeadWidget->setControllerInfo(controller);
	m_ui.genericPropsFeetWidget->setDataDispatcher(controller.getDataDispatcher());

	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendAuthorsList);

	m_ui.file->hide();
	m_ui.fileLabel->hide();

	adjustSize();

	// Link action
	connect(m_ui.XCenterInfield, &QLineEdit::editingFinished, this, &PropertyMeshObject::onCenterXEdit);
	connect(m_ui.YCenterInfield, &QLineEdit::editingFinished, this, &PropertyMeshObject::onCenterYEdit);
	connect(m_ui.ZCenterInfield, &QLineEdit::editingFinished, this, &PropertyMeshObject::onCenterZEdit);
	connect(m_ui.XOrientInfield, &QLineEdit::editingFinished, this, &PropertyMeshObject::onOrientXEdit);
	connect(m_ui.YOrientInfield, &QLineEdit::editingFinished, this, &PropertyMeshObject::onOrientYEdit);
	connect(m_ui.ZOrientInfield, &QLineEdit::editingFinished, this, &PropertyMeshObject::onOrientZEdit);
	connect(m_ui.ScaleInfield, &QLineEdit::editingFinished, this, &PropertyMeshObject::onScaleEdit);

	m_ui.XCenterInfield->setType(NumericType::DISTANCE);
	m_ui.YCenterInfield->setType(NumericType::DISTANCE);
	m_ui.ZCenterInfield->setType(NumericType::DISTANCE);

	m_ui.XOrientInfield->setUnit(UnitType::DEG);
	m_ui.YOrientInfield->setUnit(UnitType::DEG);
	m_ui.ZOrientInfield->setUnit(UnitType::DEG);

	m_ui.ScaleInfield->setRules(ANumericLineEdit::LineEditRules::PositiveStrict);
}

PropertyMeshObject::~PropertyMeshObject()
{
	m_dataDispatcher.unregisterObserver(this);
}

void PropertyMeshObject::hideEvent(QHideEvent* event)
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

bool PropertyMeshObject::actualizeProperty(SafePtr<AGraphNode> object )
{
	if (object)
		m_object = static_pointer_cast<MeshObjectNode>(object);

	m_ui.genericPropsHeadWidget->setObject(m_object);
	m_ui.genericPropsFeetWidget->setObject(m_object);

	return updateMeshObject();
}

void PropertyMeshObject::informData(IGuiData* data)
{
	APropertyGeneral::informData(data);
	if (m_meshObjMethods.find(data->getType()) != m_meshObjMethods.end())
	{
		PropertyMeshObjectMethod method = m_meshObjMethods.at(data->getType());
		(this->*method)(data);
	}
}

bool PropertyMeshObject::updateMeshObject()
{
	ReadPtr<MeshObjectNode> mo = m_object.cget();
	if (!mo)
		return false;

	m_ui.file->setText(QString::fromStdWString(mo->getFilePath().wstring()));
	m_ui.object->setText(QString::fromStdWString(mo->getObjectName()));

	m_ui.XCenterInfield->blockSignals(true);
	m_ui.YCenterInfield->blockSignals(true);
	m_ui.ZCenterInfield->blockSignals(true);
	m_ui.XOrientInfield->blockSignals(true);
	m_ui.YOrientInfield->blockSignals(true);
	m_ui.ZOrientInfield->blockSignals(true);

	glm::dvec3 eulers = tls::math::quat_to_euler_zyx_deg(mo->getOrientation());

	m_ui.XCenterInfield->setValue(mo->getCenter().x);
	m_ui.YCenterInfield->setValue(mo->getCenter().y);
	m_ui.ZCenterInfield->setValue(mo->getCenter().z);

	m_ui.XOrientInfield->setValue(eulers.x);
	m_ui.YOrientInfield->setValue(eulers.y);
	m_ui.ZOrientInfield->setValue(eulers.z);

	m_ui.ScaleInfield->setValue(mo->getScale().x);

	m_ui.XCenterInfield->blockSignals(false);
	m_ui.YCenterInfield->blockSignals(false);
	m_ui.ZCenterInfield->blockSignals(false);
	m_ui.XOrientInfield->blockSignals(false);
	m_ui.YOrientInfield->blockSignals(false);
	m_ui.ZOrientInfield->blockSignals(false);

	return true;
}

void PropertyMeshObject::changeOrientation()
{
	glm::dvec3 orientation(m_ui.XOrientInfield->getValue(), m_ui.YOrientInfield->getValue(), m_ui.ZOrientInfield->getValue());
	m_dataDispatcher.sendControl(new control::object3DEdition::SetRotation(m_object, tls::math::euler_deg_to_quat(orientation)));
}

void PropertyMeshObject::onOrientXEdit()
{
	if (!m_ui.XOrientInfield->isModified())
		return;
	m_ui.XOrientInfield->setModified(false);

	changeOrientation();
}

void PropertyMeshObject::onOrientYEdit()
{
	if (!m_ui.YOrientInfield->isModified())
		return;
	m_ui.YOrientInfield->setModified(false);

	changeOrientation();
}

void PropertyMeshObject::onOrientZEdit()
{
	if (!m_ui.ZOrientInfield->isModified())
		return;
	m_ui.ZOrientInfield->setModified(false);

	changeOrientation();
}

void  PropertyMeshObject::changeCenter()
{
	glm::dvec3 position(m_ui.XCenterInfield->getValue(), 
							m_ui.YCenterInfield->getValue(),
							m_ui.ZCenterInfield->getValue());
	m_dataDispatcher.sendControl(new control::object3DEdition::SetCenter(m_object, position));
}

void PropertyMeshObject::onCenterXEdit()
{
	if (!m_ui.XCenterInfield->isModified())
		return;
	m_ui.XCenterInfield->setModified(false);

	changeCenter();
}

void PropertyMeshObject::onCenterYEdit()
{
	if (!m_ui.YCenterInfield->isModified())
		return;
	m_ui.YCenterInfield->setModified(false);

	changeCenter();
}

void PropertyMeshObject::onCenterZEdit()
{
	if (!m_ui.ZCenterInfield->isModified())
		return;
	m_ui.ZCenterInfield->setModified(false);

	changeCenter();
}

void PropertyMeshObject::onScaleEdit()
{
	if (!m_ui.ScaleInfield->isModified())
		return;
	m_ui.ScaleInfield->setModified(false);
	m_dataDispatcher.sendControl(new control::object3DEdition::SetSize(m_object, glm::dvec3(m_ui.ScaleInfield->getValue())));
}
