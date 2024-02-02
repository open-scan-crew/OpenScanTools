#include "ContextMeshDistance.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "controller/messages/FullClickMessage.h"
#include "controller/controls/ControlFunction.h"
#include "gui/DataDispatcher.h"
#include "gui/GuiData/GuiDataContextRequest.h"

#include "models/3d/Graph/SimpleMeasureNode.h"
#include "models/3d/Graph/OpenScanToolsGraphManager.hxx"

#include "vulkan/VulkanManager.h"

#include "gui/texts/RayTracingTexts.hpp"

ContextMeshDistance::ContextMeshDistance(const ContextId& id)
    : ARayTracingContext(id)
    , m_meshSelected(xg::Guid())
    , m_meshReady(false)
{
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_WAITING_CLICK_OR_TAG });
    m_repeatInput = true;
}

ContextMeshDistance::~ContextMeshDistance()
{}

ContextState ContextMeshDistance::start(Controller& controller)
{
    return ARayTracingContext::start(controller);
}

ContextState ContextMeshDistance::feedMessage(IMessage* message, Controller& controller)
{
    switch (message->getType())
    {
    case IMessage::MessageType::FULL_CLICK:
    {
        FullClickMessage* clickMsg = static_cast<FullClickMessage*>(message);
        if (clickMsg->m_clickInfo.hover &&
            clickMsg->m_clickInfo.mesh != nullptr)
        {
            m_meshReady = true;
        }
        break;
    }
    default: {}
    }

    ARayTracingContext::feedMessage(message, controller);
    return (m_state);
}

ContextState ContextMeshDistance::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
    if (pointMissing())
        return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

    if (!m_meshReady)
        return (m_state = ContextState::waiting_for_input);


    std::chrono::steady_clock::time_point tp[2];
    tp[0] = std::chrono::steady_clock::now();
    glm::vec3 M = m_clickResults.back().position;
    glm::vec3 meshPoint = ARayTracingContext::minimalDistanceOnMesh(M);

    tp[1] = std::chrono::steady_clock::now();
    float dta = std::chrono::duration<float, std::milli>(tp[1] - tp[0]).count();

    Logger::log(LoggerMode::FunctionLog) << "CPU projection on triangles : " << dta << "[ms]." << Logger::endl;

    // Crée une mesure simple entre le point de départ et le point projeté
    SafePtr<SimpleMeasureNode> measure = controller.getOpenScanToolsGraphManager().createMeasureNode<SimpleMeasureNode>();
    WritePtr<SimpleMeasureNode> wMeasure = measure.get();
    if (!wMeasure)
    {
        assert(false);
        m_clickResults.clear();
        return waitForNextPoint(controller);
    }

    wMeasure->setDefaultData(controller);

    wMeasure->setName(L"D_to_mesh");
    wMeasure->setMeasure({ M, meshPoint });

    controller.getControlListener()->notifyUIControl(new control::function::AddNodes(measure));

    m_clickResults.clear();
    return waitForNextPoint(controller);
}

bool ContextMeshDistance::canAutoRelaunch() const
{
    return false;
}

ContextType ContextMeshDistance::getType() const
{
    return ContextType::meshDistance;
}
