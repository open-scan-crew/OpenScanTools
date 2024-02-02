#ifndef OCTREE_DECODER_H
#define OCTREE_DECODER_H

#include "pointCloudEngine/OctreeBase.h"


class OctreeDecoder : public OctreeBase // IPointReader
{
public:
    OctreeDecoder(OctreeBase const& base, tls::BoundingBox const& bbox);
	OctreeDecoder(OctreeBase const& base);
    ~OctreeDecoder();

    bool readPointsFromFile(std::ifstream& is, const uint64_t& pointDataOffset, const uint64_t& pointDataSize);
    //bool decode();

    uint32_t getCellCount();
    uint32_t getCellPointCount(uint32_t cellId);

    bool isLeaf(uint32_t cellId) const;
    const PointXYZIRGB* getCellPoints(uint32_t cellId, uint64_t& pointCount);
    bool copyCellPoints(uint32_t cellId, PointXYZIRGB* dstPoints, uint64_t dstSize, uint64_t& dstOffset);

	const tls::BoundingBox& getBBox() const;
protected:
    OctreeDecoder() {};
	void cleanBuffers();
	void cleanEncodedBuffers();
	void decodeCell(uint32_t cellId, PointXYZIRGB* optionalOutput = nullptr);


protected:
    std::vector<char*> m_encodedBuffers;
    std::vector<PointXYZIRGB*> m_decodedBuffers;

    bool m_isEncodedBuffered;
    bool m_isDecoded;
	tls::BoundingBox m_bbox;
};

#endif // !OCTREE_READER_H