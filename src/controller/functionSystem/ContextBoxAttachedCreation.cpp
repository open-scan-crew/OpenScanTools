#include "controller/functionSystem/ContextBoxAttachedCreation.h"
#include "controller/controls/ControlFunction.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "utils/Logger.h"

#include "models/graph/BoxNode.h"
#include "gui/texts/RayTracingTexts.hpp"
#include "gui/texts/ContextTexts.hpp"
#include "gui/Texts.hpp"

ContextCreateBoxAttached3Points::ContextCreateBoxAttached3Points(const ContextId& id)
	: AContextCreateAttached(id)
{}

ContextCreateBoxAttached3Points::~ContextCreateBoxAttached3Points()
{}

void ContextCreateBoxAttached3Points::createObject(Controller& controller, TransformationModule& transfo)
{
	FUNCLOG << "ContextCreateBoxAttached3Points createObject" << LOGENDL;
	const ClippingBoxSettings& settings = controller.getContext().getClippingSettings();
	SafePtr<BoxNode> box = make_safe<BoxNode>(true);
	WritePtr<BoxNode> wBox = box.get();
	if (!wBox)
	{
		assert(false);
		return;
	}

	wBox->setDefaultData(controller);
	wBox->setTransformationModule(transfo);

	controller.getControlListener()->notifyUIControl(new control::function::AddNodes(box));
	controller.updateInfo(new GuiDataTmpMessage(TEXT_CLIPPINGBOX_CREATION_DONE));
}

ContextType ContextCreateBoxAttached3Points::getType() const
{
	return ContextType::clippingBoxAttached3Points;
}

bool ContextCreateBoxAttached3Points::canAutoRelaunch() const
{
	return true;
}

ContextCreateBoxAttached2Points::ContextCreateBoxAttached2Points(const ContextId& id)
	: ARayTracingContext(id)
{
	m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_RAYTRACING_DEFAULT_1 });
	m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_RAYTRACING_DEFAULT_2 });
}

ContextCreateBoxAttached2Points::~ContextCreateBoxAttached2Points()
{
}

ContextState ContextCreateBoxAttached2Points::feedMessage(IMessage* message, Controller& controller)
{
	ARayTracingContext::feedMessage(message, controller);
	return m_state;
}

ContextState ContextCreateBoxAttached2Points::launch(Controller& controller)
{
	// --- Ray Tracing ---
	ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
	// -!- Ray Tracing -!-

	FUNCLOG << "ContextCreateBoxAttached2Points createObject" << LOGENDL;

	glm::dvec3 P0 = m_clickResults[0].position;
	glm::dvec3 P1 = m_clickResults[1].position;
	glm::dvec3 D(P1 - P0);

	if (D.x == 0. && D.y == 0.)
	{
		m_clickResults.clear();
		return waitForNextPoint(controller, TEXT_USER_ORIENTATION_PROPERTIES_SAME_POINT_AXIS.arg(QObject::tr("vertical axis")));
	}

	TransformationModule mod;
	double angle = std::atan2(D.y, D.x);
	mod.setRotation(glm::dquat(glm::dvec3(0.0, 0.0, angle)));

	const ClippingBoxSettings& settings = controller.getContext().CgetClippingSettings();
	double d = glm::distance(glm::dvec2(P0), glm::dvec2(P1));
	mod.setScale(glm::dvec3(d, settings.size.y, settings.size.z) / 2.);

	mod.setPosition((P1 + P0) / 2.);

	SafePtr<BoxNode> box = make_safe<BoxNode>(true);
	WritePtr<BoxNode> wBox = box.get();
	if (!wBox)
	{
		m_clickResults.clear();
		return waitForNextPoint(controller);
	}

	wBox->setDefaultData(controller);
	wBox->setTransformationModule(mod);

	controller.getControlListener()->notifyUIControl(new control::function::AddNodes(box));
	controller.updateInfo(new GuiDataTmpMessage(TEXT_CLIPPINGBOX_CREATION_DONE));

	m_clickResults.clear();
	return waitForNextPoint(controller);
}

ContextType ContextCreateBoxAttached2Points::getType() const
{
	return ContextType::clippingBoxAttached2Points;
}

bool ContextCreateBoxAttached2Points::canAutoRelaunch() const
{
	return true;
}
