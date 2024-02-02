#ifndef MEASURE_DRAWDATA_H
#define MEASURE_DRAWDATA_H

#include <vector>
#include <list>
#include <unordered_map>

#include "pointCloudEngine/SmartBuffer.h"
#include "models/3d/Measures.h"
#include "models/data/Data.h"

class AMeasureNode;

struct PointBufferData
{
public:
    PointBufferData(const glm::dvec3& _point);
    PointBufferData(const glm::dvec3& _first, const glm::dvec3& _second);

private:
    float position[3];
};

class MeasureDrawData
{
public:
    MeasureDrawData();
    ~MeasureDrawData();

    virtual bool store(std::vector<Data*>& _measures, std::unordered_map<xg::Guid, std::list<Measure>>& storedMeasure) = 0 {};
    bool storeNode(const std::unordered_map<xg::Guid, AMeasureNode*>& nodes);
    SimpleBuffer const& getSBuffer() const;
    VkBuffer const& getBuffer() const;
    uint32_t getMeasureCount() const;
    VkDeviceSize const& getPositionsOffset() const;
    VkDeviceSize const& getColorsOffset() const;
    VkDeviceSize const& getIdsOffset() const;

protected:
    void reset(VkDeviceSize _size);
    virtual void checkBufferSize(std::vector<PointBufferData> const&, std::vector<uint32_t> const&, std::vector<uint32_t> const&) final;
    void storeInBuffer(std::vector<PointBufferData> const& measurePoints, std::vector<uint32_t> const& measureColors, std::vector<uint32_t> const& measureIds);

    SimpleBuffer m_sbuf;

    uint32_t m_measureCount;
    VkDeviceSize m_positionsOffset;
    VkDeviceSize m_colorsOffset;
    VkDeviceSize m_idsOffset;
};

class SimpleMeasureDrawData : public MeasureDrawData
{
public:
    SimpleMeasureDrawData();
    ~SimpleMeasureDrawData();
    bool store(std::vector<Data*>& _measures, std::unordered_map<xg::Guid, std::list<Measure>>& storedMeasure) override;
};

class ComplexeMeasureDrawData : public MeasureDrawData
{
public:
    ComplexeMeasureDrawData();
    ~ComplexeMeasureDrawData();
    bool store(std::vector<Data*>& _measures, std::unordered_map<xg::Guid, std::list<Measure>>& storedMeasure) override;
};

class PolylineMeasureDrawData : public MeasureDrawData
{
public:
    PolylineMeasureDrawData();
    ~PolylineMeasureDrawData();
    bool store(std::vector<Data*>& _measures, std::unordered_map<xg::Guid, std::list<Measure>>& storedMeasure) override;
};

#endif