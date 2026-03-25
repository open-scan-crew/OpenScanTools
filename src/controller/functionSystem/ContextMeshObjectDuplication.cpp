#include "controller/functionSystem/ContextMeshObjectDuplication.h"
#include "controller/controls/ControlFunction.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/IControlListener.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/MeshObjectTexts.hpp"
#include "utils/Logger.h"
#include "vulkan/MeshManager.h"

#include "models/graph/MeshObjectNode.h"
#include "models/graph/GraphManager.h"

#include <cwctype>

namespace
{
std::wstring getNextCopyName(const std::wstring& sourceName, const GraphManager& graphManager)
{
    const std::wstring copyToken = L"_copy";
    std::wstring baseName = sourceName;
    const size_t suffixPos = sourceName.rfind(copyToken);
    if (suffixPos != std::wstring::npos)
    {
        bool isNumericSuffix = (suffixPos + copyToken.size() < sourceName.size());
        for (size_t i = suffixPos + copyToken.size(); i < sourceName.size(); ++i)
        {
            if (!iswdigit(sourceName[i]))
            {
                isNumericSuffix = false;
                break;
            }
        }
        if (isNumericSuffix)
            baseName = sourceName.substr(0, suffixPos);
    }

    std::unordered_set<std::wstring> usedNames;
    for (const SafePtr<AGraphNode>& node : graphManager.getNodesByTypes({ ElementType::MeshObject }, ObjectStatusFilter::ALL))
    {
        ReadPtr<AGraphNode> rNode = node.cget();
        if (!rNode)
            continue;
        usedNames.insert(rNode->getName());
    }

    uint32_t index = 1;
    std::wstring candidate;
    do
    {
        candidate = baseName + copyToken + std::to_wstring(index++);
    } while (usedNames.find(candidate) != usedNames.end());

    return candidate;
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
    wNewObj->setName(getNextCopyName(wNewObj->getName(), graphManager));
    wNewObj->setObjectName(wNewObj->getName());
    wNewObj->setImportedOriginal(false);
    setObjectParameters(controller, *&wNewObj, m_clickResults.empty() ? glm::dvec3() : m_clickResults[0].position, scale * glm::dvec3(dim));

    MeshManager& meshManager = MeshManager::getInstance();
    if (!meshManager.addMeshInstance(wNewObj->getMeshId()))
    {
        const std::filesystem::path meshFolder = controller.getContext().cgetProjectInternalInfo().getObjectsFilesFolderPath();
        if (meshManager.reloadMeshFile(*&wNewObj, meshFolder, &controller) != ObjectAllocation::ReturnCode::Success)
        {
            FUNCLOG << "AContextWavefrontDuplication failed to add mesh instance for copy" << LOGENDL;
            if (m_mode == DuplicationMode::Click)
                return ARayTracingContext::abort(controller);
            else
                return (m_state = ContextState::abort);
        }
    }

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
