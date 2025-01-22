#include "controller/functionSystem/ADuplication.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "models/graph/TransformationModule.h"

#include "glm/gtc/quaternion.hpp"

ADuplication::ADuplication(DuplicationMode mode)
    : m_mode(mode)
{}

ADuplication::~ADuplication()
{}

void ADuplication::setObjectParameters(Controller& controller, TransformationModule& transfo, const glm::dvec3& position, const glm::dvec3& scale)
{
    switch (m_mode)
    {
    case DuplicationMode::Offset:
    {
        const DuplicationSettings& settings = controller.getContext().CgetDuplicationSettings();
        glm::dmat3 rot = settings.isLocal ? glm::mat3_cast(transfo.getOrientation()) : glm::dmat3(1.0);

        transfo.addGlobalTranslation(rot * settings.offset);

    }
    break;
    case DuplicationMode::Click:
    {
        const ClippingBoxSettings& settings = controller.getContext().getClippingSettings();
        switch (settings.offset)
        {
        case ClippingBoxOffset::CenterOnPoint:
            transfo.setPosition(position);
            break;
        case ClippingBoxOffset::Topface:
            transfo.setPosition(position - glm::dvec3(0., 0., scale.z));
            break;
        case ClippingBoxOffset::BottomFace:
            transfo.setPosition(position + glm::dvec3(0., 0., scale.z));
            break;
        }
    }
    break;
    case DuplicationMode::SizeStep:
    {
        const DuplicationSettings& settings = controller.getContext().CgetDuplicationSettings();
        glm::dvec3 tr(scale[0] * settings.step[0],
                      scale[1] * settings.step[1],
                      scale[2] * settings.step[2]);

        glm::dmat3 rot = glm::mat3_cast(transfo.getOrientation());

        transfo.addGlobalTranslation(rot * tr);
    }
    break;
    default:
        break;
    }
}