#include "controller/functionSystem/ADuplication.h"
//#include "controller/controls/ControlFunctionClipping.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "models/graph/TransformationModule.h"
#include "utils/math/trigo.h"

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
        if (settings.isLocal)
        {
            double pos[3] = { 0.0, 0.0, 0.0 };
            glm::mat4 mat(tls::math::getTransformMatrix(pos, &transfo.getOrientation()[0]));
            transfo.addGlobalTranslation(mat*glm::vec4(settings.offset, 1.0f));
        }
        else
            transfo.addGlobalTranslation(settings.offset);
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
        glm::dvec4 tr(0.0);
        tr.w = 1.0;

        for (uint32_t iterator(0); iterator < 3; iterator++)
            tr[iterator] = scale[iterator] * settings.step[iterator];
        
        double pos[3] = { 0.0, 0.0, 0.0 };
        glm::mat4 mat(tls::math::getTransformMatrix(pos, &transfo.getOrientation()[0]));
        transfo.addGlobalTranslation(mat* tr);
    }
    break;
    default:
        break;
    }
}