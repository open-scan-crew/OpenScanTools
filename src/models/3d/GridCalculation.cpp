#include "models/3d/GridCalculation.h"
#include "models/3d/MeshBuffer.h"

#include "vulkan/MeshManager.h"

#include <glm/gtx/quaternion.hpp>

GridCalculation::GridCalculation()
{}

GridCalculation::~GridCalculation()
{}

bool GridCalculation::calculateBoxes(std::vector<GridBox>& ret_grid, const BoxNode& box)
{
    switch (box.getGridType())
    {
    case GridType::ByStep:
        return calculateBoxesByStep(ret_grid, (TransformationModule)box, box.getGridDivision());
    case GridType::ByMultiple:
        return calculateBoxesByMultiple(ret_grid, (TransformationModule)box, box.getGridDivision());
    default:
        return false;
    }
}

bool GridCalculation::calculateBoxesByStep(std::vector<GridBox>& grid, const TransformationModule& transfo, const glm::vec3& step)
{
    grid.clear();
    const glm::dvec3& scale = transfo.getScale();
    const glm::dvec3& center = transfo.getCenter();
    glm::dvec3 step2(step / 2.0f);

    glm::dvec3 maxIteration(glm::floor(scale / step2));
    glm::dvec3 lambda3(scale - maxIteration * step2);
    glm::ivec3 gridSize(maxIteration);

    if (maxIteration.x * maxIteration.y * maxIteration.z > MAX_BOXES)
        return false;

    GridBox gbox(glm::vec3(0.f), transfo.getOrientation(), step2);
    glm::dmat4 rotation(glm::toMat4(transfo.getOrientation()));
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

bool GridCalculation::calculateBoxesByMultiple(std::vector<GridBox>& grid, const TransformationModule& transfo, const glm::ivec3& division)
{
    grid.clear();

    const glm::dvec3& size = transfo.getScale();
    const glm::dvec3& center = transfo.getCenter();

    float sizeX((float)size.x / division.x);
    float sizeY((float)size.y / division.y);
    float sizeZ((float)size.z / division.z);

    if (division.x * division.y * division.z > MAX_BOXES)
        return false;

    GridBox gbox(glm::vec3(0.f), transfo.getOrientation(), glm::vec3(sizeX, sizeY, sizeZ));
    glm::dmat4 rotation(glm::toMat4(transfo.getOrientation()));
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
    if (step.x <= 0.f || step.y <= 0.f || step.z <= 0.f)
        return false;

    glm::vec3 size = box.getScale();

    std::vector<glm::vec3> linesPositions;

    uint32_t max_i_x = (uint32_t)ceil(size.x * 2.f / step.x) + 1;
    uint32_t max_i_y = (uint32_t)ceil(size.y * 2.f / step.y) + 1;
    uint32_t max_i_z = (uint32_t)ceil(size.z * 2.f / step.z) + 1;

    if (max_i_x * max_i_y * max_i_z > MAX_BOXES)
        return false;

    // lines orthogonal to XY faces
    // normali_ze the steps in each dimension for a box of size (2, 2, 2)
    glm::vec3 step_n = step / glm::vec3(size);

    for (uint32_t i_x = 0; i_x < max_i_x; i_x++)
    {
        float tx = std::min(i_x * step.x / size.x - 1.f, 1.f);
        for (uint32_t i_y = 0; i_y < max_i_y; i_y++)
        {
            float ty = std::min(i_y * step.y / size.y - 1.f, 1.f);
            linesPositions.push_back(glm::vec3(tx, ty, -1.f));
            linesPositions.push_back(glm::vec3(tx, ty, 1.f));
        }
    }

    for (uint32_t i_y = 0; i_y < max_i_y; i_y++)
    {
        float ty = std::min(i_y * step.y / size.y - 1.f, 1.f);
        for (uint32_t i_z = 0; i_z < max_i_z; i_z++)
        {
            float tz = std::min(i_z * step.z / size.z - 1.f, 1.f);
            linesPositions.push_back(glm::vec3(-1.f, ty, tz));
            linesPositions.push_back(glm::vec3(1.f, ty, tz));
        }
    }

    for (uint32_t i_x = 0; i_x < max_i_x; i_x++)
    {
        float tx = std::min(i_x * step.x / size.x - 1.f, 1.f);
        for (uint32_t i_z = 0; i_z < max_i_z; i_z++)
        {
            float tz = std::min(i_z * step.z / size.z - 1.f, 1.f);
            linesPositions.push_back(glm::vec3(tx, -1.f, tz));
            linesPositions.push_back(glm::vec3(tx, 1.f, tz));
        }
    }

    meshBuf.addDraw(VK_PRIMITIVE_TOPOLOGY_LINE_LIST, { 0, (uint32_t)linesPositions.size() });

    VkResult res(MeshManager::getInstance().uploadInMeshBuffer(meshBuf, linesPositions, {}, {}));

    return (res == VkResult::VK_SUCCESS);
}

bool GridCalculation::allocGridMeshByMultiple(MeshBuffer& meshBuf, const TransformationModule& box, const glm::ivec3& division)
{
    if (division.x == 0 || division.y == 0 || division.z == 0)
        return false;

    glm::vec3 size = box.getScale();
    glm::vec3 step(2.f / division.x, 2.f / division.y, 2.f / division.z);

    if (division.x * division.y * division.z > MAX_BOXES)
        return false;

    std::vector<glm::vec3> linesPositions;

    for (int i_x = 0; i_x < division.x + 1; i_x++)
    {
        float tx = i_x * step.x - 1.f;
        for (int i_y = 0; i_y < division.y + 1; i_y++)
        {
            float ty = i_y * step.y - 1.f;
            linesPositions.push_back(glm::vec3(tx, ty, -1.f));
            linesPositions.push_back(glm::vec3(tx, ty, 1.f));
        }
    }

    for (int i_y = 0; i_y < division.y + 1; i_y++)
    {
        float ty = i_y * step.y - 1.f;
        for (int i_z = 0; i_z < division.z + 1; i_z++)
        {
            float tz = i_z * step.z - 1.f;
            linesPositions.push_back(glm::vec3(-1.f, ty, tz));
            linesPositions.push_back(glm::vec3(1.f, ty, tz));
        }
    }

    for (int i_x = 0; i_x < division.x + 1; i_x++)
    {
        float tx = i_x * step.x - 1.f;
        for (int i_z = 0; i_z < division.z + 1; i_z++)
        {
            float tz = i_z * step.z - 1.f;
            linesPositions.push_back(glm::vec3(tx, -1.f, tz));
            linesPositions.push_back(glm::vec3(tx, 1.f, tz));
        }
    }

    meshBuf.addDraw(VK_PRIMITIVE_TOPOLOGY_LINE_LIST, { 0, (uint32_t)linesPositions.size() });

    VkResult res(MeshManager::getInstance().uploadInMeshBuffer(meshBuf, linesPositions, {}, {}));
    assert(res == VkResult::VK_SUCCESS);

    return res == VkResult::VK_SUCCESS;
}
