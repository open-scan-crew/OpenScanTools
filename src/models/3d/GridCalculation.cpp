#include "models/3d/GridCalculation.h"
#include "models/data/Grid/GridData.h"
#include "models/3d/MeshBuffer.h"

#include "vulkan/MeshManager.h"

#include <glm/gtx/quaternion.hpp>

GridCalculation::GridCalculation()
{}

GridCalculation::~GridCalculation()
{}

bool GridCalculation::calculateBoxes(std::deque<GridBox>& grid, const TransformationModule& box, const glm::vec3& value, const GridType& type)
{
	if (type == GridType::ByStep)
		return calculateBoxesByStep(grid, box, value);
	return calculateBoxesByMultiple(grid, box, value);
}

bool GridCalculation::calculateBoxesByStep(std::deque<GridBox>& grid, const TransformationModule& box, const glm::vec3& step)
{
	grid.clear();
	const glm::dvec3& scale = box.getScale();
	const glm::dvec3& center = box.getCenter();
	glm::dvec3 step2(step / 2.0f);

	glm::dvec3 maxIteration(glm::floor(scale / step2));
	glm::dvec3 lambda3(scale - maxIteration * step2);
	glm::ivec3 gridSize(maxIteration);

	if (maxIteration.x > MAX_BOXES || maxIteration.y > MAX_BOXES || maxIteration.z > MAX_BOXES)
		return false;

	GridBox gbox(glm::vec3(0.f), box.getOrientation(), step2);
	glm::dmat4 rotation(glm::toMat4(box.getOrientation()));
	for (uint64_t iteratorZ(0); iteratorZ <= gridSize.z; iteratorZ++)
	{
		for (uint64_t iteratorY(0); iteratorY <= gridSize.y; iteratorY++)
		{
			for (uint64_t iteratorX(0); iteratorX <= gridSize.x; iteratorX++)
			{
				glm::dvec3 gcenter((step2.x * (iteratorX * 2.0 + 1.0)) - scale.x,
									(step2.y * (iteratorY * 2.0 + 1.0)) - scale.y,
									(step2.z * (iteratorZ * 2.0 + 1.0)) - scale.z);
				glm::vec3 size = step;

				if (iteratorX == gridSize.x)
				{
					if (lambda3.x > 0.0)
					{
						gcenter.x = scale.x - lambda3.x;
						size.x = (float)lambda3.x * 2;
					}
					else
						continue;
				}
				if (iteratorY == gridSize.y)
				{
					if (lambda3.y > 0.0)
					{
						gcenter.y = scale.y - lambda3.y;
						size.y = (float)lambda3.y * 2;
					}
					else
						continue;
				}
				if (iteratorZ == gridSize.z)
				{
					if (lambda3.z > 0.0)
					{
						gcenter.z = scale.z - lambda3.z;
						size.z = (float)lambda3.z * 2;
					}
					else
						continue;
				}

				gcenter = rotation * glm::dvec4(gcenter, 1.0);
				gbox.setPosition(center + gcenter);
				gbox.setSize(size);
				gbox.position.x = (uint8_t)iteratorX;
				gbox.position.y = (uint8_t)iteratorY;
				gbox.position.z = (uint8_t)iteratorZ;
				grid.push_back(gbox);
			}
		}
	}

	return true;
}

bool GridCalculation::calculateBoxesByMultiple(std::deque<GridBox>& grid, const TransformationModule& box, const glm::ivec3& division)
{
	grid.clear();

	const glm::dvec3& size = box.getScale();
	const glm::dvec3& center = box.getCenter();

	float sizeX((float)size.x / division.x);
	float sizeY((float)size.y / division.y);
	float sizeZ((float)size.z / division.z);
							 
	if (division.x > MAX_BOXES || division.y > MAX_BOXES || division.z > MAX_BOXES)
		return false;
	
	GridBox gbox(glm::vec3(0.f), box.getOrientation(), glm::vec3(sizeX, sizeY, sizeZ));
	glm::dmat4 rotation(glm::toMat4(box.getOrientation()));
	for (uint64_t iteratorZ(0); iteratorZ < division.z; iteratorZ++)
	{
		for (uint64_t iteratorY(0); iteratorY < division.y; iteratorY++)
		{
			for (uint64_t iteratorX(0); iteratorX < division.x; iteratorX++)
			{
				glm::dvec3 gcenter((sizeX * (iteratorX * 2.0 + 1.0)) - size.x, 
									(sizeY * (iteratorY * 2.0 + 1.0)) - size.y,
									(sizeZ * (iteratorZ * 2.0 + 1.0)) - size.z);
				gcenter = rotation * glm::dvec4(gcenter,1.0);
				gbox.setPosition(center + gcenter);
				gbox.position.x = (uint8_t)iteratorX;
				gbox.position.y = (uint8_t)iteratorY;
				gbox.position.z = (uint8_t)iteratorZ;
				grid.push_back(gbox);
			}
		}
	}
	return true;
}

bool GridCalculation::allocGridMeshByStep(MeshBuffer& meshBuf, const TransformationModule& box, const glm::vec3& step)
{
	if (step.x == 0.f || step.y == 0.f || step.z == 0.f)
		return false;

	const glm::dvec3& size = box.getScale();
	const glm::dvec3& center = box.getCenter();

	float xLastIteration(((float)size.x * 2.0f) / step.x);
	float yLastIteration(((float)size.y * 2.0f) / step.y);
	float zLastIteration(((float)size.z * 2.0f) / step.z);

	glm::vec3 lambda(1.0f / xLastIteration,
		1.0f / yLastIteration,
		1.0f / zLastIteration);

	uint64_t maxXIteration(uint64_t(floor(xLastIteration)));
	uint64_t maxYIteration(uint64_t(floor(yLastIteration)));
	uint64_t maxZIteration(uint64_t(floor(zLastIteration)));

	xLastIteration -= maxXIteration;
	yLastIteration -= maxYIteration;
	zLastIteration -= maxZIteration;

	if (maxXIteration > MAX_BOXES || maxYIteration > MAX_BOXES || maxZIteration > MAX_BOXES)
		return false;

	std::vector<glm::vec3> linesPositions;
	std::vector<uint32_t> linesIndices;

	for (uint64_t iteratorZ(0); iteratorZ < maxZIteration; iteratorZ++)
		for (uint64_t iteratorY(0); iteratorY < maxYIteration; iteratorY++)
			for (uint64_t iteratorX(0); iteratorX < maxXIteration; iteratorX++)
				addLine(linesPositions, linesIndices, glm::ivec3(iteratorX, iteratorY, iteratorZ), lambda, { 0.5f, 0.5f ,0.5f });

	if (xLastIteration != 0.0f)
		for (uint64_t iteratorZ(0); iteratorZ < maxZIteration; iteratorZ++)
			for (uint64_t iteratorY(0); iteratorY < maxYIteration; iteratorY++)
				addLine(linesPositions, linesIndices, glm::vec3(lambda.x * maxXIteration , lambda.y * (iteratorY+1), lambda.z * (iteratorZ+1)),
					glm::vec3(1.0f, lambda.y * iteratorY, lambda.z * iteratorZ), { 0.5f, 0.5f ,0.5f });


	if (yLastIteration != 0.0f)
		for (uint64_t iteratorZ(0); iteratorZ < maxZIteration; iteratorZ++)
			for (uint64_t iteratorX(0); iteratorX < maxXIteration; iteratorX++)
				addLine(linesPositions, linesIndices, glm::vec3(lambda.x * (iteratorX + 1), lambda.y * maxYIteration, lambda.z * (iteratorZ + 1)),
					glm::vec3( lambda.x * iteratorX, 1.0f, lambda.z * iteratorZ), { 0.5f, 0.5f ,0.5f });

	
	if (zLastIteration != 0.0f)
		for (uint64_t iteratorX(0); iteratorX < maxXIteration; iteratorX++)
			for (uint64_t iteratorY(0); iteratorY < maxYIteration; iteratorY++)
				addLine(linesPositions, linesIndices, glm::vec3(lambda.x * (iteratorX + 1), lambda.y * (iteratorY + 1), lambda.z * maxZIteration ),
					glm::vec3(lambda.x * iteratorX, lambda.y * iteratorY, 1.0f), { 0.5f, 0.5f ,0.5f });


	if (xLastIteration != 0.0f && yLastIteration != 0.0f)
		for (uint64_t iteratorZ(0); iteratorZ < maxZIteration; iteratorZ++)

			addLine(linesPositions, linesIndices, glm::vec3(lambda.x * maxXIteration, lambda.y * maxYIteration, lambda.z * (iteratorZ + 1)),
				glm::vec3(1.0f, 1.0f, lambda.z * iteratorZ), { 0.5f, 0.5f ,0.5f });


	if (xLastIteration != 0.0f && zLastIteration != 0.0f)
		for (uint64_t iteratorY(0); iteratorY < maxYIteration; iteratorY++)
			addLine(linesPositions, linesIndices, glm::vec3(lambda.x * maxXIteration, lambda.y * (iteratorY + 1), lambda.z * maxZIteration),
				glm::vec3(1.0f, lambda.y * iteratorY, 1.0f), { 0.5f, 0.5f ,0.5f });


	if (yLastIteration != 0.0f && zLastIteration != 0.0f)
		for (uint64_t iteratorX(0); iteratorX < maxXIteration; iteratorX++)
			addLine(linesPositions, linesIndices, glm::vec3(lambda.x * (iteratorX + 1), lambda.y * maxYIteration , lambda.z * maxZIteration),
				glm::vec3(lambda.x * iteratorX, 1.0f, 1.0f), { 0.5f, 0.5f ,0.5f });


	if (xLastIteration != 0.0f && yLastIteration != 0.0f && zLastIteration != 0.0f)
		addLine(linesPositions, linesIndices, glm::vec3(lambda.x * maxXIteration, lambda.y * maxYIteration, lambda.z * maxZIteration),
			glm::vec3(1.0f, 1.0f, 1.0f), { 0.5f, 0.5f ,0.5f });

	meshBuf.addIndexedDraw(VK_PRIMITIVE_TOPOLOGY_LINE_LIST, { 0, 0, (uint32_t)linesIndices.size() });

	VkResult res(MeshManager::getInstance().uploadInMeshBuffer(meshBuf, linesPositions, {}, linesIndices));
	assert(res == VkResult::VK_SUCCESS);

	return res == VkResult::VK_SUCCESS;
}

bool GridCalculation::allocGridMeshByMultiple(MeshBuffer& meshBuf, const TransformationModule& box, const glm::ivec3& division)
{
	if (division.x == 0 || division.y == 0 || division.z == 0)
		return false;

	const glm::dvec3& size = box.getScale();
	const glm::dvec3& center = box.getCenter();

	float sizeX((float)size.x / division.x);
	float sizeY((float)size.y / division.y);
	float sizeZ((float)size.z / division.z);

	if (division.x > MAX_BOXES || division.y > MAX_BOXES || division.z > MAX_BOXES)
		return false;

	glm::vec3 lambda( 1.0f / division.x, 1.0f / division.y, 1.0f / division.z);

	std::vector<glm::vec3> linesPositions;
	std::vector<uint32_t> linesIndices;

	for (uint64_t iteratorZ(0); iteratorZ < division.z; iteratorZ++)
		for (uint64_t iteratorY(0); iteratorY < division.y; iteratorY++)
			for (uint64_t iteratorX(0); iteratorX < division.x; iteratorX++)
				addLine(linesPositions, linesIndices, glm::ivec3(iteratorX, iteratorY, iteratorZ), lambda, {0.5f, 0.5f ,0.5f });

	meshBuf.addIndexedDraw(VK_PRIMITIVE_TOPOLOGY_LINE_LIST, { 0, 0, (uint32_t)linesIndices.size() });

	VkResult res(MeshManager::getInstance().uploadInMeshBuffer(meshBuf, linesPositions, {}, linesIndices));
	assert(res == VkResult::VK_SUCCESS);

	return res == VkResult::VK_SUCCESS;
}

//TODO (aurélien) can be optimize (probably)
void GridCalculation::addLine(std::vector<glm::vec3>& linesPositions, std::vector<uint32_t>& linesIndices, const glm::ivec3& gridPosition, const glm::vec3& step, const glm::vec3& offset)
{
	uint32_t i_xyz = (uint32_t)linesPositions.size();
	linesPositions.push_back(glm::vec3((step.x * gridPosition.x) - offset.x, (step.y * gridPosition.y) - offset.y, (step.z * gridPosition.z - offset.z)) * 2.0f);
	uint32_t i_xyZ = (uint32_t)linesPositions.size();
	linesPositions.push_back(glm::vec3((step.x * gridPosition.x) - offset.x, (step.y * gridPosition.y) - offset.y, (step.z * (gridPosition.z + 1) - offset.z)) * 2.0f);
	uint32_t i_xYz = (uint32_t)linesPositions.size();
	linesPositions.push_back(glm::vec3((step.x * gridPosition.x) - offset.x, (step.y * (gridPosition.y + 1)) - offset.y, (step.z * gridPosition.z - offset.z)) * 2.0f);
	uint32_t i_xYZ = (uint32_t)linesPositions.size();
	linesPositions.push_back(glm::vec3((step.x * gridPosition.x) - offset.x, (step.y * (gridPosition.y + 1)) - offset.y, (step.z * (gridPosition.z + 1) - offset.z)) * 2.0f);
	uint32_t i_Xyz = (uint32_t)linesPositions.size();
	linesPositions.push_back(glm::vec3((step.x * (gridPosition.x + 1)) - offset.x, (step.y * gridPosition.y) - offset.y, (step.z * gridPosition.z - offset.z)) * 2.0f);
	uint32_t i_XyZ = (uint32_t)linesPositions.size();
	linesPositions.push_back(glm::vec3((step.x * (gridPosition.x + 1)) - offset.x, (step.y * gridPosition.y) - offset.y, (step.z * (gridPosition.z + 1) - offset.z)) * 2.0f);
	uint32_t i_XYz = (uint32_t)linesPositions.size();
	linesPositions.push_back(glm::vec3((step.x * (gridPosition.x + 1)) - offset.x, (step.y * (gridPosition.y + 1)) - offset.y, (step.z * gridPosition.z - offset.z)) * 2.0f);
	uint32_t i_XYZ = (uint32_t)linesPositions.size();
	linesPositions.push_back(glm::vec3((step.x * (gridPosition.x + 1)) - offset.x, (step.y * (gridPosition.y + 1)) - offset.y, (step.z * (gridPosition.z + 1) - offset.z)) * 2.0f);

	linesIndices.insert(linesIndices.end(), { i_xyz, i_xyZ });
	linesIndices.insert(linesIndices.end(), { i_xyz, i_xYz });
	linesIndices.insert(linesIndices.end(), { i_xyz, i_Xyz });

	linesIndices.insert(linesIndices.end(), { i_XYz, i_xYz });
	linesIndices.insert(linesIndices.end(), { i_XYz, i_Xyz });
	linesIndices.insert(linesIndices.end(), { i_XYz, i_XYZ });

	linesIndices.insert(linesIndices.end(), { i_XyZ, i_Xyz });
	linesIndices.insert(linesIndices.end(), { i_XyZ, i_xyZ });
	linesIndices.insert(linesIndices.end(), { i_XyZ, i_XYZ });

	linesIndices.insert(linesIndices.end(), { i_xYZ, i_xyZ });
	linesIndices.insert(linesIndices.end(), { i_xYZ, i_xYz });
	linesIndices.insert(linesIndices.end(), { i_xYZ, i_XYZ });
}

void GridCalculation::addLine(std::vector<glm::vec3>& linesPositions, std::vector<uint32_t>& linesIndices, const glm::vec3& begin, const glm::vec3& end, const glm::vec3& offset)
{
	uint32_t i_xyz = (uint32_t)linesPositions.size();
	linesPositions.push_back((begin - offset) * 2.0f);
	uint32_t i_xyZ = (uint32_t)linesPositions.size();
	linesPositions.push_back((glm::vec3(begin.x, begin.y, end.z) - offset) * 2.0f);
	uint32_t i_xYz = (uint32_t)linesPositions.size();
	linesPositions.push_back((glm::vec3(begin.x, end.y, begin.z) - offset) * 2.0f);
	uint32_t i_xYZ = (uint32_t)linesPositions.size();
	linesPositions.push_back((glm::vec3(begin.x, end.y, end.z) - offset) * 2.0f);
	uint32_t i_Xyz = (uint32_t)linesPositions.size();
	linesPositions.push_back((glm::vec3(end.x, begin.y, begin.z) - offset) * 2.0f);
	uint32_t i_XyZ = (uint32_t)linesPositions.size();
	linesPositions.push_back((glm::vec3(end.x, begin.y, end.z) - offset) * 2.0f);
	uint32_t i_XYz = (uint32_t)linesPositions.size();
	linesPositions.push_back((glm::vec3(end.x, end.y, begin.z) - offset) * 2.0f);
	uint32_t i_XYZ = (uint32_t)linesPositions.size();
	linesPositions.push_back((end - offset) * 2.0f);

	linesIndices.insert(linesIndices.end(), { i_xyz, i_xyZ });
	linesIndices.insert(linesIndices.end(), { i_xyz, i_xYz });
	linesIndices.insert(linesIndices.end(), { i_xyz, i_Xyz });

	linesIndices.insert(linesIndices.end(), { i_XYz, i_xYz });
	linesIndices.insert(linesIndices.end(), { i_XYz, i_Xyz });
	linesIndices.insert(linesIndices.end(), { i_XYz, i_XYZ });

	linesIndices.insert(linesIndices.end(), { i_XyZ, i_Xyz });
	linesIndices.insert(linesIndices.end(), { i_XyZ, i_xyZ });
	linesIndices.insert(linesIndices.end(), { i_XyZ, i_XYZ });

	linesIndices.insert(linesIndices.end(), { i_xYZ, i_xyZ });
	linesIndices.insert(linesIndices.end(), { i_xYZ, i_xYz });
	linesIndices.insert(linesIndices.end(), { i_xYZ, i_XYZ });
}