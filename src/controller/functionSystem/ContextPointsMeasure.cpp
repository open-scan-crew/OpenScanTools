#include "controller/functionSystem/ContextPointsMeasure.h"
#include "controller/controls/ControlFunctionMeasure.h"
#include "controller/controls/ControlFunction.h"
#include "controller/messages/ClickMessage.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "controller/functionSystem/FunctionManager.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataMeasure.h"
#include "gui/texts/ContextTexts.hpp"

#include "models/3d/Graph/PolylineMeasureNode.h"
#include "models/3d/Graph/GraphManager.hxx"
#include "utils/Logger.h"
#include "magic_enum/magic_enum.hpp"

ContextPointsMeasure::ContextPointsMeasure(const ContextId& id)
	: ARayTracingContext(id)
	, m_isPolylineCreated(false)
	, m_lastPointInd(-1)
{
    m_usages.push_back({ true, { ElementType::Tag, ElementType::Point }, TEXT_POINT_MEASURE_START });
    m_usages.push_back({ true, { ElementType::Tag, ElementType::Point }, TEXT_POINT_MEASURE_NEXT_POINT });
}

ContextPointsMeasure::~ContextPointsMeasure()
{}

ContextState ContextPointsMeasure::start(Controller& controller)
{
	return ARayTracingContext::start(controller);
}

ContextState ContextPointsMeasure::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);

	switch (message->getType())
	{
		case IMessage::MessageType::UNDO:
			if (m_lastPointInd > 1)
				m_lastPointInd--;
			else if (m_lastPointInd == 1)
				m_lastPointInd = -1;
			break;
		case IMessage::MessageType::REDO:
			if (m_points.size() > m_lastPointInd + 1)
			{
				if (m_lastPointInd == -1)
					m_lastPointInd = 1;
				else
					m_lastPointInd++;
			}
			break;
		default:
			FUNCLOG << "wrong message type (" << magic_enum::enum_name<IMessage::MessageType>(message->getType())<< ")" << LOGENDL;
			break;
	}
	return (m_state);
}

ContextState ContextPointsMeasure::launch(Controller& controller)
{
    // --- Ray Tracing ---
	ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

	{
		ReadPtr<PolylineMeasureNode> readPoly = m_polylineToEdit.cget();
		if (m_isPolylineCreated && (!readPoly || readPoly->isDead()))
		{
			m_isPolylineCreated = false;
			m_lastPointInd = -1;
			m_clickResults.pop_front();
			return waitForNextPoint(controller);
		}
	}

	m_state = ContextState::running;
	uint64_t size(m_clickResults.size());

	m_points.resize(m_lastPointInd + 1);
	if (m_points.empty())
	{
		m_points.push_back(m_clickResults[size - 2].position);
		m_lastPointInd = 0;
	}

	Measure newMeasure;
	newMeasure.origin = m_points[m_lastPointInd];

	m_points.push_back(m_clickResults[size - 1].position);
	m_lastPointInd = m_points.size() - 1;

	applyPolylineOption(newMeasure.origin, m_points[m_lastPointInd], controller.getContext().getPolyLineOptions());

	newMeasure.final = m_points[m_lastPointInd];

	{
		ReadPtr<PolylineMeasureNode> readPoly = m_polylineToEdit.cget();
		if (!readPoly || readPoly->isDead())
		{
			m_polylineToEdit = controller.getGraphManager().createMeasureNode<PolylineMeasureNode>();
			WritePtr<PolylineMeasureNode> wMeasure = m_polylineToEdit.get();
			m_isPolylineCreated = true;

			if (wMeasure)
			{
				wMeasure->setDefaultData(controller);
				wMeasure->setMeasures({ newMeasure });
				controller.getControlListener()->notifyUIControl(new control::function::AddNodes(m_polylineToEdit));
			}
		}
		else
			controller.getControlListener()->notifyUIControl(new control::function::measure::AddMeasureToPolylineMeasure(m_polylineToEdit, newMeasure, m_id));
	}

	m_clickResults.pop_front();
	return waitForNextPoint(controller);
}

bool ContextPointsMeasure::canAutoRelaunch() const
{
	return true;
}

ContextType ContextPointsMeasure::getType() const
{
	return ContextType::pointsMeasure;
}

void ContextPointsMeasure::applyPolylineOption(const glm::dvec3& origin, glm::dvec3& next, const PolyLineOptions& options)
{
	if (!options.activeOption)
		return;

	double cosUO = cos(options.decalOrientation);
	double sinUO = sin(options.decalOrientation);

	double deplX = next.x - origin.x;
	double deplY = next.y - origin.y;

	switch (options.currentLock)
	{
		case PolyLineLock::LockX:
		{
			next.x -= (deplX * cosUO + deplY * sinUO) * cosUO;
			next.y -= (deplX * cosUO + deplY * sinUO) * sinUO;
		}
		break;
		case PolyLineLock::LockY:
		{
			next.x -= (deplY * cosUO - deplX * sinUO) * (-sinUO);
			next.y -= (deplY * cosUO - deplX * sinUO) * cosUO;
		}
		break;
		case PolyLineLock::LockZ:
		{
			next.z = origin.z;
		}
		break;

	}

}
