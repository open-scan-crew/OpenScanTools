#include "controller/functionSystem/ContextMeshObjectDuplication.h"
#include "controller/controls/ControlFunction.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/IControlListener.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/MeshObjectTexts.hpp"
#include "utils/Logger.h"

#include "models/graph/MeshObjectNode.h"
#include "models/graph/GraphManager.h"
#include "vulkan/MeshManager.h"
#include <algorithm>
#include <regex>

namespace
{
std::wstring getMeshCopyName(const std::wstring& sourceName, const GraphManager& graphManager)
{
    std::wstring baseName = sourceName;
    std::wsmatch match;
    static const std::wregex copySuffix(LR"(^(.*)_copy([0-9]+)$)");
    if (std::regex_match(sourceName, match, copySuffix) && match.size() > 1)
        baseName = match[1].str();

    int maxIndex = 0;
    std::unordered_set<SafePtr<AGraphNode>> allMeshes = graphManager.getNodesByTypes({ ElementType::MeshObject }, ObjectStatusFilter::ALL);
    for (const SafePtr<AGraphNode>& meshNode : allMeshes)
    {
        ReadPtr<MeshObjectNode> rMesh = static_pointer_cast<MeshObjectNode>(meshNode).cget();
        if (!rMesh)
            continue;
        const std::wstring& existingName = rMesh->getName();
        if (existingName == baseName)
        {
            maxIndex = std::max(maxIndex, 0);
            continue;
        }
        if (std::regex_match(existingName, match, copySuffix) && match.size() > 2 && match[1].str() == baseName)
        {
            maxIndex = std::max(maxIndex, std::stoi(match[2].str()));
        }
    }
    return baseName + L"_copy" + std::to_wstring(maxIndex + 1);
}
}

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
    
    GraphManager& graphManager = controller.getGraphManager();

    m_state = ContextState::running;
    FUNCLOG << "AContextWavefrontDuplication launch" << LOGENDL;

    std::unordered_set<SafePtr<AGraphNode>> meshes = controller.getGraphManager().getNodesByTypes({ ElementType::MeshObject }, ObjectStatusFilter::SELECTED);
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
        newObj = make_safe<MeshObjectNode>(*&rMesh);
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
    wNewObj->setName(getMeshCopyName(wNewObj->getName(), graphManager));
    setObjectParameters(controller, *&wNewObj, m_clickResults.empty() ? glm::dvec3() : m_clickResults[0].position, scale * glm::dvec3(dim));

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
