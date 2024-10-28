#ifndef GRID_CALCULATION_H_
#define GRID_CALCULATION_H_

#include "models/3d/GridBox.h"
#include "models/graph/BoxNode.h"
#include <glm/glm.hpp>
#include <vector>

#define MAX_BOXES 2000000

class MeshBuffer;

class GridCalculation
{
public:
	GridCalculation();
	~GridCalculation();

	static bool calculateBoxes(std::vector<GridBox>& ret_grid, const BoxNode& box);
	static bool calculateBoxesByStep(std::vector<GridBox>& grid, const TransformationModule& transfo, const glm::vec3& step);
	static bool calculateBoxesByMultiple(std::vector<GridBox>& grid, const TransformationModule& transfo, const glm::ivec3& division);

	static bool allocGridMeshByStep(MeshBuffer& meshBuf, const TransformationModule& box, const glm::vec3& step);
	static bool allocGridMeshByMultiple(MeshBuffer& meshBuf, const TransformationModule& box, const glm::ivec3& division);
};
#endif // !GRID_CALCULATION_H_