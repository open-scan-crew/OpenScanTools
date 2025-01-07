#include "gui/widgets/PropertyElbow.h"
#include "gui/widgets/CustomWidgets/GenericPropertiesHead.h"
#include "gui/widgets/CustomWidgets/GenericPropertiesFeet.h"
#include "models/3d/NodeFunctions.h"
#include "models/graph/TorusNode.h"
#include "utils/math/trigo.h"

#include "controller/Controller.h"

#include <glm/gtx/quaternion.hpp>

PropertyElbow::PropertyElbow(Controller& controller, QWidget *parent, float guiScale)
	: APropertyGeneral(controller.getDataDispatcher(), parent)
	, m_storedTorus()
{
	m_ui.setupUi(this);

	m_ui.genericPropsHeadWidget->setControllerInfo(controller);
	m_ui.genericPropsFeetWidget->setDataDispatcher(controller.getDataDispatcher());
	m_ui.subPropertyClipping->setDataDispatcher(&controller.getDataDispatcher());

	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendListsStandards);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendAuthorsList);
	//connect(m_ui.comboStandard, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PropertyElbow::onStandardChange);
	//connect(m_ui.forceDiameterField, &QLineEdit::editingFinished, this, &PropertyElbow::onDiameterEdit);
	//m_ui.forceDiameterField->setRules(ANumericLineEdit::LineEditRules::PositiveStrict);

	m_ui.diametreGroupBox->hide();
	/*
	connect(m_ui.StandardRadioButton, &QRadioButton::released, [this]() {this->onDiameterMethod(DiameterMode::Standard); });
	connect(m_ui.ForceRadioButton, &QRadioButton::released, [this]() {this->onDiameterMethod(DiameterMode::Forced); });
	connect(m_ui.KeepRadioButton, &QRadioButton::released, [this]() {this->onDiameterMethod(DiameterMode::Detected); });
	*/

	m_ui.radiusField->setType(NumericType::DISTANCE);
	m_ui.diameterField->setType(NumericType::DIAMETER);
	m_ui.angleField->setUnit(UnitType::DEG);


	m_ui.XStartInfield->setType(NumericType::DISTANCE);
	m_ui.YStartInfield->setType(NumericType::DISTANCE);
	m_ui.ZStartInfield->setType(NumericType::DISTANCE);

	m_ui.XEndInfield->setType(NumericType::DISTANCE);
	m_ui.YEndInfield->setType(NumericType::DISTANCE);
	m_ui.ZEndInfield->setType(NumericType::DISTANCE);

	m_ui.volumeLineEdit->setType(NumericType::VOLUME);
}

PropertyElbow::~PropertyElbow()
{
	m_dataDispatcher.unregisterObserver(this);
}


void PropertyElbow::hideEvent(QHideEvent* event)
{
	m_ui.radiusField->blockSignals(true);
	m_ui.diameterField->blockSignals(true);

	QWidget::hideEvent(event);

	m_ui.radiusField->blockSignals(false);
	m_ui.diameterField->blockSignals(false);
}

bool PropertyElbow::actualizeProperty(SafePtr<AGraphNode> object )
{
	if (object)
		m_storedTorus = static_pointer_cast<TorusNode>(object);

	m_ui.genericPropsHeadWidget->setObject(m_storedTorus);
	m_ui.genericPropsFeetWidget->setObject(m_storedTorus);
	m_ui.subPropertyClipping->setObject(m_storedTorus);

	return updateTorus();
}

void PropertyElbow::informData(IGuiData *data)
{
	APropertyGeneral::informData(data);
	if (m_elbowMethods.find(data->getType()) != m_elbowMethods.end())
	{
		PropertyElbowMethod method = m_elbowMethods.at(data->getType());
		(this->*method)(data);
	}
}

bool PropertyElbow::updateTorus()
{
	m_ui.volumeLineEdit->setValue(nodeFunctions::calculateVolume(m_storedTorus));

	ReadPtr<TorusNode> torus = m_storedTorus.cget();
	if (!torus)
		return false;

	m_ui.radiusField->blockSignals(true);
	m_ui.diameterField->blockSignals(true);

	m_ui.radiusField->setValue(torus->getMainRadius());
	m_ui.diameterField->setValue(2.0 * torus->getAdjustedTubeRadius());

	m_ui.angleField->setValue(torus->getMainAngle() * (180.0/M_PI));
	m_ui.ratioRonDextField->setValue(torus->getMainRadius()/(2.0 * torus->getAdjustedTubeRadius()));

	/*if (!m_diameter) {
		m_diameter = m_storedCylinder.getRadius() * 2.0;
		m_ui.forceDiameterField->setPlaceholderText(valueDisplay(m_diameter));
	}*/

	glm::dvec4 posStart, posEnd, dir;
	glm::dmat4 rotation(glm::toMat4(torus->getOrientation()));
	posStart = glm::dvec4(0.0, 0.0, -torus->getScale().z, 1.0);
	posEnd = glm::dvec4(0.0, 0.0, torus->getScale().z, 1.0);
	posStart = rotation * posStart;
	posEnd = rotation * posEnd;
	posStart += glm::dvec4(torus->getCenter(), 0.0);
	posEnd += glm::dvec4(torus->getCenter(), 0.0);
	dir = glm::normalize(posEnd - posStart);

	m_eulers = tls::math::quat_to_euler_zyx_deg(torus->getOrientation());

	m_ui.XStartInfield->setValue(posStart.x);
	m_ui.YStartInfield->setValue(posStart.y);
	m_ui.ZStartInfield->setValue(posStart.z);
	m_ui.XEndInfield->setValue(posEnd.x);
	m_ui.YEndInfield->setValue(posEnd.y);
	m_ui.ZEndInfield->setValue(posEnd.z); 
	
	m_ui.radiusField->blockSignals(false);
	m_ui.diameterField->blockSignals(false);

	return true;
}

/*void PropertyElbow::onStandardRecieved(IGuiData* data)
{
	auto* standards = static_cast<GuiDataSendListsStandards*>(data);
	if(standards->m_type == StandardType::Pipe)
		setStandards(standards->m_list);
}*/

/*void PropertyElbow::setStandards(const std::unordered_map<listId, StandardList>& list)
{
	m_ui.comboStandard->blockSignals(true);
	//int selectedItem(0);
	//selectedItem = selectedItem == -1 ? 0 : selectedItem;
	m_ui.comboStandard->clear();
	m_standards.clear();

	for (const auto& iterator : list) {
		if (iterator.second.getOrigin()) {
			m_standards.push_back(std::pair<listId, std::string>(iterator.first, iterator.second.getName()));
			m_ui.comboStandard->addItem(QString::fromStdString(iterator.second.getName()));
			break;
		}
	}

	for (const auto& iterator : list)
	{
		if (iterator.second.getOrigin())
			continue;
		if (iterator.first == m_storedTorus.getStandard())
			selectedItem = (int)m_standards.size();
		m_standards.push_back(std::pair<listId, std::string>(iterator.first, iterator.second.getName()));
		m_ui.comboStandard->addItem(QString::fromStdString(iterator.second.getName()));
	}

	switch (m_storedTorus.getDiameterSet())
	{
		case UICylinder::DiameterSet::Standard:
			m_ui.StandardRadioButton->setChecked(true);
			m_ui.comboStandard->setEnabled(true);
			break;
		case UICylinder::DiameterSet::Forced:
			m_ui.ForceRadioButton->setChecked(true);
			m_ui.comboStandard->setEnabled(false);
			break;
		case UICylinder::DiameterSet::Detected:
			m_ui.KeepRadioButton->setChecked(true);
			m_ui.comboStandard->setEnabled(false);
			break;
	}
	m_standardSet = selectedItem;
	m_ui.comboStandard->setCurrentIndex(selectedItem);

	m_ui.comboStandard->blockSignals(false);
}
*/
/*
void PropertyElbow::onDiameterChange()
{
	switch (m_mode)
	{
	case DiameterMode::Standard:
		if(m_standardSet >= 0 && m_standardSet < m_standards.size())
			m_dataDispatcher.sendControl(new control::cylinderEdition::CheckStandard(m_storedCylinder.getId(), m_standards[m_standardSet].first));
		break;
	case DiameterMode::Forced:
		m_dataDispatcher.sendControl(new control::cylinderEdition::SetRadius(m_storedCylinder.getId(), m_diameter / 2.0, true));
		break;
	case DiameterMode::Detected:
		m_dataDispatcher.sendControl(new control::cylinderEdition::SetRadius(m_storedCylinder.getId(), m_storedCylinder.getDetectedRadius(), false));
		break;
	}
}
*/

/*
void  PropertyElbow::onDiameterEdit()
{
	GUI_LOG << "on diameter edit" << LOGENDL;
	double diameter(unit_converter::XToMeter(m_ui.forceDiameterField->getValue(), m_valueDisplayParameters.diameterUnit));
	if (diameter && m_diameter != diameter)
	{
		m_diameter = diameter; 
		//onDiameterChange();
	}
}
*/

/*
void PropertyElbow::onDiameterMethod(const DiameterMode& mode)
{
	m_mode = mode;
	onDiameterChange();
}*/

/*void PropertyElbow::onStandardChange(const int& index)
{
	if (index != m_standardSet)
	{
		m_standardSet = index;
		onDiameterChange();
	}
}*/