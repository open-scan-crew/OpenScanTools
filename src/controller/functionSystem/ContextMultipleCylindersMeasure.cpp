#include "controller/functionSystem/ContextMultipleCylindersMeasure.h"
#include "controller/messages/IMessage.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"
#include "controller/Controller.h"
//#include "controller/IControlListener.h"
#include "pointCloudEngine/TlScanOverseer.h"

#include "models/graph/GraphManager.h"

ContextMultipleCylinders::ContextMultipleCylinders(const ContextId& id)
	: ARayTracingContext(id)
{
    // No usage found
}

ContextMultipleCylinders::~ContextMultipleCylinders()
{
}

ContextState ContextMultipleCylinders::start(Controller& controller)
{
	return ARayTracingContext::start(controller);
}

ContextState ContextMultipleCylinders::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
    return m_state;
}

ContextState ContextMultipleCylinders::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

	controller.updateInfo(new GuiDataTmpMessage(TEXT_LUCAS_SEARCH_ONGOING, 0));

	GraphManager& graphManager = controller.getGraphManager();

    ClippingAssembly clippingAssembly;
	graphManager.getClippingAssembly(clippingAssembly, true, false);
	std::vector<glm::dvec3> cylinderCenters, cylinderDirections, seedPoints;
	std::vector<double> cylinderRadii;
    TlScanOverseer::setWorkingScansTransfo(graphManager.getVisiblePointCloudInstances(m_panoramic, true, true));

	bool temp = TlScanOverseer::getInstance().beginMultipleCylinders(cylinderCenters, cylinderDirections, cylinderRadii, clippingAssembly, seedPoints);
	controller.updateInfo(new GuiDataTmpMessage(TEXT_LUCAS_SEARCH_DONE));

	for (int i = 0; i < (int)cylinderCenters.size(); i++)
	{
		/*Tag* tag = new Tag(controller.getContext());

		tag->setName(std::wstring(L"pipe" + std::to_wstring(i)));
		tag->setDescription(L"Diameter = " + std::to_wstring(2 * cylinderRadii[i]) + L"\n Direction: " + std::to_wstring(cylinderDirections[i].x) + L" " + std::to_wstring(cylinderDirections[i].y) + L" " + std::to_wstring(cylinderDirections[i].z));
		tag->setVisible(true);
		tag->setPosition(cylinderCenters[i]);
		time_t timeNow1;
		tag->setCreationTime(time(&timeNow1));
		//controller.getControlListener()->notifyUIControl(new control::function::tag::CreateTag(tag));

		/*Tag *tag1 = new Tag("tag", controller.getContext().getCurrentProject()->getNextUserId(ElementType::Tag),
			controller.getContext().getActiveAuthor(),
			controller.getContext().getTemplates().at(controller.getContext().getCurrentTemplate()));
		tag1->setName(std::string("seedPoint" + std::to_string(i)));
		tag1->setColor(Color32(0,255,0));
		tag1->setDescription("Diameter = " + std::to_string(2 * cylinderRadii[i]) + "\n Direction: " + std::to_string(cylinderDirections[i].x) + " " + std::to_string(cylinderDirections[i].y) + " " + std::to_string(cylinderDirections[i].z));
		tag1->setVisible(true);
		tag1->setPosition(seedPoints[i]);
		//time_t timeNow1;
		tag1->setTime(time(&timeNow1));
		controller.getControlListener()->notifyUIControl(new control::function::tag::CreateTag(tag1));*/
	}

	m_clickResults.clear();
	return waitForNextPoint(controller);
}

bool ContextMultipleCylinders::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextMultipleCylinders::getType() const
{
	return (ContextType::multipleCylinders);
}
