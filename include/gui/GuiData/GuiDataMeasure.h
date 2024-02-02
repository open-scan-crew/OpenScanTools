#ifndef GUI_DATA_MEASURE_H_
#define GUI_DATA_MEASURE_H_

#include "gui/GuiData/IGuiData.h"
#include <glm/glm.hpp>

#include "models/OpenScanToolsModelEssentials.h"

class AGraphNode;

class GuiDataPoint : public IGuiData
{
public:
    GuiDataPoint(const glm::vec3& point);
    ~GuiDataPoint() {};
    virtual guiDType getType() override;

public:
    const glm::vec3 m_point;
};

#endif // !GUI_DATA_MEASURE_H_