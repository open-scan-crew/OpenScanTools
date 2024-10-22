#include "controller/functionSystem/ContextViewPoint.h"
#include "controller/Controller.h"
#include "controller/ControlListener.h" // forward declaration
#include "controller/controls/ControlFunction.h"
#include "controller/controls/ControlViewpoint.h"
#include "controller/messages/CameraMessage.h"
#include "controller/messages/DataIdListMessage.h"

#include "models/graph/CameraNode.h"
#include "models/graph/ViewPointNode.h"

#include "gui/GuiData/GuiDataContextRequest.h"
#include "gui/texts/DefaultNameTexts.hpp"


/*
* ContextViewPointCreation
*/

ContextViewPointCreation::ContextViewPointCreation(const ContextId& id)
    : AContext(id)
{}

ContextViewPointCreation::~ContextViewPointCreation()
{}

ContextState ContextViewPointCreation::start(Controller& controller)
{
    controller.updateInfo(new GuiDataContextRequestActiveCamera(m_id));
    return (m_state = ContextState::waiting_for_input);
}

ContextState ContextViewPointCreation::feedMessage(IMessage* message, Controller& controller)
{
    switch(message->getType())
    {
        case IMessage::MessageType::CAMERA:
        {
            CameraMessage* modal = static_cast<CameraMessage*>(message);
            m_cameraNode = modal->m_cameraNode;
            return (m_state = ContextState::ready_for_using);
        }
    }
    return (m_state = ContextState::waiting_for_input);
}

ContextState ContextViewPointCreation::launch(Controller& controller)
{
    ReadPtr<CameraNode> rCameraInfos = m_cameraNode.cget();

    SafePtr<ViewPointNode> viewpoint = make_safe<ViewPointNode>();
    {
        WritePtr<ViewPointNode> wViewPoint = viewpoint.get();
        wViewPoint->setName(TEXT_DEFAULT_NAME_VIEWPOINT.toStdWString());
        wViewPoint->setVisible(true);
        wViewPoint->setDefaultData(controller);
    }

    controller.getControlListener()->notifyUIControl(new control::viewpoint::UpdateViewPoint(viewpoint, m_cameraNode, false));

    controller.getControlListener()->notifyUIControl(new control::function::AddNodes(viewpoint));
    return (m_state = ContextState::done);
}

bool ContextViewPointCreation::canAutoRelaunch() const
{
    return (false);
}

ContextType ContextViewPointCreation::getType() const
{
    return (ContextType::viewpointCreation);
}

/*
* ContextViewPointUpdate 
*/

ContextViewPointUpdate::ContextViewPointUpdate(const ContextId& id)
    : AContext(id)
    , m_messageCount(0)
{}

ContextViewPointUpdate::~ContextViewPointUpdate()
{}

ContextState ContextViewPointUpdate::start(Controller& controller)
{
    controller.updateInfo(new GuiDataContextRequestActiveCamera(m_id));
    return (m_state = ContextState::waiting_for_input);
}

ContextState ContextViewPointUpdate::feedMessage(IMessage* message, Controller& controller)
{
    switch (message->getType())
    {
    case IMessage::MessageType::DATAID_LIST:
    {
        auto msgCast = static_cast<DataListMessage*>(message);
        if (msgCast->m_type == ElementType::ViewPoint && msgCast->m_dataPtrs.size() == 1)
        {
            m_viewpoint = static_pointer_cast<ViewPointNode>(*(msgCast->m_dataPtrs.begin()));
            m_messageCount++;
        }
        break;
    }
    case IMessage::MessageType::CAMERA:
    {
        CameraMessage* modal = static_cast<CameraMessage*>(message);
        m_cameraNode = modal->m_cameraNode;
        m_messageCount++;
        break;
    }
    }
    return (m_state = m_messageCount == 2 ? ContextState::ready_for_using : ContextState::waiting_for_input);
}

ContextState ContextViewPointUpdate::launch(Controller& controller)
{
    controller.getControlListener()->notifyUIControl(new control::viewpoint::UpdateViewPoint(m_viewpoint, m_cameraNode, true));
    return (m_state = ContextState::done);
}

bool ContextViewPointUpdate::canAutoRelaunch() const
{
    return (false);
}

ContextType ContextViewPointUpdate::getType() const
{
    return (ContextType::viewpointUpdate);
}
