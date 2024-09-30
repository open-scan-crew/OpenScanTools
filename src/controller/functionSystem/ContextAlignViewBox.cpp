#include "controller/functionSystem/ContextAlignViewBox.h"
#include "controller/messages/CameraMessage.h"
#include "gui/GuiData/GuiDataContextRequest.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataMeasure.h"
#include "gui/GuiData/GuiDataRendering.h"

#include "models/3d/Graph/CameraNode.h"
#include "models/3d/Graph/GraphManager.hxx"

#include "utils/math/trigo.h"
#include "utils/Logger.h"
#include "magic_enum/magic_enum.hpp"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/vector_angle.hpp>

ContextAlignViewBox::ContextAlignViewBox(const ContextId& id)
	: AContext(id)
{}

ContextAlignViewBox::~ContextAlignViewBox()
{}

ContextState ContextAlignViewBox::start(Controller& controller)
{
    controller.updateInfo(new GuiDataContextRequestActiveCamera(m_id));
	return (m_state = ContextState::waiting_for_input);
}

ContextState ContextAlignViewBox::feedMessage(IMessage* message, Controller& controller)
{
	switch (message->getType())
	{
		case  IMessage::MessageType::CAMERA:
		{
			auto out = static_cast<CameraMessage*>(message);
            m_cameraNode = out->m_cameraNode;
            ReadPtr<CameraNode> rCam = m_cameraNode.cget();
            if (!rCam)
            {
                assert(false);
                return (m_state = ContextState::abort);
            }

			if (std::isnan(rCam->getCenter().x) == true)
			{
				FUNCLOG << "picking nan detected" << LOGENDL;
				return (m_state);
			}
			FUNCLOG << "ContextAlignViewBox feedClick " << rCam->getCenter() << LOGENDL;
			return (m_state = ContextState::ready_for_using);
		}
		default:
		FUNCLOG << "wrong message type (" << magic_enum::enum_name<IMessage::MessageType>(message->getType())<< ")" << LOGENDL;
		break;
	}
	return (m_state = ContextState::waiting_for_input);
}

ContextState ContextAlignViewBox::launch(Controller& controller)
{
	FUNCLOG << "ContextAlignViewBox launch" << LOGENDL;
    GraphManager& graphManager = controller.getGraphManager();
	std::unordered_set<SafePtr<AGraphNode>> selected = graphManager.getSelectedNodes();
	if (selected.empty()|| selected.size() > 1)
		return (m_state = ContextState::abort);
	ReadPtr<AGraphNode> rObject = (SafePtr<AGraphNode>(*selected.begin())).cget();
	if (!rObject && (rObject->getType() != ElementType::Box || rObject->getType() != ElementType::Grid))
	{
		FUNCLOG << "ContextAlignViewBox failed do find object " << LOGENDL;
		return (m_state = ContextState::abort);
	}

	alignView(controller, *&rObject);
	return (m_state = ContextState::done);
}

void ContextAlignViewBox::alignView(Controller& controller, const TransformationModule& module)
{
    // Results
    glm::dvec3 newPos;
    double theta = 0.0;
    double phi = 0.0;

    std::vector<glm::dvec4> normals = {
        {1.0, 0.0, 0.0, 0.0},
        {-1.0, 0.0, 0.0, 0.0},
        {0.0, 1.0, 0.0, 0.0},
        {0.0, -1.0, 0.0, 0.0},
        {0.0, 0.0, 1.0, 0.0},
        {0.0, 0.0, -1.0, 0.0}
    };

    ReadPtr<CameraNode> rCam = m_cameraNode.cget();
    if (!rCam)
    {
        assert(false);
        return;
    }

    const glm::dvec4 center(module.getCenter(), 1.0);
    const glm::dvec4 camera(rCam->getCenter(), 1.0);
    glm::dvec4 vecCamera;
    double minAngle(std::numeric_limits<double>::max());
    int id(0);

    for (int i = 0; i < normals.size(); ++i)
    {
        if (rCam->getProjectionMode() == ProjectionMode::Perspective)
        {
            glm::dvec4 scale(module.getScale(), 1.0);
            glm::dvec4 scaleN = normals[i] * scale;
            glm::dvec4 offset(glm::rotate(module.getOrientation(), scaleN));
            glm::dvec4 faceCenter(center + offset);
            vecCamera = glm::dvec4(glm::normalize(camera - faceCenter)); // w = 0
        }
        else
        {
            vecCamera = glm::dvec4(0.0, 0.0, -1.0, 0.0);
            vecCamera = glm::rotate(rCam->getOrientation(), vecCamera);
        }

        normals[i] = glm::rotate(module.getOrientation(), normals[i]);
        double angle(glm::angle(normals[i], vecCamera));
        if (minAngle > angle)
        {
            id = i;
            minAngle = angle;
        }
    }

    double dist = glm::length(camera - center);
    newPos = rCam->getProjectionMode() == ProjectionMode::Perspective ? center + normals[id] * dist : center;

    // Notes on theta computation :
    //  * the camera convention is theta = 0 along Y axis.
    theta = atan2(normals[id].x, -normals[id].y);
    // Notes on phi computation :
    //  * we take -sqrt because we want phi between [0, -pi/2]
    //  * we take -nz because we want to look in the opposite way of the normal
    phi = atan2(-std::sqrt(std::pow(normals[id].x, 2) + std::pow(normals[id].y, 2)), -normals[id].z);

    controller.updateInfo(new GuiDataRenderCameraMoveTo(glm::dvec4(newPos, 1.0), m_cameraNode));
	controller.updateInfo(new GuiDataRenderRotateCamera(theta, phi, false, m_cameraNode));
}

bool ContextAlignViewBox::canAutoRelaunch() const
{
	return (false);
}

ContextType ContextAlignViewBox::getType() const
{
	return ContextType::alignView2P;
}
