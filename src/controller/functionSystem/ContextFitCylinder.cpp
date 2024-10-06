#include "controller/functionSystem/ContextFitCylinder.h"
#include "controller/controls/ControlFunction.h"
#include "controller/controls/ControlCylinderEdition.h"
#include "controller/messages/PipeMessage.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/controls/ControlFunctionClipping.h"
#include "utils/Logger.h"

#include "models/graph/CylinderNode.h"
#include "models/graph/GraphManager.hxx"

#include <glm/gtx/quaternion.hpp>

ContextFitCylinder::ContextFitCylinder(const ContextId& id)
    : ARayTracingContext(id)
{
}

ContextFitCylinder::~ContextFitCylinder()
{
}

void ContextFitCylinder::resetClickUsages(Controller& controller)
{
	m_options = controller.getContext().getPipeDetectionOptions();

    m_usages.clear();
    m_clickResults.clear();
    if (m_options.extendMode == PipeDetectionExtendMode::Manual)
    {
        m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_POINTTOCYLINDER_START });
        m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_BIGCYLINDERFIT_FOURTH });
    }
    else
    {
        m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_POINTTOCYLINDER_START });
    }
}

ContextState ContextFitCylinder::start(Controller& controller)
{
	resetClickUsages(controller);
	return ARayTracingContext::start(controller);
}

ContextState ContextFitCylinder::feedMessage(IMessage* message, Controller& controller)
{
	return ARayTracingContext::feedMessage(message, controller);
}

ContextState ContextFitCylinder::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

	FUNCLOG << "ContextFitCylinder launch" << LOGENDL;
	controller.updateInfo(new GuiDataTmpMessage(TEXT_LUCAS_SEARCH_ONGOING, 0));
	bool success = false;

	GraphManager& graphManager = controller.getGraphManager();

    ClippingAssembly clippingAssembly;
	graphManager.getClippingAssembly(clippingAssembly, true, false);

	glm::dvec3 cylinderDirection, cylinderCenter;
	double cylinderRadius, radius(0.4);

	bool isAutoExtend = m_options.extendMode == PipeDetectionExtendMode::Auto;
	bool isManualExtend = m_options.extendMode == PipeDetectionExtendMode::Manual;

    double threshold = m_options.noisy ? 0.004 : 0.002;
    FitCylinderMode mode = m_options.optimized ? FitCylinderMode::robust : FitCylinderMode::fast;

    std::vector<double> heights(2);
    TlScanOverseer::setWorkingScansTransfo(graphManager.getVisiblePointCloudInstances(m_panoramic, true, true));

    // apply the fitting method between "auto extend", "manual extend" and "normal"
    if (isAutoExtend)
    {
        assert(m_clickResults.size());
        success = TlScanOverseer::getInstance().extendCylinder(m_clickResults[0].position, radius, threshold, cylinderRadius, cylinderDirection, cylinderCenter, heights, mode, clippingAssembly);
    }
    else if (isManualExtend)
    {
        assert(m_clickResults.size() >= 2);
        std::vector<glm::dvec3> seedPoints;
        //seedPoints.push_back(0.5*(m_clickResults[0] + m_clickResults[1]));
        //seedPoints.push_back(0.25*m_clickResults[0] + 0.75*m_clickResults[1]);
        //seedPoints.push_back(0.75*m_clickResults[0] + 0.25*m_clickResults[1]);
		seedPoints.push_back(m_clickResults[0].position);
		seedPoints.push_back(m_clickResults[1].position);
        /*for (int i = 0; i < seedPoints.size(); ++i)
        {
            success = TlScanOverseer::getInstance().fitCylinder(seedPoints[i], radius, threshold, cylinderRadius, cylinderDirection, cylinderCenter, mode, clippingAssembly);
            if (success)
                break;
        }*/
		success = TlScanOverseer::getInstance().fitCylinderMultipleSeeds(seedPoints, radius, threshold, cylinderRadius, cylinderDirection, cylinderCenter, mode, clippingAssembly);
        heights[0] = TlScanOverseer::computeHeight(m_clickResults[0].position, cylinderDirection, cylinderCenter);
        heights[1] = TlScanOverseer::computeHeight(m_clickResults[1].position, cylinderDirection, cylinderCenter);
        cylinderCenter = cylinderCenter + 0.5*(heights[0] + heights[1])*cylinderDirection;
		if (!success)
		{
			seedPoints[0] = (0.1*m_clickResults[0].position + 0.9*m_clickResults[1].position);
			seedPoints[1] = (0.9*m_clickResults[0].position + 0.1*m_clickResults[1].position);
			success = TlScanOverseer::getInstance().fitCylinderMultipleSeeds(seedPoints, radius, threshold, cylinderRadius, cylinderDirection, cylinderCenter, mode, clippingAssembly);
			heights[0] = TlScanOverseer::computeHeight(m_clickResults[0].position, cylinderDirection, cylinderCenter);
			heights[1] = TlScanOverseer::computeHeight(m_clickResults[1].position, cylinderDirection, cylinderCenter);
			cylinderCenter = cylinderCenter + 0.5*(heights[0] + heights[1])*cylinderDirection;
		}
		if (!success)
		{
			seedPoints[0] = (0.25*m_clickResults[0].position + 0.75*m_clickResults[1].position);
			seedPoints[1] = (0.75*m_clickResults[0].position + 0.25*m_clickResults[1].position);
			success = TlScanOverseer::getInstance().fitCylinderMultipleSeeds(seedPoints, radius, threshold, cylinderRadius, cylinderDirection, cylinderCenter, mode, clippingAssembly);
			heights[0] = TlScanOverseer::computeHeight(m_clickResults[0].position, cylinderDirection, cylinderCenter);
			heights[1] = TlScanOverseer::computeHeight(m_clickResults[1].position, cylinderDirection, cylinderCenter);
			cylinderCenter = cylinderCenter + 0.5*(heights[0] + heights[1])*cylinderDirection;
		}
    }
    else
    {
        assert(m_clickResults.size());
        success = TlScanOverseer::getInstance().fitCylinder(m_clickResults[0].position, radius, threshold, cylinderRadius, cylinderDirection, cylinderCenter, mode, clippingAssembly);
		cylinderCenter = cylinderCenter + cylinderDirection * glm::dot(cylinderDirection, m_clickResults[0].position - cylinderCenter);
	}

	QString abortPipeErrorText = QString();
    // Check fitting success and send message to GUI
	if (!success || m_state != ContextState::running)
		abortPipeErrorText = TEXT_CYLINDER_NOT_FOUND;
	else if (m_options.insulatedThickness >= cylinderRadius)
		abortPipeErrorText = TEXT_CYLINDER_INSULATED_THICKNESS_EXCEEDS;

	if (!abortPipeErrorText.isEmpty())
	{
		if (m_state == ContextState::running)
		{
			resetClickUsages(controller);
			return waitForNextPoint(controller, abortPipeErrorText);
		}
		else
			return m_state == ContextState::done ? ARayTracingContext::validate(controller) : ((m_state == ContextState::abort) ? ARayTracingContext::abort(controller) : m_state);
	}

	controller.updateInfo(new GuiDataTmpMessage(QString(TEXT_CYLINDER_FOUND).arg(2 * cylinderRadius, cylinderCenter.x, cylinderCenter.y, cylinderCenter.z)));

	SafePtr<CylinderNode> cylinder = make_safe<CylinderNode>(cylinderRadius);
	WritePtr<CylinderNode> wCyl = cylinder.get();
	if (!wCyl)
	{
		resetClickUsages(controller);
		return waitForNextPoint(controller);
	}

	wCyl->setDefaultData(controller);
	
	TransformationModule mod;
	mod.setPosition(cylinderCenter);
	mod.setRotation(glm::dquat(glm::rotation(glm::dvec3(0.0, 0.0, 1.0), cylinderDirection)));
	wCyl->setTransformationModule(mod);

    if (isAutoExtend || isManualExtend)
    {
		wCyl->setLength(abs(heights[1] - heights[0]));
    }
    else
    {
        double length;
        if (cylinderRadius < 0.015)
            length = 3 * cylinderRadius;
        else if (cylinderRadius > 0.075)
            length = cylinderRadius;
        else length = cylinderRadius * (2 * (0.015 - cylinderRadius) / 0.06 + 3);
		wCyl->setLength(abs(2 * length));
    }

	controller.getControlListener()->notifyUIControl(new control::function::AddNodes(cylinder));
    
	resetClickUsages(controller);
	return waitForNextPoint(controller);
}

bool ContextFitCylinder::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextFitCylinder::getType() const
{
	return (ContextType::fitCylinder);
}
