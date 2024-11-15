#include "controller/functionSystem/ContextPointToPlane3Measure.h"
#include "controller/Controller.h"
#include "controller/IControlListener.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "pointCloudEngine/MeasureClass.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"
#include "controller/controls/ControlFunction.h"

#include "models/graph/GraphManager.hxx"
#include "models/graph/PointToPlaneMeasureNode.h"

ContextPointToPlane3Measure::ContextPointToPlane3Measure(const ContextId& id)
	: ARayTracingContext(id)
{
    m_usages.push_back({ true, {ElementType::Tag, ElementType::Point}, TEXT_POINTTOPLANE_START });
    m_usages.push_back({ true, {ElementType::Tag, ElementType::Point}, TEXT_PLANE3_FIRST });
    m_usages.push_back({ true, {ElementType::Tag, ElementType::Point}, TEXT_PLANE3_SECOND });
    m_usages.push_back({ true, {ElementType::Tag, ElementType::Point}, TEXT_PLANE3_THIRD });
}

ContextPointToPlane3Measure::~ContextPointToPlane3Measure()
{
}

ContextState ContextPointToPlane3Measure::start(Controller& controller)
{
	return ARayTracingContext::start(controller);
}

ContextState ContextPointToPlane3Measure::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
    return m_state;
}

ContextState ContextPointToPlane3Measure::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

	controller.updateInfo(new GuiDataTmpMessage(TEXT_LUCAS_SEARCH_ONGOING, 0));
	bool success = false;

	GraphManager& graphManager = controller.getGraphManager();

    TlScanOverseer::setWorkingScansTransfo(graphManager.getVisiblePointCloudInstances(m_panoramic, true, true));
	std::vector<double> plane;
	std::vector<glm::dvec3> planePoints;
	planePoints.push_back(m_clickResults[1].position);
	planePoints.push_back(m_clickResults[2].position);
	planePoints.push_back(m_clickResults[3].position);
	QString error;
	if (TlScanOverseer::getInstance().fitPlane3Points(planePoints, plane))
	{
		glm::dvec3 normalVector = glm::dvec3(plane[0], plane[1], plane[2]);
		normalVector /= glm::length(normalVector);
		glm::dvec3 projectedPoint = MeasureClass::projectPointToPlane(m_clickResults[0].position, plane);

		SafePtr<PointToPlaneMeasureNode> measure = graphManager.createMeasureNode<PointToPlaneMeasureNode>();
		WritePtr<PointToPlaneMeasureNode> wMeasure = measure.get();
		if (!wMeasure)
		{
			m_clickResults.clear();
			return waitForNextPoint(controller);
		}

		wMeasure->setDefaultData(controller);

		wMeasure->setVisible(true);
		wMeasure->setPointToPlaneD((float)glm::length(m_clickResults[0].position - projectedPoint));
		wMeasure->setVertical((float)abs((m_clickResults[0].position - projectedPoint)[2]));
		glm::dvec3 temp = m_clickResults[0].position - projectedPoint;
		temp[2] = 0;
		wMeasure->setHorizontal((float)glm::length(temp));
		wMeasure->setPointCoord(m_clickResults[0].position);
		wMeasure->setpointOnPlane((1.0 / 3.0) * (m_clickResults[1].position + m_clickResults[2].position + m_clickResults[3].position));
		normalVector /= glm::length(normalVector);
		wMeasure->setNormalToPlane(normalVector);
		wMeasure->setProjPoint(projectedPoint);

		controller.getControlListener()->notifyUIControl(new control::function::AddNodes(measure));

		/*Measure newMeasure1;
		newMeasure1.origin = m_points[1];
		newMeasure1.final = projectedPoint;
		SimpleMeasure *measure1 = new SimpleMeasure("Measure1", controller.getContext().getActiveAuthor());
		measure1->setName(std::string("measure1"));
		measure1->setMeasure(newMeasure1);
		measure1->setOriginPos(newMeasure1.origin);
		measure1->setDestinationPos(newMeasure1.final);
		measure1->setDescription("");
		measure1->setVisible(true);
		controller.getControlListener()->notifyUIControl(new control::function::measure::CreateSimpleMeasure(measure1));
		//box to display plane//
		/*const ClippingBoxSettings& settings = controller.getContext().getClippingSettings();
		Clipping* box = new Clipping("Clipping Box", controller.getContext().getCurrentProject()->getNextUserId(ElementType::Box), controller.getContext().getActiveAuthor());
		box->setColor(settings.color);
		glm::dvec3 boxSize(3.0, 3.0, 0.02);
		box->setSize(boxSize);
		glm::dvec3 boxOrientation;
		if (normalVector[0] > 0)
		{
			boxOrientation = glm::dvec3(0.0, atan(sqrt(normalVector[0] * normalVector[0] + normalVector[1] * normalVector[1]) / normalVector[2]), atan(normalVector[1] / normalVector[0]));
		}
		else
		{
			boxOrientation = glm::dvec3(0.0, -atan(sqrt(normalVector[0] * normalVector[0] + normalVector[1] * normalVector[1]) / normalVector[2]), atan(normalVector[1] / normalVector[0]));

		}


		box->setOrientation(boxOrientation);

		box->setCenter(m_points[1]);

		box->setVisible(true);
		//box->setActive(false);
		time_t timeNow;
		box->setTime(time(&timeNow));
		controller.getControlListener()->notifyUIControl(new control::function::clipping::CreateClippingBox(box));*/
		
	}
	else
		error = TEXT_POINTTOPLANE_FAILED;

	m_clickResults.clear();
	return waitForNextPoint(controller, error);
}


bool ContextPointToPlane3Measure::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextPointToPlane3Measure::getType() const
{
	return (ContextType::pointToPlane3);
}
