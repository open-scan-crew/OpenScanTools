#pragma once

#include <assert.h>
#include <vector>
#include "algo_helpers.h"
#include "StreamScan.h"



struct OctreeNode {

	unsigned pointId;
	unsigned pointCount;

	NodeId children[8];
	bool isLeaf;
};

struct Octree {

	Position center;
	float size;

	OctreeNode const& getNode(NodeId nodeId) const { return _nodes[nodeId]; }

	std::vector<OctreeNode> _nodes;
};

void buildFakeOctree(Octree& octree, float size, std::vector<PointColor>& points, Color const& color);


struct ScanFake : public IScan {

	ScanFake() : m_encodingSize(sizeof(PointColor)) {};
	ScanFake(Position _center, float _size, Color _color) : m_encodingSize(sizeof(PointColor)), color(_color) {
		buildFakeOctree(octree, _size, points, color);
		octree.center = _center;
	};

	Octree octree;
	std::vector<PointColor> points;
	// mat4 transfo;
	unsigned m_encodingSize;
	Color color;

	NodeId const getRootNodeId() const override { return 0; }
	NodeId const getNodeChildId(NodeId id, unsigned _child) const override {
		if (_child < 8) return octree.getNode(id).children[_child];
		else return 0;
	}
	bool isNodeALeaf(NodeId nodeId) const { return octree.getNode(nodeId).isLeaf; }
	Position getOctreeCenter() const override { return octree.center; }
	float getOctreeSize() const override { return octree.size; }
	Color getColor() const { return color; }

	uint32_t getSizeofPoints(NodeId nodeId) const override {

		return octree._nodes[nodeId].pointCount * m_encodingSize;
	}

	void readPoints(NodeId nodeId, void *buffer, uint32_t bufferSize) {

		uint64_t srcByteOffset = (uint64_t)octree._nodes[nodeId].pointId * m_encodingSize;
		uint32_t sizeofSrc = octree._nodes[nodeId].pointCount * m_encodingSize;

		assert(bufferSize >= sizeofSrc);

		const void *src = (const char*)points.data() + srcByteOffset;
		memcpy(buffer, src, sizeofSrc);
	}
};
