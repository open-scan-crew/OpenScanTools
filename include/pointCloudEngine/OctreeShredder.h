#ifndef OCTREE_SHREDDER_H
#define OCTREE_SHREDDER_H

#include "pointCloudEngine/OctreeBase.h"
#include "io/exports/TlsWriter.h"
#include "models/data/clipping/ClippingGeometry.h"

#include <glm/glm.hpp>
#include <filesystem>

class OctreeShredder : public OctreeBase
{
public:
    OctreeShredder(const std::filesystem::path& tlsPath);
    ~OctreeShredder();

    bool isEmpty();
    uint64_t cutPoints(const glm::dmat4& modelMat, const ClippingAssembly& clippingAssembly);
    bool save(const std::filesystem::path& savePath, const tls::ScanHeader& header);

    const std::vector<uint32_t>& getCorrespCellId();

protected:
    OctreeShredder() {};

    void cutCells(uint32_t _cellId, const ClippingAssembly& clippingAssembly);

    void clipAndReforgeSPT(uint32_t cellId, const ClippingAssembly& clippingAssembly);

    bool clearEmptyCells(uint32_t cellId);
    void repairEmptyBranches(uint32_t cellId);

    /*!
     * \brief The aim of this function is to create on single block of memory containing all
     *  the point data attached to the remaining cells.
     *  This function use the same buffer `m_pointData` as a source and destination in order
     *  to avoid a double cost of memory.
     *  To achieve that, each block of data already stored in `m_pointData` is shifted in their
     *  order of apparition (overlapping is considered).
     *  The data not wanted anymore (because of cell entirely clipped OR point clipped) are
     *  simply not rewritten.
     *
     * \Example
     *  Original arrangement of `m_pointData`:
     *    |A____|--|D___|---|E__|------|C_____|B___|---|
     *  Final arrangement:
     *    |A____|D___|E__|C_____|B___|-----------------|
     */
    void shiftData();
    void generateInstanceData();
    void logStats();

    TLS_WRITER_DECLARE(OctreeShredder);

private:
    uint64_t m_pointDataCapacity;
    uint64_t m_pointDataSize;
    char* m_pointData;
    uint64_t m_instanceDataSize;
    char* m_instanceData;
    char* m_newInstanceData;
    std::vector<TreeCell> m_newTreeCells;
    std::vector<uint32_t> m_correspCellId;

    // Statistics
    uint64_t m_deletedLeafPoints;
    uint64_t m_deletedBranchPoints;
    uint64_t m_deletedCells;
    uint64_t m_reforgedCells;
    uint64_t m_totalSphereTests;
    uint64_t m_totalCellTested;
    uint64_t m_totalPointTests;
};

#endif // !OCTREE_READER_H