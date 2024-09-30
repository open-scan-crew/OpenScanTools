#include "controller/functionSystem/ContextBeamBending.h"
#include "controller/controls/ControlFunction.h"
#include "controller/messages/ClickMessage.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "controller/functionSystem/FunctionManager.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "utils/ProjectColor.hpp"
#include "utils/Logger.h"
#include "magic_enum/magic_enum.hpp"
#include "pointCloudEngine/MeasureClass.h"

#include "models/3d/Graph/GraphManager.hxx"
#include "models/3d/Graph/BeamBendingMeasureNode.h"

ContextBeamBending::ContextBeamBending(const ContextId& id)
	: ARayTracingContext(id)
{
    // The context need 2 clicks
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_BEAMBENDING_START });
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_BEAMBENDING_NEXT });
}

ContextBeamBending::~ContextBeamBending()
{
}

ContextState ContextBeamBending::start(Controller& controller)
{
	return ARayTracingContext::start(controller);
}

void ContextBeamBending::resetClickUsages()
{
	m_usages.clear();
	m_clickResults.clear();

	if (m_options == BeamBendingOptions::normal)
	{
		// The context need 2 clicks
		m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_BEAMBENDING_START });
		m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_BEAMBENDING_NEXT });
	}
	if (m_options == BeamBendingOptions::manual)
	{
		//need 2 clicks
		m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_PLANE3_FIRST });
		m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_PLANE3_SECOND });
		m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_POINTTOPLANE_SELECT_PLANE });

	}
}

ContextState ContextBeamBending::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
	switch(message->getType())
	{
	case IMessage::MessageType::BEAMBENDINGMESSAGE:
	{
		BeamBendingMessage* beamBendingMessage = static_cast<BeamBendingMessage*>(message);
		m_options = beamBendingMessage->getOptions();
		if ((m_options == BeamBendingOptions::manual) && ((int)m_clickResults.size() == 2))
		{
			m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_POINTTOPLANE_SELECT_PLANE });
		}
		else
			resetClickUsages();
		break;
	}
	}
    return (m_state);
}

ContextState ContextBeamBending::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

	FUNCLOG << "ContextBeamBending launch" << LOGENDL;
	controller.updateInfo(new GuiDataTmpMessage(TEXT_LUCAS_SEARCH_ONGOING, 0));
	GraphManager& graphManager = controller.getGraphManager();
    ClippingAssembly clippingAssembly;
	graphManager.getClippingAssembly(clippingAssembly, true, false);
	TlScanOverseer::setWorkingScansTransfo(graphManager.getVisiblePointCloudInstances(m_panoramic, true, true));
	glm::dvec3 bendPoint;
	double maxBend, ratio;
	bool reliable(true);
	std::vector<glm::dvec3> globalEndPoints = {m_clickResults[0].position, m_clickResults[1].position };

	if (m_options == BeamBendingOptions::normal)
	{
		if (TlScanOverseer::getInstance().beamBending(globalEndPoints, bendPoint, maxBend, ratio, reliable, clippingAssembly))
		{
			controller.updateInfo(new GuiDataTmpMessage(QString(TEXT_BEAMBENDING_RESULT).arg(maxBend, ratio)));
			SafePtr<BeamBendingMeasureNode> measure = make_safe<BeamBendingMeasureNode>();
			WritePtr<BeamBendingMeasureNode> wMeasure = measure.get();

			if (!wMeasure)
			{
				m_clickResults.clear();
				return waitForNextPoint(controller);
			}

			wMeasure->setDefaultData(controller);
			wMeasure->setPoint1Pos(m_clickResults[0].position);	
			wMeasure->setPoint2Pos(m_clickResults[1].position);
			wMeasure->setBendingValue((float)maxBend);
			wMeasure->setMaxBendingPos(bendPoint);
			wMeasure->setMaxRatio(controller.getContext().cgetProjectInfo().m_beamBendingTolerance);
			wMeasure->setLength((float)glm::length(m_clickResults[0].position - m_clickResults[1].position));
			wMeasure->setRatio((int)(glm::length(m_clickResults[0].position - m_clickResults[1].position) / maxBend));
			Reliability strResult;
			strResult = Reliability::reliable;
			wMeasure->setColor(ProjectColor::getColor("GREEN"));

			wMeasure->setRatioSup(RatioSup::no);
			double tolerance = controller.getContext().cgetProjectInfo().m_beamBendingTolerance;

			if ((int)(glm::length(m_clickResults[0].position - m_clickResults[1].position) / maxBend) < controller.getContext().cgetProjectInfo().m_beamBendingTolerance)
			{
				wMeasure->setRatioSup(RatioSup::yes);
				wMeasure->setColor(ProjectColor::getColor("RED"));
			}
			if ((!reliable) || ((2000 * maxBend) < glm::length(m_clickResults[0].position - m_clickResults[1].position)))
			{
				strResult = Reliability::unreliable;
				wMeasure->setColor(Color32(255, 165, 0)); //ORANGE
			}

			wMeasure->setResultReliability(strResult);

			controller.getControlListener()->notifyUIControl(new control::function::AddNodes(measure));

			m_clickResults.clear();
			return waitForNextPoint(controller);
		}
	}
	else
	{
		bendPoint = m_clickResults[2].position;
		glm::dvec3 beamDir = m_clickResults[1].position - m_clickResults[0].position;
		beamDir /= glm::length(beamDir);
		glm::dvec3 horizontal = glm::cross(glm::dvec3(0.0, 0.0, 1.0), beamDir);
		horizontal /= glm::length(horizontal);
		glm::dvec3 bendDir = glm::cross(horizontal, beamDir);
		bendDir /= glm::length(bendDir);
		std::vector<double> plane(0);
		plane.push_back(bendDir[0]);
		plane.push_back(bendDir[1]);
		plane.push_back(bendDir[2]);
		plane.push_back(-glm::dot(bendDir,m_clickResults[0].position));


		glm::dvec3 projectedPoint = MeasureClass::projectPointToPlane(bendPoint, plane);
		maxBend = glm::length(bendPoint- projectedPoint);
		controller.updateInfo(new GuiDataTmpMessage(QString(TEXT_BEAMBENDING_RESULT).arg(maxBend, ratio)));
		SafePtr<BeamBendingMeasureNode> measure = make_safe<BeamBendingMeasureNode>();
		WritePtr<BeamBendingMeasureNode> wMeasure = measure.get();

		if (!wMeasure)
		{
			m_clickResults.clear();
			return waitForNextPoint(controller);
		}

		wMeasure->setDefaultData(controller);

		wMeasure->setMaxBendingPos(bendPoint);
		wMeasure->setPoint1Pos(m_clickResults[0].position);
		wMeasure->setMaxBendingPos(bendPoint);
		wMeasure->setMaxRatio(controller.getContext().cgetProjectInfo().m_beamBendingTolerance);
		wMeasure->setPoint1Pos(m_clickResults[0].position);
		wMeasure->setLength((float)glm::length(m_clickResults[0].position - m_clickResults[1].position));
		wMeasure->setRatio((int)(glm::length(m_clickResults[0].position - m_clickResults[1].position) / maxBend));
		Reliability strResult;
		strResult = Reliability::reliable;
		wMeasure->setColor(ProjectColor::getColor("GREEN"));

		wMeasure->setRatioSup(RatioSup::no);
		double tolerance = controller.getContext().cgetProjectInfo().m_beamBendingTolerance;

		if ((int)(glm::length(m_clickResults[0].position - m_clickResults[1].position) / maxBend) < controller.getContext().cgetProjectInfo().m_beamBendingTolerance)
		{
			wMeasure->setRatioSup(RatioSup::yes);
			wMeasure->setColor(ProjectColor::getColor("RED"));
		}
		
		wMeasure->setResultReliability(strResult);

		controller.getControlListener()->notifyUIControl(new control::function::AddNodes(measure));

		m_clickResults.pop_back();
		return waitForNextPoint(controller);
	}

	m_clickResults.clear();
	return waitForNextPoint(controller);
}

bool ContextBeamBending::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextBeamBending::getType() const
{
	return (ContextType::beamBending);
}
