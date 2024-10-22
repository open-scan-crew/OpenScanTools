#include "controller/functionSystem/ContextSimpleMeasure.h"
#include "gui/texts/ContextTexts.hpp"
#include "gui/texts/DefaultNameTexts.hpp"
#include "controller/Controller.h"
#include "controller/ControlListener.h" // forward declaration
#include "controller/controls/ControlFunction.h"

#include "models/graph/SimpleMeasureNode.h"
#include "models/graph/GraphManager.hxx"


ContextSimpleMeasure::ContextSimpleMeasure(const ContextId& id)
	: ContextPointsMeasure(id)
{
    m_usages.clear();
    m_usages.push_back({ true, {ElementType::Tag, ElementType::Point}, TEXT_POINT_MEASURE_START });
    m_usages.push_back({ true, {ElementType::Tag, ElementType::Point}, TEXT_POINT_MEASURE_NEXT_POINT });
}

ContextSimpleMeasure::~ContextSimpleMeasure()
{}

ContextState ContextSimpleMeasure::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

	uint64_t size(m_clickResults.size());
	Measure newMeasure;
	newMeasure.origin = m_clickResults[size - 2].position;
	newMeasure.final = m_clickResults[size - 1].position;

	SafePtr<SimpleMeasureNode> measure = controller.getGraphManager().createMeasureNode<SimpleMeasureNode>();
	WritePtr<SimpleMeasureNode> wMeasure = measure.get();
	if (!wMeasure)
	{
		assert(false);
		m_clickResults.clear();
		return waitForNextPoint(controller);
	}

	wMeasure->setDefaultData(controller);
	wMeasure->setMeasure(newMeasure);

	controller.getControlListener()->notifyUIControl(new control::function::AddNodes(measure));

	m_clickResults.clear();
    return waitForNextPoint(controller);
}

ContextType ContextSimpleMeasure::getType() const
{
	return ContextType::simpleMeasure;
}
