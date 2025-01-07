#include "gui/widgets/PropertyColumnTiltMeasure.h"
#include "gui/Texts.hpp"
#include "models/graph/ColumnTiltMeasureNode.h"
#include "controller/Controller.h"

PropertyColumnTiltMeasure::PropertyColumnTiltMeasure(Controller& controller, QWidget *parent, float guiScale)
	: APropertyGeneral(controller.getDataDispatcher(), parent)
{
	m_ui.setupUi(this);
	m_ui.genericPropsHeadWidget->setControllerInfo(controller);
	m_ui.genericPropsFeetWidget->setDataDispatcher(controller.getDataDispatcher());

	m_dataDispatcher.registerObserverOnKey(this, guiDType::renderValueDisplay);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendAuthorsList);


	m_ui.TiltValuInfield->setType(NumericType::DIAMETER);

	m_ui.HeightInfield->setType(NumericType::DISTANCE);

	m_ui.Point1XInfield->setType(NumericType::DISTANCE);
	m_ui.Point1YInfield->setType(NumericType::DISTANCE);
	m_ui.Point1ZInfield->setType(NumericType::DISTANCE);

	m_ui.Point2XInfield->setType(NumericType::DISTANCE);
	m_ui.Point2YInfield->setType(NumericType::DISTANCE);
	m_ui.Point2ZInfield->setType(NumericType::DISTANCE);

	m_ui.BPXInfield->setType(NumericType::DISTANCE);
	m_ui.BPYInfield->setType(NumericType::DISTANCE);
	m_ui.BPZInfield->setType(NumericType::DISTANCE);

	m_ui.TPXInfield->setType(NumericType::DISTANCE);
	m_ui.TPYInfield->setType(NumericType::DISTANCE);
	m_ui.TPZInfield->setType(NumericType::DISTANCE);
}

PropertyColumnTiltMeasure::~PropertyColumnTiltMeasure()
{
	m_dataDispatcher.unregisterObserver(this);
}

bool PropertyColumnTiltMeasure::actualizeProperty(SafePtr<AGraphNode> object )
{
	if (object)
		m_measure = static_pointer_cast<ColumnTiltMeasureNode>(object);

	m_ui.genericPropsHeadWidget->setObject(m_measure);
	m_ui.genericPropsFeetWidget->setObject(m_measure);

	return updateColumnTilt();
}

void PropertyColumnTiltMeasure::informData(IGuiData *data)
{
	APropertyGeneral::informData(data);
	if (m_columnTiltMethods.find(data->getType()) != m_columnTiltMethods.end())
	{
		PropertyColumnTiltMethod method = m_columnTiltMethods.at(data->getType());
		(this->*method)(data);
	}
}

bool PropertyColumnTiltMeasure::updateColumnTilt()
{
	ReadPtr<ColumnTiltMeasureNode> ct = m_measure.cget();
	if (!ct)
		return false;

	m_ui.TiltValuInfield->setValue(ct->getTiltValue());

	m_ui.HeightInfield->setValue(ct->getHeight());

	m_ui.RatioInfield->setValue(ct->getRatio());
	m_ui.ValueMaxRatioInfield->setValue(ct->getMaxRatio());

	m_ui.OverRatioInfield->setText(ct->getRatioSup() == RatioSup::NA ? TEXT_NA : ct->getRatioSup() == RatioSup::yes ? TEXT_RATIOSUP_YES : TEXT_RATIOSUP_NO);
	m_ui.ResultInfield->setText(ct->getResultReliability() == Reliability::NA ? TEXT_NA : ct->getResultReliability() == Reliability::reliable ? TEXT_RELIABILITY_RELIABLE : TEXT_RELIABILITY_UNRELIABLE);

	m_ui.Point1XInfield->setValue(ct->getPoint1().x);
	m_ui.Point1YInfield->setValue(ct->getPoint1().y);
	m_ui.Point1ZInfield->setValue(ct->getPoint1().z);
	m_ui.Point2XInfield->setValue(ct->getPoint2().x);
	m_ui.Point2YInfield->setValue(ct->getPoint2().y);
	m_ui.Point2ZInfield->setValue(ct->getPoint2().z);

	m_ui.BPXInfield->setValue(ct->getBottomPoint().x);
	m_ui.BPYInfield->setValue(ct->getBottomPoint().y);
	m_ui.BPZInfield->setValue(ct->getBottomPoint().z);
	m_ui.TPXInfield->setValue(ct->getTopPoint().x);
	m_ui.TPYInfield->setValue(ct->getTopPoint().y);
	m_ui.TPZInfield->setValue(ct->getTopPoint().z); 
	
	return true;
}
