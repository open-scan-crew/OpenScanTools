#include "gui/widgets/PropertyPointToPlaneMeasure.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "controller/Controller.h"
#include "controller/controls/ControlDataEdition.h"

#include "models/3d/Graph/PointToPlaneMeasureNode.h"

PropertyPointToPlanMeasure::PropertyPointToPlanMeasure(Controller& controller, QWidget *parent, float guiScale)
	: APropertyGeneral(controller.getDataDispatcher(), parent)
{
	m_ui.setupUi(this);
	m_ui.genericPropsHeadWidget->setControllerInfo(controller);
	m_ui.genericPropsFeetWidget->setDataDispatcher(controller.getDataDispatcher());

	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendAuthorsList);


	m_ui.PtoPDistInfield->setType(NumericType::DISTANCE);
	m_ui.HorizontalInfield->setType(NumericType::DISTANCE);
	m_ui.VerticalInfield->setType(NumericType::DISTANCE);
	m_ui.ProjPtoPDistInfield->setType(NumericType::DISTANCE);

	m_ui.PointCoordXInfield->setType(NumericType::DISTANCE);
	m_ui.PointCoordYInfield->setType(NumericType::DISTANCE);
	m_ui.PointCoordZInfield->setType(NumericType::DISTANCE);

	m_ui.PointOnPlaneXInfield->setType(NumericType::DISTANCE);
	m_ui.PointOnPlaneYInfield->setType(NumericType::DISTANCE);
	m_ui.PointOnPlaneZInfield->setType(NumericType::DISTANCE);

	m_ui.ProjPointXInfield->setType(NumericType::DISTANCE);
	m_ui.ProjPointYInfield->setType(NumericType::DISTANCE);
	m_ui.ProjPointZInfield->setType(NumericType::DISTANCE);

}

PropertyPointToPlanMeasure::~PropertyPointToPlanMeasure()
{
	m_dataDispatcher.unregisterObserver(this);
}

bool PropertyPointToPlanMeasure::actualizeProperty(SafePtr<AGraphNode> object )
{
	if (object)
		m_measure = static_pointer_cast<PointToPlaneMeasureNode>(object);

	m_ui.genericPropsHeadWidget->setObject(m_measure);
	m_ui.genericPropsFeetWidget->setObject(m_measure);

	return updateMeasure();
}

void PropertyPointToPlanMeasure::informData(IGuiData *data)
{
	APropertyGeneral::informData(data);
	if (m_measureMethods.find(data->getType()) != m_measureMethods.end())
	{
		PropertyPointToPlanMethod method = m_measureMethods.at(data->getType());
		(this->*method)(data);
	}
}

bool PropertyPointToPlanMeasure::updateMeasure()
{
	ReadPtr<PointToPlaneMeasureNode> measure = m_measure.cget();
	if (!measure)
		return false;

	m_ui.PtoPDistInfield->setValue(measure->getPointToPlaneD());
	m_ui.HorizontalInfield->setValue(measure->getHorizontal());
	m_ui.VerticalInfield->setValue(measure->getVertical());
	m_ui.ProjPtoPDistInfield->setValue(measure->getPointProjToPlaneD());

	m_ui.PointCoordXInfield->setValue(measure->getPointCoord().x);
	m_ui.PointCoordYInfield->setValue(measure->getPointCoord().y);
	m_ui.PointCoordZInfield->setValue(measure->getPointCoord().z);

	m_ui.PointOnPlaneXInfield->setValue(measure->getPointOnPlane().x);
	m_ui.PointOnPlaneYInfield->setValue(measure->getPointOnPlane().y);
	m_ui.PointOnPlaneZInfield->setValue(measure->getPointOnPlane().z);

	m_ui.ProjPointXInfield->setValue(measure->getProjPoint().x);
	m_ui.ProjPointYInfield->setValue(measure->getProjPoint().y);
	m_ui.ProjPointZInfield->setValue(measure->getProjPoint().z);

	m_ui.NormalToPlaneXInfield->setValue(measure->getNormalToPlane().x);
	m_ui.NormalToPlaneYInfield->setValue(measure->getNormalToPlane().y);
	m_ui.NormalToPlaneZInfield->setValue(measure->getNormalToPlane().z);

	return true;
}
