#include "controller/functionSystem/ContextClippingBoxCreation.h"
#include "controller/controls/ControlFunction.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/IControlListener.h"
//#include "controller/functionSystem/FunctionManager.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"
#include "utils/math/trigo.h"
#include "utils/Logger.h"

#include "models/graph/BoxNode.h"
//#include "models/graph/GraphManager.hxx"

ContextClippingBoxCreation::ContextClippingBoxCreation(const ContextId& id)
	: ARayTracingContext(id)
{
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_CLIPPINGBOX_START });
}

ContextClippingBoxCreation::~ContextClippingBoxCreation()
{}

ContextState ContextClippingBoxCreation::start(Controller& controller)
{
	return  ARayTracingContext::start(controller);
}

ContextState ContextClippingBoxCreation::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
    return (m_state);
}

ContextState ContextClippingBoxCreation::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

	FUNCLOG << "ContextClippingBoxCreation launch" << LOGENDL;
	const ClippingBoxSettings& settings = controller.getContext().getClippingSettings();
	SafePtr<BoxNode> box = make_safe<BoxNode>(true);
	WritePtr<BoxNode> wBox = box.get();
	if (!wBox)
	{
		assert(false);
		return ARayTracingContext::abort(controller);
	}
	
	wBox->setDefaultData(controller);

	if (settings.angleZ)
		wBox->setRotation(tls::math::euler_rad_to_quat({ 0.0, 0.0, settings.angleZ }));
	wBox->setSize(settings.size);
	switch (settings.offset)
	{
	case ClippingBoxOffset::CenterOnPoint:
		wBox->setPosition(m_clickResults[0].position);
		break;
	case ClippingBoxOffset::Topface:
		wBox->setPosition(m_clickResults[0].position - glm::dvec3(0.0, 0.0, settings.size.z / 2.0));
		break;
	case ClippingBoxOffset::BottomFace:
		wBox->setPosition(m_clickResults[0].position + glm::dvec3(0.0, 0.0, settings.size.z / 2.0));
		break;
	}

	controller.getControlListener()->notifyUIControl(new control::function::AddNodes(box));
	controller.updateInfo(new GuiDataTmpMessage(TEXT_CLIPPINGBOX_CREATION_DONE));

	m_clickResults.clear();
	return waitForNextPoint(controller);
}

bool ContextClippingBoxCreation::canAutoRelaunch() const
{
	return true;
}

ContextType ContextClippingBoxCreation::getType() const
{
	return ContextType::clippingBoxCreation;
}
