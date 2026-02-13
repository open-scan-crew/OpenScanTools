#include "controller/functionSystem/ContextPolygonalSelector.h"

#include "controller/Controller.h"
#include "controller/controls/ControlFunction.h"
#include "controller/messages/FullClickMessage.h"
#include "controller/messages/IMessage.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/texts/ContextTexts.hpp"

namespace
{
    PolygonalSelectorSettings s_polygonalSelectorRuntimeSettings;
}

ContextPolygonalSelector::ContextPolygonalSelector(const ContextId& id)
    : AContext(id)
{}

ContextPolygonalSelector::~ContextPolygonalSelector()
{}

ContextState ContextPolygonalSelector::start(Controller& controller)
{
    m_settings = s_polygonalSelectorRuntimeSettings;
    controller.updateInfo(new GuiDataTmpMessage(TEXT_POINT_MEASURE_START));
    return (m_state = ContextState::waiting_for_input);
}

ContextState ContextPolygonalSelector::feedMessage(IMessage* message, Controller& controller)
{
    if (message->getType() == IMessage::MessageType::FULL_CLICK)
    {
        auto* clickMsg = static_cast<FullClickMessage*>(message);
        const ClickInfo& click = clickMsg->m_clickInfo;
        float nx = click.width > 0 ? static_cast<float>(click.picking.x / static_cast<double>(click.width)) : 0.0f;
        float ny = click.height > 0 ? static_cast<float>(click.picking.y / static_cast<double>(click.height)) : 0.0f;
        m_currentVertices.emplace_back(nx, ny);

        controller.updateInfo(new GuiDataTmpMessage(QString("Polygonal selector: %1 vertex/vertices").arg(m_currentVertices.size())));
    }

    return (m_state = ContextState::waiting_for_input);
}

ContextState ContextPolygonalSelector::launch(Controller& controller)
{
    (void)controller;
    return (m_state = ContextState::waiting_for_input);
}

ContextState ContextPolygonalSelector::abort(Controller& controller)
{
    m_currentVertices.clear();
    return AContext::abort(controller);
}

ContextState ContextPolygonalSelector::validate(Controller& controller)
{
    if (m_currentVertices.size() >= 3)
    {
        PolygonalSelectorPolygon polygon;
        polygon.normalizedVertices = m_currentVertices;
        m_settings.polygons.push_back(polygon);
        m_settings.active = true;
        s_polygonalSelectorRuntimeSettings = m_settings;

        controller.updateInfo(new GuiDataRenderPolygonalSelector(m_settings, SafePtr<CameraNode>()));
    }

    m_currentVertices.clear();
    return AContext::validate(controller);
}

bool ContextPolygonalSelector::canAutoRelaunch() const
{
    return true;
}

ContextType ContextPolygonalSelector::getType() const
{
    return ContextType::polygonalSelector;
}
