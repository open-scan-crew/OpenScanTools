#ifndef GUI_DATA_MEASURE_H_
#define GUI_DATA_MEASURE_H_

#include "gui/GuiData/IGuiData.h"
#include "models/OpenScanToolsModelEssentials.h"
#include <glm/glm.hpp>

class GuiDataPoint : public IGuiData
{
public:
    GuiDataPoint(const glm::vec3& point);
    ~GuiDataPoint() {};
    virtual guiDType getType() override;

public:
    const glm::vec3 m_point;
};

class PolylineMeasureNode;

class GuiDataPolylineMeasureUpdated : public IGuiData
{
public:
    GuiDataPolylineMeasureUpdated(const SafePtr<PolylineMeasureNode>& polyline);
    ~GuiDataPolylineMeasureUpdated() {};
    virtual guiDType getType() override;

public:
    const SafePtr<PolylineMeasureNode> m_polyline;
};

#endif // !GUI_DATA_MEASURE_H_
