#include "controller/functionSystem/ContextMeshObjectDuplication.h"
#include "controller/controls/ControlFunction.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/MeshObjectTexts.hpp"
#include "utils/Logger.h"

#include "models/3d/Graph/MeshObjectNode.h"
#include "models/3d/Graph/OpenScanToolsGraphManager.hxx"

ContextMeshObjectDuplication::ContextMeshObjectDuplication(const ContextId& id)
	: ARayTracingContext(id)
	, ADuplication(DuplicationMode::Click)
{}

ContextMeshObjectDuplication::~ContextMeshObjectDuplication()
{}

ContextState ContextMeshObjectDuplication::start(Controller& controller)
{
    m_mode = controller.getContext().CgetDuplicationSettings().type;
    if(m_mode == DuplicationMode::Click)
        m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_MESHOBJECT_START });
	return ARayTracingContext::start(controller);
}

ContextState ContextMeshObjectDuplication::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
    return m_state;
}

ContextState ContextMeshObjectDuplication::launch(Controller& controller)
{
    // --- Ray Tracing ---
    if (m_mode == DuplicationMode::Click)
    {
        ARayTracingContext::getNextPosition(controller);
        if (pointMissing())
            return waitForNextPoint(controller);
    }
    // -!- Ray Tracing -!-
    
    OpenScanToolsGraphManager& graphManager = controller.getOpenScanToolsGraphManager();

    m_state = ContextState::running;
    FUNCLOG << "AContextWavefrontDuplication launch" << LOGENDL;

    std::unordered_set<SafePtr<AGraphNode>> meshes = controller.getOpenScanToolsGraphManager().getNodesByTypes({ ElementType::MeshObject }, ObjectStatusFilter::SELECTED);
    if (meshes.size() != 1)
        return ARayTracingContext::abort(controller);

    SafePtr<MeshObjectNode> newObj;
    glm::dvec4 dim;
    glm::dvec3 scale;
    {
        ReadPtr<MeshObjectNode> rMesh = static_pointer_cast<MeshObjectNode>(*(meshes.begin())).cget();
        if (!rMesh)
        {
            FUNCLOG << "AContextWavefrontDuplication failed do find object " << LOGENDL;
            if (m_mode == DuplicationMode::Click)
                return ARayTracingContext::abort(controller);
            else
                return (m_state = ContextState::abort);
        }
        newObj = graphManager.createCopyNode(*&rMesh);
        dim = glm::dvec4(rMesh->getDimension(), 1.0);
        scale = rMesh->getScale();
    }
   
    WritePtr<MeshObjectNode> wNewObj = newObj.get();
    if (!wNewObj)
    {
        FUNCLOG << "AContextWavefrontDuplication failed to create copy" << LOGENDL;
        if (m_mode == DuplicationMode::Click)
            return ARayTracingContext::abort(controller);
        else
            return (m_state = ContextState::abort);
    }

    wNewObj->setId(xg::newGuid());
    time_t timeNow;
    wNewObj->setCreationTime(time(&timeNow));
    wNewObj->setModificationTime(time(&timeNow));
    wNewObj->setAuthor(controller.getContext().getActiveAuthor());
    wNewObj->setUserIndex(controller.getNextUserId(wNewObj->getType()));
    setObjectParameters(controller, *&wNewObj, m_clickResults.empty() ? glm::dvec3() : m_clickResults[0].position, scale * glm::dvec3(dim));

    MeshManager::getInstance().addMeshInstance(wNewObj->getMeshId());

    controller.getControlListener()->notifyUIControl(new control::function::AddNodes(newObj));

    controller.updateInfo(new GuiDataTmpMessage(TEXT_MESHOBJECT_DUPLICATION_DONE));
    
    if (m_mode == DuplicationMode::Click)
    {
        m_clickResults.clear();
        return waitForNextPoint(controller);
    }
    else
        return ARayTracingContext::validate(controller);
}

bool ContextMeshObjectDuplication::canAutoRelaunch() const
{
    return true;
}

ContextType ContextMeshObjectDuplication::getType() const
{
	return ContextType::meshObjectDuplication;
}
