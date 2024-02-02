#include "controller/functionSystem/ContextCylinderToCylinderMeasure.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/controls/ControlFunction.h"

#include "pointCloudEngine/TlScanOverseer.h"
#include "pointCloudEngine/MeasureClass.h"

#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"

#include "models/3d/Graph/PipeToPipeMeasureNode.h"
#include "models/3d/Graph/CylinderNode.h"
#include "models/3d/Graph/OpenScanToolsGraphManager.hxx"

#include "utils/Logger.h"

#include <glm/gtx/quaternion.hpp>

ContextCylinderToCylinderMeasure::ContextCylinderToCylinderMeasure(const ContextId& id)
	: ARayTracingContext(id)
{
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_CYLINDERTOCYLINDER_START });
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_CYLINDERTOCYLINDER_NEXT });
}

ContextCylinderToCylinderMeasure::~ContextCylinderToCylinderMeasure()
{
}

ContextState ContextCylinderToCylinderMeasure::start(Controller& controller)
{
	return ARayTracingContext::start(controller);
}

ContextState ContextCylinderToCylinderMeasure::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
    return m_state;
}

ContextState ContextCylinderToCylinderMeasure::launch(Controller& controller)
{
    // --- Ray Tracing ---
    if(!ARayTracingContext::getNextPosition(controller))
        return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

	controller.updateInfo(new GuiDataTmpMessage(TEXT_LUCAS_SEARCH_ONGOING, 0));

    double cylinder2Radius;
    glm::dvec3 cylinder2Center, cylinder2Direction;
    glm::dvec3 cylinder1AxisPoint, cylinder2AxisPoint, point1onAxis2, point2onAxis1, point2onAxis2;

    if (m_clickResults.size() == 1)
    {
        // Try to fit the first cylinder and return
        QString error;
        if (!findCylinder(controller, m_clickResults[0].position, m_cylinder1Radius, m_cylinder1Center, m_cylinder1Direction))
        {
            error = TEXT_CYLINDER_NOT_FOUND;
            m_clickResults.clear();
        }
        return waitForNextPoint(controller, error);
    }
    else
    {
        // Try to fit the second cylinder and continue
        if (!findCylinder(controller, m_clickResults[1].position, cylinder2Radius, cylinder2Center, cylinder2Direction))
        {
            m_clickResults.pop_back();
            return waitForNextPoint(controller, TEXT_CYLINDER_NOT_FOUND);
        }
    }

    // ------------
	cylinder1AxisPoint = MeasureClass::projectPointToLine(m_clickResults[0].position, m_cylinder1Direction, m_cylinder1Center);
	cylinder2AxisPoint = MeasureClass::projectPointToLine(cylinder1AxisPoint, cylinder2Direction, cylinder2Center);
	point1onAxis2 = MeasureClass::projectPointToLine(m_clickResults[0].position, cylinder2Direction, cylinder2Center);
	point2onAxis1 = MeasureClass::projectPointToLine(m_clickResults[1].position, m_cylinder1Direction, m_cylinder1Center);
	point2onAxis2 = MeasureClass::projectPointToLine(m_clickResults[1].position, cylinder2Direction, cylinder2Center);
	glm::dvec3 point1onAxis1 = cylinder1AxisPoint;
	glm::dvec3 measurePoint1(cylinder1AxisPoint), measurePoint2(point1onAxis2);
    glm::dvec3 correctedCenter1, correctedCenter2;
    double lenght1, lenght2;
	//case for non parallel pipes (skip for parallel pipes)
	double S = glm::dot(m_cylinder1Direction, cylinder2Direction);
	bool isParallel = abs(S) > 0.996;
	if (!isParallel)
	{
		measurePoint1 = cylinder1AxisPoint + m_cylinder1Direction * ((glm::dot(cylinder2AxisPoint - cylinder1AxisPoint, m_cylinder1Direction - S * cylinder2Direction)) / (1 - S * S));
		measurePoint2 = cylinder2AxisPoint + cylinder2Direction * ((glm::dot(cylinder2AxisPoint - cylinder1AxisPoint, S * m_cylinder1Direction - cylinder2Direction)) / (1 - S * S));
		cylinder1AxisPoint = measurePoint1;
		cylinder2AxisPoint = measurePoint2;
	}

    if (isParallel)
    {
        correctedCenter1 = 0.5 * (cylinder1AxisPoint + point2onAxis1);
        correctedCenter2 = 0.5 * (cylinder2AxisPoint + point2onAxis2);
        lenght1 = 0.5 * glm::length(cylinder1AxisPoint - point2onAxis1);
        lenght2 = 0.5 * glm::length(cylinder2AxisPoint - point2onAxis2);
    }
    else
    {
        correctedCenter1 = 0.5 * (measurePoint1 + point1onAxis1);
        correctedCenter2 = 0.5 * (measurePoint2 + point2onAxis2);
        lenght1 = (glm::length(measurePoint1 - point1onAxis1));
        lenght2 = (glm::length(point2onAxis2 - measurePoint2));
    }

    createCylinder(controller, m_cylinder1Radius, correctedCenter1, m_cylinder1Direction, lenght1);
    createCylinder(controller, cylinder2Radius, correctedCenter2, cylinder2Direction, lenght2);

	//create measure
    SafePtr<PipeToPipeMeasureNode> measure = controller.getOpenScanToolsGraphManager().createMeasureNode<PipeToPipeMeasureNode>();
    WritePtr<PipeToPipeMeasureNode> wMeasure = measure.get();
    if (!wMeasure)
    {
        m_clickResults.clear();
        return waitForNextPoint(controller);
    }
    wMeasure->setDefaultData(controller);

    wMeasure->setPipe1Center(cylinder1AxisPoint);
    wMeasure->setPipe2Center(cylinder2AxisPoint);
    wMeasure->setPipe1Diameter(2.f * m_cylinder1Radius);
    wMeasure->setPipe2Diameter(2.f * cylinder2Radius);
    wMeasure->setCenterP1ToAxeP2((float)glm::length(cylinder1AxisPoint - cylinder2AxisPoint));
    wMeasure->setPipe2CenterToProj((float)glm::length(cylinder2Center - cylinder2AxisPoint));
    wMeasure->setProjPoint(cylinder2AxisPoint);
	glm::dvec3 freeD = cylinder1AxisPoint - cylinder2AxisPoint;
	freeD = freeD / glm::length(freeD);
	freeD = freeD * (glm::length(cylinder1AxisPoint - cylinder2AxisPoint) - m_cylinder1Radius - cylinder2Radius);
    wMeasure->setFreeDist((float)glm::length(freeD));
    wMeasure->setFreeDistVertical((float)abs(freeD.z));
	freeD.z = 0;
    wMeasure->setFreeDistHorizontal((float)glm::length(freeD));

    wMeasure->setTotalFootprint((float)(glm::length(cylinder1AxisPoint - cylinder2AxisPoint) + m_cylinder1Radius + cylinder2Radius));

	glm::dvec3 P1ToP2 = cylinder2AxisPoint - cylinder1AxisPoint;
    wMeasure->setP1ToP2Vertical((float)abs(P1ToP2[2]));
	P1ToP2[2] = 0;
    wMeasure->setP1ToP2Horizontal((float)glm::length(P1ToP2));
    wMeasure->setVisible(true);

    controller.getControlListener()->notifyUIControl(new control::function::AddNodes(measure));

    m_clickResults.clear();
    return waitForNextPoint(controller);
}


bool ContextCylinderToCylinderMeasure::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextCylinderToCylinderMeasure::getType() const
{
	return (ContextType::cylinderToCylinder);
}

bool ContextCylinderToCylinderMeasure::findCylinder(Controller& controller, const glm::dvec3& seedPoint, double& _cylinderRadius, glm::dvec3& _cylinderCenter, glm::dvec3& _cylinderDir)
{
    OpenScanToolsGraphManager& graphManager = controller.getOpenScanToolsGraphManager();
    ClippingAssembly clippingAssembly;
    graphManager.getClippingAssembly(clippingAssembly, true, false);

    controller.updateInfo(new GuiDataTmpMessage(TEXT_LUCAS_SEARCH_ONGOING, 0));

    TlScanOverseer::setWorkingScansTransfo(graphManager.getVisiblePointCloudInstances(m_panoramic, true, true));

    if (!TlScanOverseer::getInstance().fitCylinder(seedPoint, 0.4, 0.002, _cylinderRadius, _cylinderDir, _cylinderCenter, FitCylinderMode::robust, clippingAssembly))
    {
        controller.updateInfo(new GuiDataTmpMessage(TEXT_CYLINDERTOPLANE_FAILED));
        return false;
    }

    return true;
}

void ContextCylinderToCylinderMeasure::createCylinder(Controller& controller, double radius, glm::dvec3 correctedCenter, glm::dvec3 direction, double lenght)
{
    OpenScanToolsGraphManager& graphManager = controller.getOpenScanToolsGraphManager();
    //create cylinders
    SafePtr<CylinderNode> cylinder = make_safe<CylinderNode>(radius);
    WritePtr<CylinderNode> wCylinder = cylinder.get();
    if (!wCylinder)
    {
        assert(false);
        return;
    }

    wCylinder->setDefaultData(controller);

    TransformationModule mod;
    mod.setRotation(glm::dquat(glm::rotation(glm::dvec3(0.0, 0.0, 1.0), direction)));
    mod.setPosition(correctedCenter);
    wCylinder->setTransformationModule(mod);
    wCylinder->setLength(lenght);

    controller.getControlListener()->notifyUIControl(new control::function::AddNodes(cylinder));
}