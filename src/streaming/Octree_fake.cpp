#include "Octree_fake.h"

//----------------------------------------------------------------------------

NodeId buildFakeOctreeNodes(Octree& octree, Position const& center, float size, unsigned depth, std::vector<PointColor>& points, Color const& color);

void buildFakeOctree(Octree& _octree, float _size, std::vector<PointColor> &_points, Color const& _color) {

	Position center(0, 0, 0);
	_octree.size = _size;
	buildFakeOctreeNodes(_octree, center, _size, 0, _points, _color);
}

static NodeId buildFakeOctreeNodes(Octree& octree, Position const& center, float size, unsigned depth, std::vector<PointColor>& points, Color const& color) {

	const NodeId nodeId = (NodeId)octree._nodes.size();

	octree._nodes.push_back(OctreeNode());

	octree._nodes.back().pointId = (unsigned)(points.size());

	float dy = size * 0.1f; // Let a space between the points and the face of the cube
	float y = (center.coord[1] < 0) ? -dy : dy;
	float e = size * 0.25f;
	points.push_back({ center.coord[0] - e, y, center.coord[2] - e ,
		               (uint8_t)(color.rgb[0] * 255.f), (uint8_t)(color.rgb[1] * 255.f), (uint8_t)(color.rgb[2] * 255.f) });
	points.push_back({ center.coord[0] - e, y, center.coord[2] + e ,
					   (uint8_t)(color.rgb[0] * 255.f), (uint8_t)(color.rgb[1] * 255.f), (uint8_t)(color.rgb[2] * 255.f) });
	points.push_back({ center.coord[0] + e, y, center.coord[2] - e ,
					   (uint8_t)(color.rgb[0] * 255.f), (uint8_t)(color.rgb[1] * 255.f), (uint8_t)(color.rgb[2] * 255.f) });
	points.push_back({ center.coord[0] + e, y, center.coord[2] + e ,
					   (uint8_t)(color.rgb[0] * 255.f), (uint8_t)(color.rgb[1] * 255.f), (uint8_t)(color.rgb[2] * 255.f) });

	octree._nodes.back().pointCount = 4u;


	if (depth >= 5) {

		OctreeNode &currentNode = octree._nodes[nodeId];
		currentNode.isLeaf = true;
	}
	else {

		const float childSize = size * 0.5f;

		for (unsigned i = 0; i < 8; i++) {

			const float o = childSize * 0.5f;
			const float offset[3] = { ((i & 4) ? -o : o), ((i & 2) ? o : -o), ((i & 1) ? o : -o) };
			const Position childCenter(center.coord[0] + offset[0], center.coord[1] + offset[1], center.coord[2] + offset[2]);

			NodeId childId = 0;
			if (fabsf(childCenter.coord[1]) <= o) {
				childId = buildFakeOctreeNodes(octree, childCenter, childSize, depth + 1, points, color);
			}

			OctreeNode &currentNode = octree._nodes[nodeId];
			currentNode.children[i] = childId;
		}

		OctreeNode &currentNode = octree._nodes[nodeId];
		currentNode.isLeaf = false;
	}

	return nodeId;
}
