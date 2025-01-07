#include "gui/widgets/PropertyPipeToPipeMeasure.h"
#include "controller/Controller.h"

#include "models/graph/PipeToPipeMeasureNode.h"

PropertyPipeToPipeMeasure::PropertyPipeToPipeMeasure(Controller& controller, QWidget *parent, float guiScale)
	: APropertyGeneral(controller.getDataDispatcher(), parent)
{
	m_ui.setupUi(this);
	m_ui.genericPropsHeadWidget->setControllerInfo(controller);
	m_ui.genericPropsFeetWidget->setDataDispatcher(controller.getDataDispatcher());

	m_dataDispatcher.registerObserverOnKey(this, guiDType::sendAuthorsList);


	m_ui.C1DiamInfield->setType(NumericType::DIAMETER);
	m_ui.C2DiamInfield->setType(NumericType::DIAMETER);

	m_ui.DbetweenAxisInfield->setType(NumericType::DISTANCE);
	m_ui.HoriInfield->setType(NumericType::DISTANCE);
	m_ui.VertiInfield->setType(NumericType::DISTANCE);
	m_ui.FreeIdstBetCInfield->setType(NumericType::DISTANCE);
	m_ui.Hori2Infield->setType(NumericType::DISTANCE);
	m_ui.Verti2Infield->setType(NumericType::DISTANCE);
	m_ui.totalFootprintLineEdit->setType(NumericType::DISTANCE);

	m_ui.P2CenterToProjInfield->setType(NumericType::DISTANCE);

	m_ui.Pipe1CenterXInfield->setType(NumericType::DISTANCE);
	m_ui.Pipe1CenterYInfield->setType(NumericType::DISTANCE);
	m_ui.Pipe1CenterZInfield->setType(NumericType::DISTANCE);

	m_ui.Pipe2CenterXInfield->setType(NumericType::DISTANCE);
	m_ui.Pipe2CenterYInfield->setType(NumericType::DISTANCE);
	m_ui.Pipe2CenterZInfield->setType(NumericType::DISTANCE);

	m_ui.ProjPointXInfield->setType(NumericType::DISTANCE);
	m_ui.ProjPointYInfield->setType(NumericType::DISTANCE);
	m_ui.ProjPointZInfield->setType(NumericType::DISTANCE);
}

PropertyPipeToPipeMeasure::~PropertyPipeToPipeMeasure()
{
	m_dataDispatcher.unregisterObserver(this);
}

bool PropertyPipeToPipeMeasure::actualizeProperty(SafePtr<AGraphNode> object)
{
	if (object)
		m_measure = static_pointer_cast<PipeToPipeMeasureNode>(object);

	m_ui.genericPropsHeadWidget->setObject(m_measure);
	m_ui.genericPropsFeetWidget->setObject(m_measure);

	return updateMeasure();
}

void PropertyPipeToPipeMeasure::informData(IGuiData *data)
{
	APropertyGeneral::informData(data);
	if (m_measureMethods.find(data->getType()) != m_measureMethods.end())
	{
		PropertyPipeToPipeMethod method = m_measureMethods.at(data->getType());
		(this->*method)(data);
	}
}

bool PropertyPipeToPipeMeasure::updateMeasure()
{
	ReadPtr<PipeToPipeMeasureNode> node = m_measure.cget();
	if (!node)
		return false;

	m_ui.C1DiamInfield->setValue(node->getPipe1Diameter());
	m_ui.C2DiamInfield->setValue(node->getPipe2Diameter());

	m_ui.DbetweenAxisInfield->setValue(node->getCenterP1ToAxeP2());
	m_ui.HoriInfield->setValue(node->getP1ToP2Horizontal());
	m_ui.VertiInfield->setValue(node->getP1ToP2Vertical());
	m_ui.FreeIdstBetCInfield->setValue(node->getFreeDist());
	m_ui.Hori2Infield->setValue(node->getFreeDistHorizontal());
	m_ui.Verti2Infield->setValue(node->getFreeDistVertical());
	m_ui.totalFootprintLineEdit->setValue(node->getTotalFootprint());

	m_ui.P2CenterToProjInfield->setValue(node->getPipe2CenterToProj());

	m_ui.Pipe1CenterXInfield->setValue(node->getPipe1Center().x);
	m_ui.Pipe1CenterYInfield->setValue(node->getPipe1Center().y);
	m_ui.Pipe1CenterZInfield->setValue(node->getPipe1Center().z);

	m_ui.Pipe2CenterXInfield->setValue(node->getPipe2Center().x);
	m_ui.Pipe2CenterYInfield->setValue(node->getPipe2Center().y);
	m_ui.Pipe2CenterZInfield->setValue(node->getPipe2Center().z);

	m_ui.ProjPointXInfield->setValue(node->getProjPoint().x);
	m_ui.ProjPointYInfield->setValue(node->getProjPoint().y);
	m_ui.ProjPointZInfield->setValue(node->getProjPoint().z);

	return true;
}
