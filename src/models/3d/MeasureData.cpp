#include "models/3d/MeasureData.h"
#include "vulkan/VulkanManager.h"
#include "utils/Logger.h"
#include "utils/math/basic_functions.h"
#include "models/data/BeamBendingMeasure.h"
#include "models/data/ColumnTiltMeasure.h"
#include "models/data/SimpleMeasure.h"
#include "models/data/PolylineMeasure.h"
#include "models/data/PipeToPipeMeasure.h"
#include "models/data/PipeToPlaneMeasure.h"
#include "models/data/PointToPlaneMeasure.h"
#include "models/data/PointToPipeMeasure.h"

#include "models/3d/Graph/AMeasureNode.h"

PointBufferData::PointBufferData(const glm::dvec3& _point)
    : position{ (float)_point.x, (float)_point.y, (float)_point.z }
{ }

PointBufferData::PointBufferData(const glm::dvec3& _first, const glm::dvec3& _second)
{
    if (_first.z < _second.z)
    {
        position[0] = (float)_second.x;
        position[1] = (float)_second.y;
        position[2] = (float)_first.z;
    }
    else
    {
        position[0] = (float)_first.x;
        position[1] = (float)_first.y;
        position[2] = (float)_second.z;
    }
}

MeasureData::MeasureData()
    : m_measureCount(0)
{ }

MeasureData::~MeasureData()
{
    VulkanManager::getInstance().freeAllocation(m_sbuf);
}

bool MeasureData::storeNode(const std::unordered_map<dataId, AMeasureNode*>& nodes)
{
    m_measureCount = 0;

    std::vector<PointBufferData> measurePoints;
    std::vector<uint32_t> measureColors;
    std::vector<uint32_t> measureIds;
    measurePoints.reserve(nodes.size() * 2);
    measureColors.reserve(nodes.size());
    measureIds.reserve(nodes.size());

    for (const auto& autoMeasure : nodes)
    {
        if (!autoMeasure.second->isVisible())
            continue;
       /* std::vector<PointBufferData>& newPoints; = autoMeasure.second->getPoints();
        measurePoints.insert(measurePoints.begin(), newPoints.begin(), newPoints.end());
        for (uint32_t iterator(0); iterator < ((newPoints.size() / 2) + 1); iterator++)
        {
            measureColors.push_back(0xFFFFFFFF); // white
            measureIds.push_back(autoMeasure.first);
        }*/
    }

    if (measurePoints.empty())
        return true;

    // Reallocate the memory only if needed
    MeasureData::checkBufferSize(measurePoints, measureColors, measureIds);
    m_measureCount = (uint32_t)measurePoints.size() / 2;
    storeInBuffer(measurePoints, measureColors, measureIds);

    return true;
}

void MeasureData::reset(VkDeviceSize _size)
{
    VulkanManager::getInstance().freeAllocation(m_sbuf);
    VulkanManager::getInstance().allocSimpleBuffer(_size, m_sbuf, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    m_measureCount = 0;
}

void MeasureData::checkBufferSize(std::vector<PointBufferData> const& points, std::vector<uint32_t> const& colors, std::vector<uint32_t> const& indices)
{
    size_t spaceNeeded = points.size() * sizeof(PointBufferData) + colors.size() * sizeof(uint32_t) + indices.size() * sizeof(uint32_t);

    if (spaceNeeded > m_sbuf.size) {
        // Reset the existing buffer
        VkDeviceSize newSize = tls::math::getCeilPowTwo(spaceNeeded);
        // and allocate a new buffer with sufficient space
        reset(newSize);

        Logger::log(LoggerMode::VKLog) << "Measures: change buffer size: " << newSize << "\n" << Logger::endl;
    }
}

void MeasureData::storeInBuffer(std::vector<PointBufferData> const& measurePoints, std::vector<uint32_t> const& measureColors, std::vector<uint32_t> const& measureIds)
{
    m_positionsOffset = 0;
    VulkanManager::getInstance().loadInSimpleBuffer(m_sbuf, measurePoints.size() * sizeof(PointBufferData), measurePoints.data(), m_positionsOffset, 4);

    m_colorsOffset = m_positionsOffset + measurePoints.size() * sizeof(PointBufferData);
    VulkanManager::getInstance().loadInSimpleBuffer(m_sbuf, measureColors.size() * sizeof(uint32_t), measureColors.data(), m_colorsOffset, 4);

    m_idsOffset = m_colorsOffset + measureColors.size() * sizeof(uint32_t);
    VulkanManager::getInstance().loadInSimpleBuffer(m_sbuf, measureIds.size() * sizeof(uint32_t), measureIds.data(), m_idsOffset, 4);
}

SimpleBuffer const& MeasureData::getSBuffer() const
{
    return (m_sbuf);
}

VkBuffer const& MeasureData::getBuffer() const
{
    return (m_sbuf.buffer);
}

uint32_t MeasureData::getMeasureCount() const
{
    return (m_measureCount);
}

VkDeviceSize const& MeasureData::getPositionsOffset() const
{
    return (m_positionsOffset);
}

VkDeviceSize const& MeasureData::getColorsOffset() const
{
    return (m_colorsOffset);
}

VkDeviceSize const& MeasureData::getIdsOffset() const
{
    return (m_idsOffset);
}

SimpleMeasureData::SimpleMeasureData()
    : MeasureData()
{}

SimpleMeasureData::~SimpleMeasureData() {}

bool SimpleMeasureData::store(std::vector<AUIData*>& _measures, std::unordered_map<dataId, std::list<Measure>>& storedMeasure)
{
    m_measureCount = 0;

    std::vector<PointBufferData> measurePoints;
    std::vector<uint32_t> measureColors;
    std::vector<uint32_t> measureIds;
    measurePoints.reserve(_measures.size() * 2);
    measureColors.reserve(_measures.size());
    measureIds.reserve(_measures.size());

    for (auto autoMeasure : _measures)
    {
        if (!autoMeasure->isDisplayed())
            continue;
        switch (autoMeasure->getType())
        {
            case TreeElementType::SimpleMeasure:
            {
                auto* sm = static_cast<UISimpleMeasure*>(autoMeasure);
                measurePoints.push_back(PointBufferData(sm->getOriginPos()));
                measurePoints.push_back(PointBufferData(sm->getDestinationPos()));
                measureColors.push_back(0xFFFFFFFF); // white
                measureIds.push_back(sm->getId());
                storedMeasure.insert({ sm->getId(), sm->getMeasures() });
            }
        }
    }

    if (measurePoints.empty())
        return true;

    // Reallocate the memory only if needed
    MeasureData::checkBufferSize(measurePoints, measureColors, measureIds);
    m_measureCount = (uint32_t)measurePoints.size() / 2;
    MeasureData::storeInBuffer(measurePoints, measureColors, measureIds);

    return true;
}

ComplexeMeasureData::ComplexeMeasureData() 
    : MeasureData()
{}

ComplexeMeasureData::~ComplexeMeasureData()
{}

bool ComplexeMeasureData::store(std::vector<AUIData*>& _measures, std::unordered_map<dataId, std::list<Measure>>& storedMeasure)
{
    m_measureCount = 0;

    std::vector<PointBufferData> measurePoints;
    std::vector<uint32_t> measureColors;
    std::vector<uint32_t> measureIds;
    measurePoints.reserve(_measures.size() * 4);
    measureColors.reserve(_measures.size());
    measureIds.reserve(_measures.size());

    for (auto autoMeasure : _measures)
    {
        if (!autoMeasure->isDisplayed())
            continue;
        switch (autoMeasure->getType())
        {
        case TreeElementType::PolylineMeasure:
        {
            auto* plm = static_cast<UIPolylineMeasure*>(autoMeasure);
            for (const Measure& measure : plm->getMeasures())
            {
                measurePoints.push_back(PointBufferData(measure.origin));
                measurePoints.push_back(PointBufferData(measure.final));
                measureColors.push_back(0xFFFFFFFF); // white
                measureIds.push_back(plm->getId());
            }
            storedMeasure.insert({ plm->getId(), plm->getMeasures()});
        }
        break;

        case TreeElementType::PipeToPipeMeasure:
        {
            auto* pipim = static_cast<UIPipeToPipeMeasure*>(autoMeasure);
            measurePoints.push_back(PointBufferData(pipim->getPipe1Center()));
            measurePoints.push_back(PointBufferData(pipim->getProjPoint()));
            measureColors.push_back(0xFFFFFFFF); // white
            measureIds.push_back(pipim->getId());
            measurePoints.push_back(PointBufferData(pipim->getProjPoint()));
            measurePoints.push_back(PointBufferData(pipim->getPipe2Center()));
            measureColors.push_back(0xFFFFFFFF); // white
            measureIds.push_back(pipim->getId());
            storedMeasure.insert({ pipim->getId(), pipim->getMeasures() });
        }
        break;
        case TreeElementType::PipeToPlaneMeasure:
        {
            auto* piplm = static_cast<UIPipeToPlaneMeasure*>(autoMeasure);
            measurePoints.push_back(PointBufferData(piplm->getPipeCenter()));
            measurePoints.push_back(PointBufferData(piplm->getProjPoint()));
            measureColors.push_back(0xFFFFFFFF); // white
            measureIds.push_back(piplm->getId());
            measurePoints.push_back(PointBufferData(piplm->getProjPoint()));
            measurePoints.push_back(PointBufferData(piplm->getPointOnPlane()));
            measureColors.push_back(0xFFFFFFFF); // white
            measureIds.push_back(piplm->getId());
            storedMeasure.insert({ piplm->getId(), piplm->getMeasures() });
        }
        break;
        case TreeElementType::PointToPlaneMeasure:
        {
            auto* poplm = static_cast<UIPointToPlaneMeasure*>(autoMeasure);
            measurePoints.push_back(PointBufferData(poplm->getPointOnPlane()));
            measurePoints.push_back(PointBufferData(poplm->getProjPoint()));
            measureColors.push_back(0xFFFFFFFF); // white
            measureIds.push_back(poplm->getId()); 
            measurePoints.push_back(PointBufferData(poplm->getPointCoord()));
            measurePoints.push_back(PointBufferData(poplm->getProjPoint()));
            measureColors.push_back(0xFFFFFFFF); // white
            measureIds.push_back(poplm->getId());
            storedMeasure.insert({ poplm->getId(), poplm->getMeasures() });
        }
        break;
        case TreeElementType::PointToPipeMeasure:
        {
            auto* poplm = static_cast<UIPointToPipeMeasure*>(autoMeasure);
            measurePoints.push_back(PointBufferData(poplm->getPipeCenter()));
            measurePoints.push_back(PointBufferData(poplm->getProjPoint()));
            measureColors.push_back(0xFFFFFFFF); // white
            measureIds.push_back(poplm->getId());
            measurePoints.push_back(PointBufferData(poplm->getProjPoint()));
            measurePoints.push_back(PointBufferData(poplm->getPointCoord()));
            measureColors.push_back(0xFFFFFFFF); // white
            measureIds.push_back(poplm->getId());
            storedMeasure.insert({ poplm->getId(), poplm->getMeasures() });
        }
        break;
        }
    }

    if (measurePoints.empty())
        return true;

    // Reallocate the memory only if needed
    MeasureData::checkBufferSize(measurePoints, measureColors, measureIds);
    m_measureCount = (uint32_t)measurePoints.size() / 2;
    storeInBuffer(measurePoints, measureColors, measureIds);

    return true;
}


PolylineMeasureData::PolylineMeasureData()
    : MeasureData()
{}

PolylineMeasureData::~PolylineMeasureData()
{}

bool PolylineMeasureData::store(std::vector<AUIData*>& _measures, std::unordered_map<dataId, std::list<Measure>>& storedMeasure)
{
    m_measureCount = 0;

    std::vector<PointBufferData> measurePoints;
    std::vector<uint32_t> measureColors;
    std::vector<uint32_t> measureIds;
    measurePoints.reserve(_measures.size() * 4);
    measureColors.reserve(_measures.size());
    measureIds.reserve(_measures.size());

    for (auto autoMeasure : _measures)
    {
        if (!autoMeasure->isDisplayed())
            continue;
        switch (autoMeasure->getType())
        {
            case TreeElementType::PolylineMeasure:
            {
                auto* plm = static_cast<UIPolylineMeasure*>(autoMeasure);
                measurePoints.push_back(PointBufferData(plm->getFirstPos()));
                measurePoints.push_back(PointBufferData(plm->getLastPos()));
                measureColors.push_back(0xFFFFFFFF); // white
                measureIds.push_back(plm->getId());
                storedMeasure.insert({ plm->getId(), {{plm->getMeasures().front().origin,plm->getMeasures().back().final} } });
            }
        }
    }

    if (measurePoints.empty())
        return true;

    // Reallocate the memory only if needed
    MeasureData::checkBufferSize(measurePoints, measureColors, measureIds);
    m_measureCount = (uint32_t)measurePoints.size() / 2;
    MeasureData::storeInBuffer(measurePoints, measureColors, measureIds);

    return true;
}
