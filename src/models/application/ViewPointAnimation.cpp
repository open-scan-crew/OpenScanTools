#include "models/application/ViewPointAnimation.h"

ViewPointAnimationConfig::ViewPointAnimationConfig()
    : m_id(xg::newGuid())
    , m_order(0)
    , m_mode(ViewPointAnimationMode::PositionAsTime)
{}

ViewPointAnimationConfig::ViewPointAnimationConfig(viewPointAnimationId id)
    : m_id(id)
    , m_order(0)
    , m_mode(ViewPointAnimationMode::PositionAsTime)
{}

viewPointAnimationId ViewPointAnimationConfig::getId() const
{
    return m_id;
}

const QString& ViewPointAnimationConfig::getName() const
{
    return m_name;
}

uint32_t ViewPointAnimationConfig::getOrder() const
{
    return m_order;
}

ViewPointAnimationMode ViewPointAnimationConfig::getMode() const
{
    return m_mode;
}

const std::vector<ViewPointAnimationLine>& ViewPointAnimationConfig::getLines() const
{
    return m_lines;
}

void ViewPointAnimationConfig::setId(const viewPointAnimationId& id)
{
    m_id = id;
}

void ViewPointAnimationConfig::setName(const QString& name)
{
    m_name = name;
}

void ViewPointAnimationConfig::setOrder(uint32_t order)
{
    m_order = order;
}

void ViewPointAnimationConfig::setMode(ViewPointAnimationMode mode)
{
    m_mode = mode;
}

void ViewPointAnimationConfig::setLines(const std::vector<ViewPointAnimationLine>& lines)
{
    m_lines = lines;
}
