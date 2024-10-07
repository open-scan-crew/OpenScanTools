#include "gui/widgets/PropertyPipeToPlaneMeasure.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "controller/Controller.h"
#include "controller/controls/ControlDataEdition.h"
#include "models/graph/PipeToPlaneMeasureNode.h"

PropertyPipeToPlaneMeasure::PropertyPipeToPlaneMeasure(Controller& controller, QWidget *parent, float guiScale)
	: APropertyGeneral(controller.getDataDispatcher(), parent)
{
	m_ui.setupUi(this);
	m_ui.genericPropsHeadWidget->setControllerInfo(controller);
	m_ui.genericPropsFeetWidget->setDataDispatcher(controller.getDataDispatcher());

	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendAuthorsList);

	m_ui.PipeDiameterInfield->setType(NumericType::DIAMETER);

	m_ui.CenterToPlaneDistInfield->setType(NumericType::DISTANCE);
	m_ui.HoriInfield->setType(NumericType::DISTANCE);
	m_ui.VertiInfield->setType(NumericType::DISTANCE);
	m_ui.FreeIdstBetCInfield->setType(NumericType::DISTANCE);
	m_ui.Hori2Infield->setType(NumericType::DISTANCE);
	m_ui.Verti2Infield->setType(NumericType::DISTANCE);
	m_ui.totalFootprintLineEdit->setType(NumericType::DISTANCE);

	m_ui.PointOnPlaneToProjInfield->setType(NumericType::DISTANCE);

	m_ui.PipeCenterXInfield->setType(NumericType::DISTANCE);
	m_ui.PipeCenterYInfield->setType(NumericType::DISTANCE);
	m_ui.PipeCenterZInfield->setType(NumericType::DISTANCE);
	m_ui.PointOnPlaneXInfield->setType(NumericType::DISTANCE);
	m_ui.PointOnPlaneYInfield->setType(NumericType::DISTANCE);
	m_ui.PointOnPlaneZInfield->setType(NumericType::DISTANCE);
	m_ui.ProjPointXInfield->setType(NumericType::DISTANCE);
	m_ui.ProjPointYInfield->setType(NumericType::DISTANCE);
	m_ui.ProjPointZInfield->setType(NumericType::DISTANCE);
}

PropertyPipeToPlaneMeasure::~PropertyPipeToPlaneMeasure()
{
	m_dataDispatcher.unregisterObserver(this);
}

bool PropertyPipeToPlaneMeasure::actualizeProperty(SafePtr<AGraphNode> object )
{
	if (object)
		m_measure = static_pointer_cast<PipeToPlaneMeasureNode>(object);

	m_ui.genericPropsHeadWidget->setObject(m_measure);
	m_ui.genericPropsFeetWidget->setObject(m_measure);

	return updateMeasure();
}

void PropertyPipeToPlaneMeasure::informData(IGuiData *data)
{
	APropertyGeneral::informData(data);
	if (m_measureMethods.find(data->getType()) != m_measureMethods.end())
	{
		PropertyPipeToPlaneMethod method = m_measureMethods.at(data->getType());
		(this->*method)(data);
	}
}

bool PropertyPipeToPlaneMeasure::updateMeasure()
{
	ReadPtr<PipeToPlaneMeasureNode> node = m_measure.cget();
	if (!node)
		return false;

	m_ui.PipeDiameterInfield->setValue(node->getPipeDiameter());

	m_ui.CenterToPlaneDistInfield->setValue(node->getCenterToPlaneDist());
	m_ui.HoriInfield->setValue(node->getPlaneCenterHorizontal());
	m_ui.VertiInfield->setValue(node->getPlaneCenterVertical());
	m_ui.FreeIdstBetCInfield->setValue(node->getFreeDist());
	m_ui.Hori2Infield->setValue(node->getFreeDistHorizontal());
	m_ui.Verti2Infield->setValue(node->getFreeDistVertical());
	m_ui.totalFootprintLineEdit->setValue(node->getTotalFootprint());

	m_ui.PointOnPlaneToProjInfield->setValue(node->getPointOnPlaneToProj());

	m_ui.PipeCenterXInfield->setValue(node->getPipeCenter().x);
	m_ui.PipeCenterYInfield->setValue(node->getPipeCenter().y);
	m_ui.PipeCenterZInfield->setValue(node->getPipeCenter().z);

	m_ui.PointOnPlaneXInfield->setValue(node->getPointOnPlane().x);
	m_ui.PointOnPlaneYInfield->setValue(node->getPointOnPlane().y);
	m_ui.PointOnPlaneZInfield->setValue(node->getPointOnPlane().z);

	m_ui.ProjPointXInfield->setValue(node->getProjPoint().x);
	m_ui.ProjPointYInfield->setValue(node->getProjPoint().y);
	m_ui.ProjPointZInfield->setValue(node->getProjPoint().z);

	m_ui.NormalToPlaneXInfield->setValue(node->getNormalOnPlane().x);
	m_ui.NormalToPlaneYInfield->setValue(node->getNormalOnPlane().y);
	m_ui.NormalToPlaneZInfield->setValue(node->getNormalOnPlane().z);

	return true;
}
