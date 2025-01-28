#include "pointCloudEngine/OctreeShredder.h"

#include "utils/Logger.h"

#include "tls_core.h"
#include "tls_impl.h"
#include "tls_Point.h"

#include <map>

OctreeShredder::OctreeShredder(const std::filesystem::path& tlsPath)
    : m_deletedLeafPoints(0)
    , m_deletedBranchPoints(0)
    , m_deletedCells(0)
    , m_reforgedCells(0)
    , m_totalSphereTests(0)
    , m_totalCellTested(0)
{
    tls::ImageFile_p img_file(tlsPath, tls::usage::read);

    if (!img_file.is_valid_file())
    {
        Logger::log(IOLog) << "Failed to open the tls file at: " << tlsPath << Logger::endl;
        return;
    }

    if (img_file.getOctreeBase(0, *(tls::OctreeBase*)this) == false)
    {
        Logger::log(IOLog) << "Failed to read the data at: " << tlsPath << Logger::endl;
        return;
    }

    // Read the data in one block
    img_file.copyRawData(0, &m_vertexData, m_vertexDataSize, &m_instData, m_instDataSize);
}

OctreeShredder::~OctreeShredder()
{}

bool OctreeShredder::isEmpty()
{
    return (m_newTreeCells.empty());
}

uint64_t OctreeShredder::cutPoints(const glm::dmat4& _modelMat, const ClippingAssembly& _clippingAssembly)
{
    ClippingAssembly localAssembly = _clippingAssembly;
    localAssembly.clearMatrix(); // Reset the matrix with the stored value
    localAssembly.addTransformation(_modelMat);

    cutCells(m_uRootCell, localAssembly);

    //^~v~^~v~^~v~^~v~^~v~^~v~^~v~^~v~^~v~^~v~^~v~^~v~^~v~.
    //   Delete the empty cells, simplify the octree,     |
    //____________________________________________________/

    // Init the correspondance map between the old cellIds and the new ones
    m_correspCellId.resize(m_vTreeCells.size());
    for (uint32_t& id : m_correspCellId)
        id = NO_CHILD;

    clearEmptyCells(m_uRootCell);
    // We do not need the old cells
    m_deletedCells = m_vTreeCells.size() - m_newTreeCells.size();
    m_vTreeCells.clear();
    m_uRootCell = m_correspCellId[m_uRootCell];
    if (m_newTreeCells.size() == 0 || m_uRootCell == NO_CHILD)
        return m_pointCount;

    //----------------------------------------------------.
    //   repack instances, repack data                    |
    //----------------------------------------------------'
    shiftInstanceData();
    repairEmptyBranches(m_uRootCell);
    shiftData();

    // Update the OctreeBase infos
    m_pointCount -= m_deletedLeafPoints;
    m_cellCount = (uint32_t)m_newTreeCells.size();

    logStats();

    return m_deletedLeafPoints;
}

bool OctreeShredder::save(const std::filesystem::path& savePath, const tls::ScanHeader& header)
{
    tls::ImageFile_p img_file(savePath, tls::usage::write);

    if (!img_file.is_valid_file())
    {
        Logger::log(IOLog) << "An error occured while opening the TLS file " << savePath << Logger::endl;
        return false;
    }

    if (img_file.writeOctreeBase(0, *(OctreeBase*)this, header) == false)
    {
        Logger::log(IOLog) << "An error occured while writing the TLS file " << savePath << Logger::endl;
        return false;
    }

    return true;
}

const std::vector<uint32_t>& OctreeShredder::getCorrespCellId()
{
    return m_correspCellId;
}

// To do the cut:
//   a. Determine the cells inside the clipping (keep, partial keep, reject)
//   b. Erase the rejected cells
//   c. Decode, test and recompose the SPT for the "partial" cells
void OctreeShredder::cutCells(uint32_t _cellId, const ClippingAssembly& _clippingAssembly)
{
    assert(_cellId != NO_CHILD && _cellId < m_vTreeCells.size());
    TreeCell& cell = m_vTreeCells[_cellId];

    // --------------------------------------------------
    //  a. Clip the sphere representing the cell boundary |
    // --------------------------------------------------

    bool acceptAllClipping = false;
    bool rejectAllClipping = false;
    bool clipIndividualPoints = false;
    ClippingAssembly assemblyRemainder;
    
    double radius = cell.m_size * sqrt(3.0) / 2.0;
    glm::dvec4 center(cell.m_position[0] + cell.m_size / 2.0,
        cell.m_position[1] + cell.m_size / 2.0,
        cell.m_position[2] + cell.m_size / 2.0, 1.0);

    // Test the assembly
    m_totalSphereTests += _clippingAssembly.clippingUnion.size();
    m_totalSphereTests += _clippingAssembly.clippingIntersection.size();
    m_totalCellTested++;
    _clippingAssembly.testSphere(center, radius, acceptAllClipping, rejectAllClipping, assemblyRemainder);

    clipIndividualPoints = !acceptAllClipping && !rejectAllClipping;

    // ----------------------------------------------------
    //  b. Erase the rejected cells                       |
    // ----------------------------------------------------

    if (rejectAllClipping)
    {
        if (cell.m_isLeaf)
            m_deletedLeafPoints += cell.m_layerIndexes[cell.m_depthSPT];
        else
            m_deletedBranchPoints += cell.m_layerIndexes[cell.m_depthSPT];
        // We simply indicate that the cell now contain 0 points
        // The real data erasure will be done by the octree simplification
        cell.m_depthSPT = 0;
        memset(cell.m_layerIndexes, 0, 16 * sizeof(uint32_t));
    }

    //------------------------------------------------------------------.
    //  c. Decode, test and recompose the SPT for the "partial" cells   |
    //------------------------------------------------------------------'

    if (clipIndividualPoints)
    {
        clipAndReforgeSPT(_cellId, _clippingAssembly);
    }

    // ------------------------------------------.
    //                 Recurssion                |
    // ------------------------------------------'

    // Check the children for the node with not enough points
    if (!cell.m_isLeaf)
    {
        // Do the recursion on the children
        for (int j = 0; j < 8; j++)
        {
            if (cell.m_children[j] == NO_CHILD)
                continue;

            cutCells(cell.m_children[j], assemblyRemainder);
        }
    }

}

void OctreeShredder::clipAndReforgeSPT(uint32_t _cellId, const ClippingAssembly& _clippingAssembly)
{
    assert(_cellId != NO_CHILD && _cellId < m_vTreeCells.size());
    TreeCell& cell = m_vTreeCells[_cellId];
    uint64_t nbOfPoints(cell.m_layerIndexes[cell.m_depthSPT]);
    uint64_t deleteCount = 0;
    m_totalPointTests += (_clippingAssembly.clippingUnion.size() + _clippingAssembly.clippingIntersection.size()) * nbOfPoints;

    uint32_t tempLayerRanges[MAX_SPT_DEPTH];
    memset(tempLayerRanges, 0, MAX_SPT_DEPTH * sizeof(uint32_t));

    char* srcPoints = m_vertexData + cell.m_dataOffset;
    const tls::Coord16* srcXYZ = (tls::Coord16*)(srcPoints);
    const uint8_t* srcI = (uint8_t*)(srcPoints + cell.m_iOffset);
    const tls::Color24* srcRGB = (tls::Color24*)(srcPoints + cell.m_rgbOffset);
    std::vector<tls::Coord16> tempXYZ;
    std::vector<uint8_t> tempI;
    std::vector<tls::Color24> tempRGB;
    tempXYZ.resize(nbOfPoints);
    tempI.resize(nbOfPoints);
    tempRGB.resize(nbOfPoints);
    uint32_t tempCount = 0;

    // NOTE - La précision des branches n'est pas indiqué clairement dans l'octree. Il faut aller la chercher dans les données déjà préparées pour le buffer GPU.
    float precision = ((float*)m_instData)[4 * _cellId + 3];

    uint32_t p = 0;
    for (uint64_t layer = 0; layer < MAX_SPT_DEPTH; layer++)
    {
        for (; p < cell.m_layerIndexes[layer]; p++)
        {
            tls::Coord16& coord = ((tls::Coord16*)srcPoints)[p];

            glm::dvec4 point;
            point.x = coord.x * precision + cell.m_position[0];
            point.y = coord.y * precision + cell.m_position[1];
            point.z = coord.z * precision + cell.m_position[2];
            point.w = 1.0;

            if (_clippingAssembly.testPoint(point))
            {
                // keep point
                tempXYZ[tempCount] = srcXYZ[p];
                if (m_ptFormat != tls::PointFormat::TL_POINT_XYZ_RGB)
                    tempI[tempCount] = srcI[p];
                if (m_ptFormat != tls::PointFormat::TL_POINT_XYZ_I)
                    tempRGB[tempCount] = srcRGB[p];
                tempCount++;
            }
            else
            {
                deleteCount++;
            }
        }
        tempLayerRanges[layer] = tempCount;
    }
    if (cell.m_isLeaf)
        m_deletedLeafPoints += deleteCount;
    else
        m_deletedBranchPoints += deleteCount;

    // Copy the new layer indexes
    memcpy(cell.m_layerIndexes, tempLayerRanges, MAX_SPT_DEPTH * sizeof(uint32_t));

    if (cell.m_layerIndexes[cell.m_depthSPT] == 0)
    {
        return;
    }

    // Validate the integrity of the layers. Each layer index must be != 0
    // We know that the cell contains at least 1 point, we want each layer to contain that point.
    for (int l = 0; l < MAX_SPT_DEPTH; ++l)
    {
        if (cell.m_layerIndexes[l] == 0)
            cell.m_layerIndexes[l] = 1;
    }

    // copy tempXYZ
    memcpy(srcPoints, tempXYZ.data(), tempCount * sizeof(tls::Coord16));
    uint32_t offset = tempCount * sizeof(tls::Coord16);

    cell.m_iOffset = offset;
    if (m_ptFormat != tls::PointFormat::TL_POINT_XYZ_RGB)
    {
        // copy tempI
        memcpy(srcPoints + offset, tempI.data(), tempCount * sizeof(uint8_t));
        offset += tempCount * sizeof(uint8_t);
    }

    cell.m_rgbOffset = offset;
    if (m_ptFormat != tls::PointFormat::TL_POINT_XYZ_I)
    {
        // copy tempRGB
        memcpy(srcPoints + offset, tempRGB.data(), tempCount * sizeof(tls::Color24));
        offset += tempCount * sizeof(tls::Color24);
    }

    cell.m_dataSize = aligned(offset, 2);

    m_reforgedCells++;
}

bool OctreeShredder::clearEmptyCells(uint32_t _cellId)
{
    assert(_cellId != NO_CHILD && _cellId < m_vTreeCells.size());
    TreeCell& cell = m_vTreeCells[_cellId];

    bool pointsRemaining = (cell.m_layerIndexes[cell.m_depthSPT] > 0);
    // Check the children for the node with not enough points
    if (!cell.m_isLeaf)
    {
        // Do the recursion on the children
        for (int j = 0; j < 8; j++)
        {
            if (cell.m_children[j] == NO_CHILD)
                continue;

            pointsRemaining |= clearEmptyCells(cell.m_children[j]);
            // Rebind the children
            cell.m_children[j] = m_correspCellId[cell.m_children[j]];
        }
    }

    // Problem : we can have 0 points in branch after a cut, but the children still have points.
    // This can occur because the SPT of a branch is not recomposed from its children.
    if (pointsRemaining)
    {
        m_newTreeCells.emplace_back(cell);
        m_correspCellId[_cellId] = (uint32_t)m_newTreeCells.size() - 1;
    }

    return pointsRemaining;
}

void OctreeShredder::repairEmptyBranches(uint32_t _cellId)
{
    assert(_cellId != NO_CHILD && _cellId < m_newTreeCells.size());
    TreeCell& cell = m_newTreeCells[_cellId];
    if (cell.m_isLeaf)
        return;

    bool repair = (cell.m_layerIndexes[cell.m_depthSPT] == 0);

    for (int j = 0; j < 8; j++)
    {
        uint32_t childId = cell.m_children[j];
        if (childId != NO_CHILD)
        {
            repairEmptyBranches(childId);
            if (repair) // We need to copy the first point of the first children with points
            {
                // Get first point from child
                TreeCell& child = m_newTreeCells[childId];
                char* childPoints = m_vertexData + child.m_dataOffset;
                float childPrecision = ((float*)m_instData)[4 * childId + 3];

                tls::Coord16& childCoord = ((tls::Coord16*)childPoints)[0];
                tls::Point point;
                point.x = childCoord.x * childPrecision + child.m_position[0];
                point.y = childCoord.y * childPrecision + child.m_position[1];
                point.z = childCoord.z * childPrecision + child.m_position[2];

                // Encode the point with the cell precision and offset
                char* cellPoints = m_vertexData + cell.m_dataOffset;
                float precision = ((float*)m_instData)[4 * _cellId + 3];
                cell.m_depthSPT = 0;
                cell.m_layerIndexes[0] = 1;

                tls::Coord16 cellCoord(point, precision, cell.m_position);
                memcpy(cellPoints, &cellCoord, sizeof(tls::Coord16));

                // Copy I and RGB
                cell.m_iOffset = 6;
                if (m_ptFormat != tls::PointFormat::TL_POINT_XYZ_RGB)
                    cell.m_rgbOffset = 7;
                else
                    cell.m_rgbOffset = 6;

                if (m_ptFormat != tls::PointFormat::TL_POINT_XYZ_RGB)
                    memcpy(cellPoints + cell.m_iOffset, childPoints + child.m_iOffset, sizeof(uint8_t));
                if (m_ptFormat != tls::PointFormat::TL_POINT_XYZ_I)
                    memcpy(cellPoints + cell.m_rgbOffset, childPoints + child.m_rgbOffset, sizeof(tls::Color24));

                repair = false;
            }
        }
    }
}

// 
void OctreeShredder::shiftData()
{
    //   map<data offset, cellId>
    std::map<uint64_t, uint32_t> cellSortedByDataOffset;

    for (uint32_t cellId = 0; cellId < m_newTreeCells.size(); ++cellId)
    {
        const TreeCell& cell = m_newTreeCells[cellId];
        cellSortedByDataOffset.insert({ cell.m_dataOffset, cellId });
    }

    uint64_t currentOffset = 0;
    for (auto it : cellSortedByDataOffset)
    {
        TreeCell& cell = m_newTreeCells[it.second];
        // Detect skip copy
        if (currentOffset != cell.m_dataOffset)
        {
            // Detect overlap
            if (currentOffset + cell.m_dataSize >= cell.m_dataOffset)
            {
                char* temp = new char[cell.m_dataSize];
                memcpy(temp, m_vertexData + cell.m_dataOffset, cell.m_dataSize);
                memcpy(m_vertexData + currentOffset, temp, cell.m_dataSize);
                delete temp;
            }
            else
            {
                memcpy(m_vertexData + currentOffset, m_vertexData + cell.m_dataOffset, cell.m_dataSize);
            }
        }
        cell.m_dataOffset = currentOffset;
        currentOffset += cell.m_dataSize;
    }
    m_vertexDataSize = currentOffset;
}

// FIXME - A better (and safer) process would be to regenerate the instance data from the cell data (position, precision)
void OctreeShredder::shiftInstanceData()
{
    // Keep temporarily the old data
    char* old_inst_data = m_instData;
    size_t old_inst_size = m_instDataSize;

    // Reset the stored data
    m_instDataSize = 4 * m_newTreeCells.size() * sizeof(float);
    m_instData = new char[m_instDataSize];
    memset(m_instData, 0, m_instDataSize);

    for (uint32_t cellId = 0; cellId < m_correspCellId.size(); ++cellId)
    {
        uint32_t newCellId = m_correspCellId[cellId];
        size_t src_offset = cellId * 16;
        size_t dst_offset = newCellId * 16;
        if (newCellId != NO_CHILD &&
            src_offset + 16 < old_inst_size &&
            dst_offset + 16 < m_instDataSize)
        {
            memcpy(m_instData + dst_offset, old_inst_data + src_offset, 16);
        }
    }

    delete old_inst_data;
}

void OctreeShredder::logStats()
{
    SubLogger& log = Logger::log(IOLog);
    log << "***** Octree Shredder stats *****\n";
    log << "* Leaf points deleted:   " << m_deletedLeafPoints << "\n";
    log << "* Branch points deleted: " << m_deletedBranchPoints << "\n";
    log << "* Deleted cells:         " << m_deletedCells << "\n";
    log << "* Reforged cells:        " << m_reforgedCells << "\n";
    log << "* Total sphere tests:    " << m_totalSphereTests << "\n";
    float testPerCell = (float)m_totalSphereTests / m_totalCellTested;
    log << "* Sphere test per cell:  " << testPerCell << "\n";
    log << "* Total point tests:     " << m_totalPointTests << "\n";
    float testPerPointDeleted = (float)(m_totalPointTests) / (m_deletedLeafPoints + m_deletedBranchPoints);
    log << "* Point tests per point deleted: " << testPerPointDeleted << "\n";
    log << Logger::endl;
}
