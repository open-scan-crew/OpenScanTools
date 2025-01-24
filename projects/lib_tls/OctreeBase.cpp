#include "OctreeBase.h"

//--- Constructors ---//

using namespace tls;

OctreeBase::OctreeBase() : OctreeBase(tls::PrecisionType::TL_OCTREE_100UM, tls::TL_POINT_XYZ_I_RGB)
{}

OctreeBase::OctreeBase(const tls::PrecisionType& _precisionType, const tls::PointFormat& _ptFormat)
    : m_precisionType(_precisionType)
    , m_precisionValue(tls::getPrecisionValue(_precisionType))
    , m_ptFormat(_ptFormat)
    , m_rootSize(0.f)
    , m_rootPosition{ 0.f, 0.f, 0.f }
    , m_limits{ std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity() }
    , m_pointCount(0)
    , m_cellCount(0)
    , m_uRootCell(0)
{
    switch (m_precisionType) {
    case tls::PrecisionType::TL_OCTREE_1MM:
        m_rootSize = 64.f;
        m_maxLeafSize = 64.f;
        break;
    case tls::PrecisionType::TL_OCTREE_100UM:
        m_rootSize = 8.f;
        m_maxLeafSize = 8.f;
        break;
    case tls::PrecisionType::TL_OCTREE_10UM:
        m_rootSize = 1.f;
        m_maxLeafSize = 1.f;
        break;
    default:
        break;
    }
    m_minLeafSize = tls::getPrecisionValue(m_precisionType);
}

OctreeBase::~OctreeBase()
{
    if (m_vertexData != nullptr)
        delete[] m_vertexData;
    if (m_instData != nullptr)
        delete[] m_instData;
}

//--- Functions ---//

tls::PointFormat OctreeBase::getPointFormat() const
{
    return m_ptFormat;
}

void OctreeBase::setPointFormat(tls::PointFormat format)
{
    m_ptFormat = format;
}

uint64_t OctreeBase::getPointCount() const
{
    return m_pointCount;
}

const tls::Limits& OctreeBase::getLimits() const
{
    return m_limits;
}

const std::vector<TreeCell>& OctreeBase::getData() const
{
    return m_vTreeCells;
}

int64_t OctreeBase::getCellCount() const
{
    return m_cellCount;
}