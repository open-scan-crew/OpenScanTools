#include "controller/functionSystem/ContextExperiment.h"
#include "controller/Controller.h"
#include "controller/ControlListener.h" // forward declaration
#include "pointCloudEngine/TlScanOverseer.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"

#include "models/graph/GraphManager.h"


ContextExperiment::ContextExperiment(const ContextId& id)
	: ARayTracingContext(id)
	, m_TEMPORARYsubFunctionID(-1)
{
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, "" });
}

ContextExperiment::~ContextExperiment()
{
}

ContextState ContextExperiment::start(Controller& controller)
{
    return ARayTracingContext::start(controller);
}

ContextState  ContextExperiment::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
	return (m_state);
}

ContextState ContextExperiment::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

	controller.updateInfo(new GuiDataTmpMessage(TEXT_LUCAS_SEARCH_ONGOING, 0));
	bool success = false;

	double beamHeight(0);


    TlScanOverseer::setWorkingScansTransfo(controller.getGraphManager().getVisiblePointCloudInstances(xg::Guid(), true, true));

	TlScanOverseer::getInstance().estimateNormals();
	/*if (manager->computeBeamHeight(m_point,beamHeight))
	{
		controller.updateInfo(new GuiDataTmpMessage("Beam Height : " + std::to_string(beamHeight)));
	}
	else
	{
		controller.updateInfo(new GuiDataTmpMessage(""));
		return (m_state = ContextState::waiting_for_input);
	}
	FUNCLOG << "ContextExperiment launch" << LOGENDL
	Tag *tag = new Tag("tag", controller.getContext().getCurrentProject()->getNextUserId(),
		controller.getContext().getActiveAuthor(),
		controller.getContext().getTemplates().at(controller.getContext().getCurrentTemplate()));
	tag->setName(std::string(""));
	tag->setColor(controller.getContext().getActiveColor());
	tag->setDescription("Beam Height : " + std::to_string(beamHeight));
	tag->setVisible(true);
	tag->setPosition(m_point);
	time_t timeNow;
	tag->setTime(time(&timeNow));
	controller.getControlListener()->notifyUIControl(new control::function::tag::CreateTag(tag));*/
	m_clickResults.clear();
	return waitForNextPoint(controller);
}

bool ContextExperiment::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextExperiment::getType() const
{
	return (ContextType::experiment);
}
