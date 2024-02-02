#include "gui/widgets/PropertySimpleMeasure.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "controller/Controller.h"
#include "controller/controls/ControlDataEdition.h"
#include "controller/controls/ControlClippingEdition.h"

#include "models/3d/Graph/SimpleMeasureNode.h"

PropertySimpleMeasure::PropertySimpleMeasure(Controller& controller, QWidget *parent, float guiScale)
	: APropertyGeneral(controller.getDataDispatcher(), parent)
{
	m_ui.setupUi(this);
	m_ui.genericPropsHeadWidget->setControllerInfo(controller);
	m_ui.genericPropsFeetWidget->setDataDispatcher(controller.getDataDispatcher());
	m_ui.subPropertyClipping->setDataDispatcher(&controller.getDataDispatcher());

	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendAuthorsList);

	m_ui.TotalInfield->setType(NumericType::DISTANCE);
	m_ui.AlongXInfield->setType(NumericType::DISTANCE);
	m_ui.AlongYInfield->setType(NumericType::DISTANCE);
	m_ui.VerticalInfield->setType(NumericType::DISTANCE);
	m_ui.HorizontalInfield->setType(NumericType::DISTANCE);

	m_ui.X1Infield->setType(NumericType::DISTANCE);
	m_ui.Y1Infield->setType(NumericType::DISTANCE);
	m_ui.Z1Infield->setType(NumericType::DISTANCE);
	m_ui.X2Infield->setType(NumericType::DISTANCE);
	m_ui.Y2Infield->setType(NumericType::DISTANCE);
	m_ui.Z2Infield->setType(NumericType::DISTANCE);

	m_ui.AngleInfield->setUnit(UnitType::DEG);
}

PropertySimpleMeasure::~PropertySimpleMeasure()
{
	m_dataDispatcher.unregisterObserver(this);
}

bool PropertySimpleMeasure::actualizeProperty(SafePtr<AGraphNode> object )
{
	if (object)
		m_measure = static_pointer_cast<SimpleMeasureNode>(object);

	m_ui.genericPropsHeadWidget->setObject(m_measure);
	m_ui.genericPropsFeetWidget->setObject(m_measure);
	m_ui.subPropertyClipping->setObject(m_measure);

	return updateMeasure();
}

void PropertySimpleMeasure::informData(IGuiData *data)
{
	APropertyGeneral::informData(data);
	if (m_simpleMethods.find(data->getType()) != m_simpleMethods.end())
	{
		PropertySimpleMethod method = m_simpleMethods.at(data->getType());
		(this->*method)(data);
	}
}

bool PropertySimpleMeasure::updateMeasure()
{
	ReadPtr<SimpleMeasureNode> node = m_measure.cget();
	if (!node)
		return false;

	m_ui.TotalInfield->blockSignals(true);
	m_ui.AlongXInfield->blockSignals(true);
	m_ui.AlongYInfield->blockSignals(true);
	m_ui.VerticalInfield->blockSignals(true);
	m_ui.HorizontalInfield->blockSignals(true);
	m_ui.AngleInfield->blockSignals(true);
	m_ui.X1Infield->blockSignals(true);
	m_ui.Y1Infield->blockSignals(true);
	m_ui.Z1Infield->blockSignals(true);
	m_ui.X2Infield->blockSignals(true);
	m_ui.Y2Infield->blockSignals(true);
	m_ui.Z2Infield->blockSignals(true);

	m_ui.TotalInfield->setValue(node->getMeasure().getDistanceTotal());
	m_ui.AlongXInfield->setValue(node->getMeasure().getDistanceAlongX());
	m_ui.AlongYInfield->setValue(node->getMeasure().getDistanceAlongY());
	m_ui.VerticalInfield->setValue(node->getMeasure().getDistanceAlongZ());
	m_ui.HorizontalInfield->setValue(node->getMeasure().getDistanceHorizontal());

	m_ui.X1Infield->setValue(node->getOriginPos().x);
	m_ui.Y1Infield->setValue(node->getOriginPos().y);
	m_ui.Z1Infield->setValue(node->getOriginPos().z);
	m_ui.X2Infield->setValue(node->getDestinationPos().x);
	m_ui.Y2Infield->setValue(node->getDestinationPos().y);
	m_ui.Z2Infield->setValue(node->getDestinationPos().z);

	m_ui.AngleInfield->setValue(node->getMeasure().getAngleHorizontal());

	m_ui.TotalInfield->blockSignals(false);
	m_ui.AlongXInfield->blockSignals(false);
	m_ui.AlongYInfield->blockSignals(false);
	m_ui.VerticalInfield->blockSignals(false);
	m_ui.HorizontalInfield->blockSignals(false);
	m_ui.AngleInfield->blockSignals(false);
	m_ui.X1Infield->blockSignals(false);
	m_ui.Y1Infield->blockSignals(false);
	m_ui.Z1Infield->blockSignals(false);
	m_ui.X2Infield->blockSignals(false);
	m_ui.Y2Infield->blockSignals(false);
	m_ui.Z2Infield->blockSignals(false);

	return true;
}
