#include "gui/widgets/PropertyPointToPipeMeasure.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "controller/Controller.h"
#include "controller/controls/ControlDataEdition.h"

#include "models/graph/PointToPipeMeasureNode.h"

PropertyPointToPipeMeasure::PropertyPointToPipeMeasure(Controller& controller, QWidget *parent, float guiScale)
	: APropertyGeneral(controller.getDataDispatcher(), parent)
{
	m_ui.setupUi(this);
	m_ui.genericPropsHeadWidget->setControllerInfo(controller);
	m_ui.genericPropsFeetWidget->setDataDispatcher(controller.getDataDispatcher());

	m_dataDispatcher.registerObserverOnKey(this, guiDType::renderValueDisplay);
	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendAuthorsList);

	m_ui.PToAxeDistInfield->setType(NumericType::DISTANCE);
	m_ui.HorizontalInfield->setType(NumericType::DISTANCE);
	m_ui.VerticalInfield->setType(NumericType::DISTANCE);
	m_ui.FreeDistInfield->setType(NumericType::DISTANCE);
	m_ui.FDHInfield->setType(NumericType::DISTANCE);
	m_ui.FDVInfield->setType(NumericType::DISTANCE);
	m_ui.totalFootprintLineEdit->setType(NumericType::DISTANCE);

	m_ui.PipeDiameterInfield->setType(NumericType::DIAMETER);
	m_ui.PipeCenterProjInfield->setType(NumericType::DISTANCE);

	m_ui.PipeCenterXInfield->setType(NumericType::DISTANCE);
	m_ui.PipeCenterYInfield->setType(NumericType::DISTANCE);
	m_ui.PipeCenterZInfield->setType(NumericType::DISTANCE);

	m_ui.PointCoordXInfield->setType(NumericType::DISTANCE);
	m_ui.PointCoordYInfield->setType(NumericType::DISTANCE);
	m_ui.PointCoordZInfield->setType(NumericType::DISTANCE);

	m_ui.ProjPointXInfield->setType(NumericType::DISTANCE);
	m_ui.ProjPointYInfield->setType(NumericType::DISTANCE);
	m_ui.ProjPointZInfield->setType(NumericType::DISTANCE);
}

PropertyPointToPipeMeasure::~PropertyPointToPipeMeasure()
{
	m_dataDispatcher.unregisterObserver(this);
}

bool PropertyPointToPipeMeasure::actualizeProperty(SafePtr<AGraphNode> object )
{
	if (object)
		m_measure = static_pointer_cast<PointToPipeMeasureNode>(object);

	m_ui.genericPropsHeadWidget->setObject(m_measure);
	m_ui.genericPropsFeetWidget->setObject(m_measure);

	return updateMeasure();
}

void PropertyPointToPipeMeasure::informData(IGuiData *data)
{
	APropertyGeneral::informData(data);
	if (m_measureMethods.find(data->getType()) != m_measureMethods.end())
	{
		PropertyPointToPipeMethod method = m_measureMethods.at(data->getType());
		(this->*method)(data);
	}
}

bool PropertyPointToPipeMeasure::updateMeasure()
{
	ReadPtr<PointToPipeMeasureNode> measure = m_measure.cget();
	if (!measure)
		return false;

	m_ui.PToAxeDistInfield->setValue(measure->getPointToAxeDist());

	m_ui.HorizontalInfield->setValue(measure->getPointToAxeHorizontal());
	m_ui.VerticalInfield->setValue(measure->getPointToAxeVertical());
	m_ui.FreeDistInfield->setValue(measure->getFreeDist());
	m_ui.FDHInfield->setValue(measure->getFreeDistHorizontal());
	m_ui.FDVInfield->setValue(measure->getFreeDistVertical());
	m_ui.totalFootprintLineEdit->setValue(measure->getTotalFootprint());

	m_ui.PipeDiameterInfield->setValue(measure->getPipeDiameter());
	m_ui.PipeCenterProjInfield->setValue(measure->getPipeCenterToProj());

	m_ui.PipeCenterXInfield->setValue(measure->getPipeCenter().x);
	m_ui.PipeCenterYInfield->setValue(measure->getPipeCenter().y);
	m_ui.PipeCenterZInfield->setValue(measure->getPipeCenter().z);

	m_ui.PointCoordXInfield->setValue(measure->getPointCoord().x);
	m_ui.PointCoordYInfield->setValue(measure->getPointCoord().y);
	m_ui.PointCoordZInfield->setValue(measure->getPointCoord().z);

	m_ui.ProjPointXInfield->setValue(measure->getProjPoint().x);
	m_ui.ProjPointYInfield->setValue(measure->getProjPoint().y);
	m_ui.ProjPointZInfield->setValue(measure->getProjPoint().z);

	return true;
}
