#include "controller/functionSystem/ContextColumnTilt.h"
#include "controller/controls/ControlFunctionTag.h"
#include "controller/messages/FullClickMessage.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "controller/functionSystem/FunctionManager.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "controller/controls/ControlFunction.h"
#include "utils/ProjectColor.hpp"
#include "utils/Logger.h"

#include "models/3d/Graph/OpenScanToolsGraphManager.hxx"
#include "models/3d/Graph/ColumnTiltMeasureNode.h"

ContextColumnTilt::ContextColumnTilt(const ContextId& id)
	: ARayTracingContext(id)
{
    // Set the description of the points needed for the processing
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_COLUMNTILIT_FIRST_POINT });
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_COLUMNTILIT_SECOND_POINT });
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_COLUMNTILIT_THIRD_POINT });
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_COLUMNTILIT_FOURTH_POINT });
}

ContextColumnTilt::~ContextColumnTilt()
{
}

ContextState ContextColumnTilt::start(Controller& controller)
{
	return ARayTracingContext::start(controller);
}


ContextState ContextColumnTilt::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
    return (m_state);
}

ContextState ContextColumnTilt::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

	FUNCLOG << "ContextColumnTilt launch" << LOGENDL;
	double offset, ratio;
	glm::dvec3 topPoint, bottomPoint;
	
	OpenScanToolsGraphManager& graphManager = controller.getOpenScanToolsGraphManager();
    TlScanOverseer::setWorkingScansTransfo(graphManager.getVisiblePointCloudInstances(m_panoramic, true, true));

    // Human readable references
    assert(m_clickResults.size() >= 4);
    glm::dvec3& wallPoint_0 = m_clickResults[0].position;
    glm::dvec3& wallPoint_1 = m_clickResults[1].position;
    glm::dvec3& columnPoint_0 = m_clickResults[2].position;
    glm::dvec3& columnPoint_1 = m_clickResults[3].position;

	if (TlScanOverseer::getInstance().columnOffset(m_lastCameraPos, wallPoint_0, wallPoint_1, columnPoint_0, columnPoint_1, offset, ratio))
	{
		controller.updateInfo(new GuiDataTmpMessage(QString(TEXT_COLUMNTILT_DONE).arg(offset,ratio)));
	}
	if (columnPoint_0.z > columnPoint_1.z)
	{
		topPoint = columnPoint_0;
		bottomPoint = columnPoint_1;
	}
	else
	{
        topPoint = columnPoint_1;
		bottomPoint = columnPoint_0;
	}
	

	SafePtr<ColumnTiltMeasureNode> measure = make_safe<ColumnTiltMeasureNode>();
	WritePtr<ColumnTiltMeasureNode> wMeasure = measure.get();
	if (!wMeasure)
	{
		m_clickResults.clear();
		return waitForNextPoint(controller);
	}

	wMeasure->setDefaultData(controller);
	wMeasure->setPoint1(wallPoint_0);
	wMeasure->setPoint2(wallPoint_1);
	wMeasure->setBottomPoint(bottomPoint);
	wMeasure->setTopPoint(topPoint);
	wMeasure->setTiltValue((float) offset);
    double tolerance = controller.getContext().cgetProjectInfo().m_columnTiltTolerance;
	wMeasure->setMaxRatio(tolerance);
	wMeasure->setHeight((float)glm::length(topPoint - bottomPoint));
	Reliability strResult;

	strResult = Reliability::reliable;
	wMeasure->setColor(ProjectColor::getColor("RED"));

	if (((int)abs(1 / ratio) > tolerance) || ((int)abs(1 / ratio) > 2000))
	{
		wMeasure->setColor(ProjectColor::getColor("GREEN"));
	}
	if (((int)abs(1 / ratio) > 2000))
		strResult = Reliability::unreliable;
	wMeasure->setRatioSup(RatioSup::no);
	if((int)abs(1 / ratio) < tolerance)
		wMeasure->setRatioSup(RatioSup::yes);

	wMeasure->setResultReliability(strResult);
	wMeasure->setRatio((int)(1/ratio));

	controller.getControlListener()->notifyUIControl(new control::function::AddNodes(measure));

    // Erase the last 2 points. The context will wait for new inputs
    m_clickResults.resize(2);
	return waitForNextPoint(controller);
}

bool ContextColumnTilt::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextColumnTilt::getType() const
{
	return (ContextType::columnTilt);
}
