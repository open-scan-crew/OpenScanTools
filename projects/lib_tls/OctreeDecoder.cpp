#include "OctreeDecoder.h"
#include <map>

using namespace tls;

OctreeDecoder::OctreeDecoder()
    : OctreeBase()
    , m_encodedBuffers(std::vector<char*>(0, nullptr))
{
}

OctreeDecoder::~OctreeDecoder()
{
    cleanBuffers();
}

void OctreeDecoder::cleanBuffers()
{
    if (m_encodedBuffers.empty())
        return;
    for (char* ebuf : m_encodedBuffers)
        if (ebuf)
            delete ebuf;
    m_encodedBuffers.clear();
}

bool OctreeDecoder::readPointsFromFile(std::fstream& _fs, const uint64_t& pointDataOffset)
{
    m_encodedBuffers.resize(m_cellCount);
    memset(m_encodedBuffers.data(), 0, sizeof(char*) * m_encodedBuffers.size());

    // NOTE(robin) - Organize the leaves to read them in the file order (faster)
    std::map<uint64_t, uint32_t> orderedCellToRead;

    for (uint32_t _cell_id(0); _cell_id < m_vTreeCells.size(); _cell_id++)
    {
        if (!m_vTreeCells[_cell_id].m_isLeaf)
            continue;

        orderedCellToRead.insert({ m_vTreeCells[_cell_id].m_dataOffset, _cell_id });
    }

    for (std::pair<uint64_t, uint32_t> cellOffset : orderedCellToRead)
    {
        m_encodedBuffers[cellOffset.second] = new char[m_vTreeCells[cellOffset.second].m_dataSize];

        _fs.seekg(pointDataOffset + cellOffset.first);
        _fs.read(m_encodedBuffers[cellOffset.second], m_vTreeCells[cellOffset.second].m_dataSize);
    }

    return true;
}

uint32_t OctreeDecoder::getCellCount() const
{
    return (uint32_t)m_vTreeCells.size();
}

uint32_t OctreeDecoder::getCellPointCount(uint32_t _cell_id) const
{
    return m_vTreeCells[_cell_id].m_layerIndexes[m_vTreeCells[_cell_id].m_depthSPT];
}

bool OctreeDecoder::isLeaf(uint32_t _cell_id) const
{
    return (m_vTreeCells[_cell_id].m_isLeaf);
}

bool OctreeDecoder::getNextPoints(Point* dst_buf, uint64_t dst_size, uint64_t& point_count)
{
    uint64_t buf_offset = 0;

    for (; current_cell_ < getCellCount(); ++current_cell_)
    {
        if (decoded_cell_ != current_cell_)
        {
            decodeCell(current_cell_);
            current_point_ = 0;
        }
        // Copy the points directly in the destination buffer
        if (copyCellPoints(current_cell_, dst_buf, dst_size, buf_offset) == false)
            break;
    }

    point_count = buf_offset;
    return (point_count > 0);
}

bool OctreeDecoder::copyCellPoints(uint32_t _cell_id, Point* _dst_buf, uint64_t _dst_size, uint64_t& _dst_offset)
{
    if (_cell_id >= m_vTreeCells.size())
    {
        return false;
    }

    if (!m_vTreeCells[_cell_id].m_isLeaf)
    {
        return true;
    }

    // Return if there is no space
    if (_dst_offset >= _dst_size)
        return false;

    uint64_t cpy_count = std::min(decoded_points_.size() - current_point_, _dst_size - _dst_offset);
    const void* src = decoded_points_.data() + current_point_;
    memcpy(_dst_buf + _dst_offset, src, cpy_count * sizeof(Point));

    current_point_ += (uint32_t)cpy_count;
    _dst_offset += cpy_count;

    return (current_point_ >= decoded_points_.size());
}

void OctreeDecoder::decodeCell(uint32_t _cell_id)
{
    if (_cell_id >= m_vTreeCells.size() || !(m_vTreeCells[_cell_id].m_isLeaf))
        return;

    const TreeCell& cell = m_vTreeCells[_cell_id];
    uint64_t point_count = cell.m_layerIndexes[cell.m_depthSPT];

    // Select the source of the buffered encoded points 
    const char* srcPoints = nullptr;
    if (_cell_id >= m_encodedBuffers.size() || m_encodedBuffers[_cell_id] == nullptr)
    {
        return;
    }
    else
    {
        srcPoints = m_encodedBuffers[_cell_id];
    }

    decoded_points_.clear();
    decoded_points_.resize(point_count);
    decoded_cell_ = _cell_id;

    // Select the destination
    for (uint64_t i = 0; i < point_count; i++)
    {
        Coord16& coord = ((Coord16*)srcPoints)[i];
        decoded_points_[i].x = coord.x * m_precisionValue + cell.m_position[0];
        decoded_points_[i].y = coord.y * m_precisionValue + cell.m_position[1];
        decoded_points_[i].z = coord.z * m_precisionValue + cell.m_position[2];
    }
    if (m_ptFormat != PointFormat::TL_POINT_FORMAT_UNDEFINED)
    {
        if (m_ptFormat != PointFormat::TL_POINT_XYZ_RGB)
        {
            uint8_t* packI = (uint8_t*)(srcPoints + cell.m_iOffset);
            for (uint64_t i = 0; i < point_count; i++)
            {
                decoded_points_[i].i = packI[i];
            }
        }

        if (m_ptFormat != PointFormat::TL_POINT_XYZ_I)
        {
            Color24* packRGB = (Color24*)(srcPoints + cell.m_rgbOffset);
            for (uint64_t i = 0; i < point_count; i++)
            {
                decoded_points_[i].r = packRGB[i].r;
                decoded_points_[i].g = packRGB[i].g;
                decoded_points_[i].b = packRGB[i].b;
            }
        }
    }
}
