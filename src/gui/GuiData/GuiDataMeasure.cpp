#include "gui/GuiData/GuiDataMeasure.h"

// *** Point *** //


GuiDataPoint::GuiDataPoint(const glm::vec3& point)
    : m_point(point)
{}

guiDType GuiDataPoint::getType()
{
    return (guiDType::point);
}

// *** Polyline Measure Updated *** //

GuiDataPolylineMeasureUpdated::GuiDataPolylineMeasureUpdated(const SafePtr<PolylineMeasureNode>& polyline)
    : m_polyline(polyline)
{}

guiDType GuiDataPolylineMeasureUpdated::getType()
{
    return (guiDType::polylineMeasureUpdated);
}
