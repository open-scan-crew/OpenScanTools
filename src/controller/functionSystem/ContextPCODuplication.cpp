#include "controller/functionSystem/ContextPCODuplication.h"
#include "controller/controls/ControlFunction.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/PointCloudTexts.hpp"
#include "models/3d/Graph/ScanObjectNode.h"
#include "models/3d/Graph/OpenScanToolsGraphManager.hxx"
#include "utils/Logger.h"


ContextPCODuplication::ContextPCODuplication(const ContextId& id)
	: ARayTracingContext(id)
	, ADuplication(DuplicationMode::Click)
{}

ContextPCODuplication::~ContextPCODuplication()
{}

ContextState ContextPCODuplication::start(Controller& controller)
{
    m_mode = controller.getContext().CgetDuplicationSettings().type;
    std::unordered_set<SafePtr<AGraphNode>> pcos = controller.getOpenScanToolsGraphManager().getNodesByTypes({ ElementType::PCO }, ObjectStatusFilter::SELECTED);
    if (pcos.size() == 1)
        m_pco = static_pointer_cast<ScanObjectNode>(SafePtr<AGraphNode>(*pcos.begin()));
    else
        return ARayTracingContext::abort(controller);

    if (m_mode == DuplicationMode::Click)
        m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_POINT_CLOUD_OBJECT_START });
    return ARayTracingContext::start(controller);
}


ContextState ContextPCODuplication::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
    return m_state;
}

ContextState ContextPCODuplication::launch(Controller& controller)
{
    // --- Ray Tracing ---
    if (m_mode == DuplicationMode::Click)
    {
        ARayTracingContext::getNextPosition(controller);
        if (pointMissing())
            return waitForNextPoint(controller);
    }
    // -!- Ray Tracing -!-

    m_state = ContextState::running;
    FUNCLOG << "AContextPCODuplication launch" << LOGENDL;
    OpenScanToolsGraphManager& graphManager = controller.getOpenScanToolsGraphManager();

    SafePtr<ScanObjectNode> newPco;
    glm::dvec3 scale;
    {
        ReadPtr<ScanObjectNode> rPco = m_pco.cget();
        if (!rPco)
        {
            FUNCLOG << "AContextPCODuplication failed do find object " << LOGENDL;
            if (m_mode == DuplicationMode::Click)
                return ARayTracingContext::abort(controller);
            else
                return (m_state = ContextState::abort);
        }
        newPco = graphManager.createCopyNode<ScanObjectNode>(*&rPco);
        scale = rPco->getScale();
    }


    WritePtr<ScanObjectNode> wNewPco = newPco.get();
    if (!wNewPco)
    {
        FUNCLOG << "AContextPCODuplication failed to create copy " << LOGENDL;
        if (m_mode == DuplicationMode::Click)
            return ARayTracingContext::abort(controller);
        else
            return (m_state = ContextState::abort);
    }

    wNewPco->setId(xg::newGuid());
    time_t timeNow;
    wNewPco->setCreationTime(time(&timeNow));
    wNewPco->setModificationTime(time(&timeNow));
    wNewPco->setAuthor(controller.getContext().getActiveAuthor());
    wNewPco->setUserIndex(controller.getNextUserId(wNewPco->getType()));
    setObjectParameters(controller, *&wNewPco, m_clickResults.empty() ? glm::dvec3() : m_clickResults[0].position, scale);

    controller.getControlListener()->notifyUIControl(new control::function::AddNodes(newPco));
    controller.updateInfo(new GuiDataTmpMessage(TEXT_POINT_CLOUD_OBJECT_DUPLICATION_DONE));
    
    if (m_mode == DuplicationMode::Click)
    {
        m_clickResults.clear();
        return waitForNextPoint(controller);
    }
    else
	    return ARayTracingContext::validate(controller);
}

bool ContextPCODuplication::canAutoRelaunch() const
{
    return true;
}

ContextType ContextPCODuplication::getType() const
{
	return ContextType::pointCloudObjectDuplication;
}
