#include "controller/controls/ControlFunctionMeasure.h"
#include "controller/Controller.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/messages/UndoRedoMessages.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataMeasure.h"

#include "models/graph/PolylineMeasureNode.h"

#include "utils/Logger.h"

namespace control::function::measure
{

	/*
	** AddMeasureToPolylineMeasure
	*/
			
	AddMeasureToPolylineMeasure::AddMeasureToPolylineMeasure(SafePtr<PolylineMeasureNode> polyline, const Measure& measure, ContextId contextId)
		: m_polyline(polyline)
		, m_measure(measure)
		, m_contextId(contextId)
		, m_isRedo(false)
	{}

	AddMeasureToPolylineMeasure::~AddMeasureToPolylineMeasure()
	{}

	void AddMeasureToPolylineMeasure::doFunction(Controller& controller)
	{
		WritePtr<PolylineMeasureNode> writePoly = m_polyline.get();
		if (!writePoly)
		{
			CONTROLLOG << "control::function::measure::AddMeasureToPolylineMeasure do : object null" << LOGENDL;
			return;
		}
		writePoly->addMeasure(m_measure);

		if (m_isRedo)
		{
			RedoMessage message;
			controller.getFunctionManager().feedMessageToSpecificContext(controller, &message, m_contextId);
		}

		controller.updateInfo(new GuiDataObjectSelected(m_polyline));
		controller.updateInfo(new GuiDataPolylineMeasureUpdated(m_polyline));

		CONTROLLOG << "ccontrol::function::measure::AddMeasureToPolylineMeasure do elemid " << writePoly->getId() << LOGENDL;
	}

	bool AddMeasureToPolylineMeasure::canUndo() const 
	{
		return (true);
	}

	void AddMeasureToPolylineMeasure::undoFunction(Controller& controller)
	{
		WritePtr<PolylineMeasureNode> writePoly = m_polyline.get();
		if (!writePoly)
		{
			CONTROLLOG << "control::function::measure::AddMeasureToPolylineMeasure undo : object null" << LOGENDL;
			return;
		}
		writePoly->removeMeasureBack();
		m_isRedo = true;

		UndoMessage message;
		controller.getFunctionManager().feedMessageToSpecificContext(controller, &message, m_contextId);

		controller.updateInfo(new GuiDataObjectSelected(m_polyline));
		controller.updateInfo(new GuiDataPolylineMeasureUpdated(m_polyline));

		CONTROLLOG << "ccontrol::function::measure::AddMeasureToPolylineMeasure undo elemid " << writePoly->getId() << LOGENDL;
	}

	ControlType AddMeasureToPolylineMeasure::getType() const
	{
		return (ControlType::addMeasureToPolylineMeasure);
	}
}
