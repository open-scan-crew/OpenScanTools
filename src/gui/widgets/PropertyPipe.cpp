#include "gui/widgets/PropertyPipe.h"
#include "gui/widgets/CustomWidgets/GenericPropertiesHead.h"
#include "gui/widgets/CustomWidgets/GenericPropertiesFeet.h"
#include "controller/Controller.h"
#include "controller/controls/ControlClippingEdition.h"
#include "controller/controls/ControlCylinderEdition.h"
#include "controller/controls/ControlObject3DEdition.h"
#include "gui/widgets/FocusWatcher.h"
#include "gui/GuiData/GuiDataList.h"
#include "utils/math/trigo.h"
#include "models/3d/NodeFunctions.h"

#include <QRegExpValidator>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QtConcurrent/qtconcurrentrun.h>
#include <cctype>

#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>

#include "models/graph/CylinderNode.h"

PropertyPipe::PropertyPipe(Controller& controller, QWidget *parent, float guiScale)
	: APropertyGeneral(controller.getDataDispatcher(), parent)
	, m_cylinder()
	, m_forcedDiameter(0.0)
	, m_mode(StandardRadiusData::DiameterSet::Standard)
{
	m_ui.setupUi(this);

	m_ui.genericPropsHeadWidget->setControllerInfo(controller);
	m_ui.genericPropsFeetWidget->setDataDispatcher(controller.getDataDispatcher());
	m_ui.subPropertyClipping->setDataDispatcher(&controller.getDataDispatcher());

	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendListsStandards);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendAuthorsList);

	connect(m_ui.comboStandard, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PropertyPipe::onStandardChange);
	connect(m_ui.forceDiameterField, &QLineEdit::editingFinished, this, &PropertyPipe::onDiameterEdit);
	m_ui.forceDiameterField->setRules(ANumericLineEdit::LineEditRules::PositiveStrict);

	connect(m_ui.StandardRadioButton, &QRadioButton::released, [this]() {this->onDiameterMethod(StandardRadiusData::DiameterSet::Standard); });
	connect(m_ui.ForceRadioButton, &QRadioButton::released, [this]() {this->onDiameterMethod(StandardRadiusData::DiameterSet::Forced); });
	connect(m_ui.KeepRadioButton, &QRadioButton::released, [this]() {this->onDiameterMethod(StandardRadiusData::DiameterSet::Detected); });

	m_pipeMethods.insert({ guiDType::sendListsStandards, &PropertyPipe::onStandardRecieved });

	m_ui.XStartInfield->setType(NumericType::DISTANCE);
	m_ui.YStartInfield->setType(NumericType::DISTANCE);
	m_ui.ZStartInfield->setType(NumericType::DISTANCE);

	m_ui.XEndInfield->setType(NumericType::DISTANCE);
	m_ui.YEndInfield->setType(NumericType::DISTANCE);
	m_ui.ZEndInfield->setType(NumericType::DISTANCE);

	m_ui.lengthField->setType(NumericType::DISTANCE);


	m_ui.diameterField->setType(NumericType::DIAMETER);
	m_ui.forceDiameterField->setType(NumericType::DIAMETER);
	m_ui.adjustedDiameterField->setType(NumericType::DIAMETER);
	m_ui.insulatedThicknessLineEdit->setType(NumericType::DIAMETER);

	m_ui.volumeDoubleEdit->setType(NumericType::VOLUME);

	m_ui.slopeField->setUnit(UnitType::DEG);
}

PropertyPipe::~PropertyPipe()
{
	m_dataDispatcher.unregisterObserver(this);
}

void PropertyPipe::hideEvent(QHideEvent* event)
{
	m_ui.lengthField->blockSignals(true);
	m_ui.diameterField->blockSignals(true);
	m_ui.adjustedDiameterField->blockSignals(true);

	QWidget::hideEvent(event);

	m_ui.lengthField->blockSignals(false);
	m_ui.diameterField->blockSignals(false);
	m_ui.adjustedDiameterField->blockSignals(false);
}

bool PropertyPipe::actualizeProperty(SafePtr<AGraphNode> object )
{
	if (object)
		m_cylinder = static_pointer_cast<CylinderNode>(object);

	m_ui.genericPropsHeadWidget->setObject(m_cylinder);
	m_ui.genericPropsFeetWidget->setObject(m_cylinder);
	m_ui.subPropertyClipping->setObject(m_cylinder);

	updateStandard();

	return updateCylinder();
}

void PropertyPipe::informData(IGuiData *data)
{
	APropertyGeneral::informData(data);
	if (m_pipeMethods.find(data->getType()) != m_pipeMethods.end())
	{
		PropertyPipeMethod method = m_pipeMethods.at(data->getType());
		(this->*method)(data);
	}
}

bool PropertyPipe::updateCylinder()
{
	m_ui.volumeDoubleEdit->setValue(nodeFunctions::calculateVolume(m_cylinder));

	ReadPtr<CylinderNode> rCyl = m_cylinder.cget();
	if (!rCyl)
		return false; 

	blockSignals(true);

	m_ui.lengthField->setValue(rCyl->getLength());

	m_ui.diameterField->setValue(2.0 * rCyl->getDetectedRadius());

	m_ui.forceDiameterField->setValue(2.0 * rCyl->getForcedRadius());
	m_forcedDiameter = 2.0 * rCyl->getForcedRadius();

	m_ui.insulatedThicknessLineEdit->setValue(rCyl->getInsulationRadius());

	setObject3DParameters(*&rCyl);
	switch (rCyl->getDiameterSet())
	{
	case StandardRadiusData::DiameterSet::Standard:
		m_ui.StandardRadioButton->setChecked(true);
		m_ui.comboStandard->setEnabled(true);
		break;
	case StandardRadiusData::DiameterSet::Forced:
		m_ui.ForceRadioButton->setChecked(true);
		m_ui.comboStandard->setEnabled(false);
		break;
	case StandardRadiusData::DiameterSet::Detected:
		m_ui.KeepRadioButton->setChecked(true);
		m_ui.comboStandard->setEnabled(false);
		break;
	}

	m_ui.adjustedDiameterField->setValue(2.0 * rCyl->getRadius());

	blockSignals(false);

	return true;
}

void PropertyPipe::setObject3DParameters(const TransformationModule& data)
{
	glm::dvec4 posStart, posEnd, dir;
	glm::dmat4 rotation(glm::toMat4(data.getOrientation()));
    posStart = glm::dvec4(0.0, 0.0, -data.getScale().z, 1.0);
    posEnd = glm::dvec4(0.0, 0.0, data.getScale().z, 1.0);
	posStart = rotation * posStart;
	posEnd = rotation * posEnd;
    posStart += glm::dvec4(data.getCenter(), 0.0);
    posEnd += glm::dvec4(data.getCenter(), 0.0);
	dir = glm::normalize(posEnd - posStart);

	m_ui.XStartInfield->setValue(posStart.x);
	m_ui.YStartInfield->setValue(posStart.y);
	m_ui.ZStartInfield->setValue(posStart.z);

	m_ui.XEndInfield->setValue(posEnd.x);
	m_ui.YEndInfield->setValue(posEnd.y);
	m_ui.ZEndInfield->setValue(posEnd.z);

	m_ui.XAxeInfield->setValue(dir.x);
	m_ui.YAxeInfield->setValue(dir.y);
	m_ui.ZAxeInfield->setValue(dir.z);

	m_ui.slopeField->setValue(asin(abs(dir.z))*180.0 / M_PI);
}

void PropertyPipe::onStandardRecieved(IGuiData* data)
{
	auto* standards = static_cast<GuiDataSendListsStandards*>(data);
	if(standards->m_type == StandardType::Pipe)
		setStandards(standards->m_lists);
}

void PropertyPipe::setStandards(const std::unordered_set<SafePtr<StandardList>>& list)
{
	m_standards.clear();
	
	for (const SafePtr<StandardList>& stand : list) {
		ReadPtr<StandardList> rStand = stand.cget();
		if (rStand && rStand->getOrigin())
			m_standards.push_back(stand);
	}

	for (const SafePtr<StandardList>& stand : list) {
		ReadPtr<StandardList> rStand = stand.cget();
		if (rStand && !rStand->getOrigin())
			m_standards.push_back(stand);
	}
	
	updateStandard();
}

void PropertyPipe::blockSignals(bool block)
{
	m_ui.lengthField->blockSignals(block);
	m_ui.diameterField->blockSignals(block);
	m_ui.adjustedDiameterField->blockSignals(block);
	m_ui.StandardRadioButton->blockSignals(block);
	m_ui.ForceRadioButton->blockSignals(block);
	m_ui.KeepRadioButton->blockSignals(block);
	m_ui.comboStandard->blockSignals(block);
}

void PropertyPipe::updateStandard()
{
	m_ui.comboStandard->blockSignals(true);

	int selectedItem(0);

	m_ui.comboStandard->clear();

	SafePtr<StandardList> cylStandard;
	{
		ReadPtr<CylinderNode> rCyl = m_cylinder.cget();
		if (!rCyl)
			return;
		cylStandard = rCyl->getStandard();
	}

	for (const SafePtr<StandardList>& stand : m_standards) {
		ReadPtr<StandardList> rStand = stand.cget();
		if (!rStand)
			continue;

		if (stand == cylStandard)
			selectedItem = m_ui.comboStandard->count();

		m_ui.comboStandard->addItem(QString::fromStdWString(rStand->getName()));
	}

	m_standardSet = selectedItem;
	m_ui.comboStandard->setCurrentIndex(selectedItem);

	m_ui.comboStandard->blockSignals(false);
}

void PropertyPipe::onDiameterChange()
{
	switch (m_mode)
	{
		case StandardRadiusData::DiameterSet::Standard:
			if(m_standardSet >= 0 && m_standardSet < m_standards.size())
				m_dataDispatcher.sendControl(new control::cylinderEdition::SetStandard(m_cylinder, m_standards[m_standardSet]));
			break;
		case StandardRadiusData::DiameterSet::Forced:
			m_dataDispatcher.sendControl(new control::cylinderEdition::SetForcedRadius(m_cylinder, m_forcedDiameter / 2.0));
			break;
		case StandardRadiusData::DiameterSet::Detected:
		{
			m_dataDispatcher.sendControl(new control::cylinderEdition::SetDetectedRadius(m_cylinder));
			break;
		}
	}
}

void  PropertyPipe::onDiameterEdit()
{
	PANELLOG << "on diameter edit" << LOGENDL;
	double diameter(m_ui.forceDiameterField->getValue());

	m_forcedDiameter = diameter; 
	onDiameterChange();
}

void PropertyPipe::onDiameterMethod(const StandardRadiusData::DiameterSet& mode)
{
	m_mode = mode;
	onDiameterChange();
}

void PropertyPipe::onStandardChange(const int& index)
{
	if (index != m_standardSet)
	{
		m_standardSet = index;
		onDiameterChange();
	}
}
