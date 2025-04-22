#include "controller/functionSystem/ContextBoxDuplication.h"
#include "controller/controls/ControlFunction.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/IControlListener.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"
#include "utils/Logger.h"

#include "models/graph/GraphManager.hxx"
#include "models/graph/BoxNode.h"

ContextBoxDuplication::ContextBoxDuplication(const ContextId& id)
    : ARayTracingContext(id)
    , ADuplication(DuplicationMode::Click)
{}

ContextBoxDuplication::~ContextBoxDuplication()
{}

ContextState ContextBoxDuplication::start(Controller& controller)
{
    m_mode = controller.getContext().CgetDuplicationSettings().type;
    if (m_mode == DuplicationMode::Click)
        m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_CLIPPINGBOX_START });
    return ARayTracingContext::start(controller);
}

ContextState ContextBoxDuplication::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
    return m_state;
}

ContextState ContextBoxDuplication::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
    if (pointMissing())
        return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

    FUNCLOG << "ContextBoxDuplication launch" << LOGENDL;
    GraphManager& graphManager = controller.getGraphManager();
    std::unordered_set<SafePtr<AGraphNode>> boxes = graphManager.getNodesByTypes({ ElementType::Box }, ObjectStatusFilter::SELECTED);

    if (boxes.empty() || boxes.size() > 1)
        return ARayTracingContext::abort(controller);
    SafePtr<BoxNode> newBox;
    {
        ReadPtr<BoxNode> boxSelected = static_pointer_cast<BoxNode>(*boxes.begin()).cget();
        if (!boxSelected)
        {
            FUNCLOG << "ContextBoxDuplication failed do find box " << LOGENDL;
            return ARayTracingContext::abort(controller);
        }
        newBox = graphManager.createCopyNode(*&boxSelected);
    }
   
    WritePtr<BoxNode> wBox = newBox.get();
    if(!wBox)
    {
        FUNCLOG << "ContextBoxDuplication failed to create box " << LOGENDL;
        return ARayTracingContext::abort(controller);
    }
    wBox->setId(xg::newGuid());
    time_t timeNow;
    wBox->setCreationTime(time(&timeNow));
    wBox->setModificationTime(time(&timeNow));
    wBox->setAuthor(controller.getContext().getActiveAuthor());
    wBox->setUserIndex(controller.getNextUserId(wBox->getType()));
    setObjectParameters(controller, *&wBox, m_clickResults.empty() ? glm::dvec3() : m_clickResults[0].position, wBox->getScale());

    controller.getControlListener()->notifyUIControl(new control::function::AddNodes(newBox));

    controller.updateInfo(new GuiDataTmpMessage(TEXT_CLIPPINGBOX_DUPLICATION_DONE));

    if (m_mode == DuplicationMode::Click)
    {
        m_clickResults.clear();
        return waitForNextPoint(controller);
    }
    else
        return ARayTracingContext::validate(controller);
}

bool ContextBoxDuplication::canAutoRelaunch() const
{
    return true;
}

ContextType ContextBoxDuplication::getType() const
{
    return ContextType::boxDuplication;
}
