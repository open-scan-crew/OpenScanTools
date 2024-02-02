#ifndef GRID_CALCULATION_H_
#define GRID_CALCULATION_H_

#include "models/3d/GridBox.h"
#include <glm/glm.hpp>
#include <deque>
#include <vector>

#define MAX_BOXES 0xFFFF

enum class GridType;
class MeshBuffer;

class GridCalculation
{
public:
	GridCalculation();
	~GridCalculation();

	static bool calculateBoxes(std::deque<GridBox>& grid, const TransformationModule& box, const glm::vec3& value, const GridType& type);
	static bool calculateBoxesByStep(std::deque<GridBox>& grid, const TransformationModule& box, const glm::vec3& step);
	static bool calculateBoxesByMultiple(std::deque<GridBox>& grid, const TransformationModule& box, const glm::ivec3& division);

	static bool allocGridMeshByStep(MeshBuffer& meshBuf, const TransformationModule& box, const glm::vec3& step);
	static bool allocGridMeshByMultiple(MeshBuffer& meshBuf, const TransformationModule& box, const glm::ivec3& division);

protected:
	static void addLine(std::vector<glm::vec3>&	linesPositions, std::vector<uint32_t>& linesIndices, const glm::ivec3& gridPosition, const glm::vec3& step, const glm::vec3& offset);
	static void addLine(std::vector<glm::vec3>& linesPositions, std::vector<uint32_t>& linesIndices, const glm::vec3& begin, const glm::vec3& end, const glm::vec3& offset);
};
#endif // !GRID_CALCULATION_H_