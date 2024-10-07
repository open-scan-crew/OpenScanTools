#include "controller/functionSystem/ContextPointToPlaneMeasure.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "controller/functionSystem/FunctionManager.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "pointCloudEngine/NormalEstimation.h"
#include "pointCloudEngine/MeasureClass.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"
#include "controller/controls/ControlFunction.h"

#include "utils/Logger.h"
#include "magic_enum/magic_enum.hpp"

#include "models/graph/GraphManager.hxx"
#include "models/graph/PointToPlaneMeasureNode.h"

ContextPointToPlaneMeasure::ContextPointToPlaneMeasure(const ContextId& id)
	: ARayTracingContext(id)
{
    m_usages.push_back({ true, {ElementType::Tag, ElementType::Point}, TEXT_POINTTOPLANE_START });
	// Add the mesh object as source for the plane
    // TODO - Enable to extract a normal from the ray tracing
	m_usages.push_back({ true, { ElementType::Tag, ElementType::Point }, TEXT_POINTTOPLANE_SELECT_PLANE });
}

ContextPointToPlaneMeasure::~ContextPointToPlaneMeasure()
{
}

ContextState ContextPointToPlaneMeasure::start(Controller& controller)
{
	return ARayTracingContext::start(controller);
}

ContextState ContextPointToPlaneMeasure::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
    return m_state;
}

ContextState ContextPointToPlaneMeasure::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

	controller.updateInfo(new GuiDataTmpMessage(TEXT_LUCAS_SEARCH_ONGOING, 0));
	bool success = false;
	GraphManager& graphManager = controller.getGraphManager();
	glm::dvec3 normalVector;

    TlScanOverseer::setWorkingScansTransfo(graphManager.getVisiblePointCloudInstances(m_panoramic, true, true));
    ClippingAssembly clippingAssembly;
	graphManager.getClippingAssembly(clippingAssembly, true, false);
	QString error;

	std::vector<double> plane;
	if (std::isnan(m_clickResults[1].normal.x))
	{
		// Detect the plane from the PC
		if (TlScanOverseer::getInstance().fitPlaneRegionGrowing(m_clickResults[1].position, plane, clippingAssembly))
		{
			normalVector = glm::dvec3(plane[0], plane[1], plane[2]);
			normalVector /= glm::length(normalVector);
		}
	}
	else
	{
		// Extract plane equation from the normal
		normalVector = m_clickResults[1].normal;
		plane.push_back(normalVector.x);
		plane.push_back(normalVector.y);
		plane.push_back(normalVector.z);
		double plane_d = -glm::dot(m_clickResults[1].position, normalVector);
		plane.push_back(plane_d);
	}

	if (plane.size() == 4)
	{
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
		wMeasure->setpointOnPlane(m_clickResults[1].position);
		normalVector /= glm::length(normalVector);
		wMeasure->setNormalToPlane(normalVector);
		wMeasure->setProjPoint(projectedPoint);

		controller.getControlListener()->notifyUIControl(new control::function::AddNodes(measure));

	}
	else
		error = TEXT_POINTTOPLANE_FAILED;

	m_clickResults.clear();
	return waitForNextPoint(controller, error);
}


bool ContextPointToPlaneMeasure::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextPointToPlaneMeasure::getType() const
{
	return (ContextType::pointToPlane);
}
