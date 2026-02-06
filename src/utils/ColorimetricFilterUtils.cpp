#include "utils/ColorimetricFilterUtils.h"

#include <algorithm>
#include <cmath>

namespace
{
    constexpr float kEpsilon = 1e-5f;

    struct ProjectedPoint
    {
        int index = 0;
        glm::vec2 pos = glm::vec2(0.0f);
        float angle = 0.0f;
    };

    glm::vec2 projectToPlane(const glm::vec3& point, const glm::vec3& origin, const glm::vec3& axisU, const glm::vec3& axisV)
    {
        glm::vec3 rel = point - origin;
        return glm::vec2(glm::dot(rel, axisU), glm::dot(rel, axisV));
    }

    std::array<int, 2> selectProjectionAxes(const std::array<glm::vec3, 4>& points)
    {
        float minVals[3] = { points[0].x, points[0].y, points[0].z };
        float maxVals[3] = { points[0].x, points[0].y, points[0].z };

        for (const glm::vec3& point : points)
        {
            minVals[0] = std::min(minVals[0], point.x);
            minVals[1] = std::min(minVals[1], point.y);
            minVals[2] = std::min(minVals[2], point.z);
            maxVals[0] = std::max(maxVals[0], point.x);
            maxVals[1] = std::max(maxVals[1], point.y);
            maxVals[2] = std::max(maxVals[2], point.z);
        }

        float ranges[3] = { maxVals[0] - minVals[0], maxVals[1] - minVals[1], maxVals[2] - minVals[2] };
        int firstAxis = 0;
        int secondAxis = 1;

        for (int axis = 0; axis < 3; ++axis)
        {
            if (ranges[axis] > ranges[firstAxis])
            {
                secondAxis = firstAxis;
                firstAxis = axis;
            }
            else if (axis != firstAxis && ranges[axis] > ranges[secondAxis])
            {
                secondAxis = axis;
            }
        }

        return { firstAxis, secondAxis };
    }
}

namespace ColorimetricFilterUtils
{
    glm::vec3 toVec3(const Color32& color)
    {
        return glm::vec3(color.r, color.g, color.b);
    }

    std::array<Color32, 4> reorderQuadColors(const std::array<Color32, 4>& colors)
    {
        std::array<glm::vec3, 4> points = { toVec3(colors[0]), toVec3(colors[1]), toVec3(colors[2]), toVec3(colors[3]) };
        glm::vec3 centroid = (points[0] + points[1] + points[2] + points[3]) / 4.0f;

        glm::vec3 normal = glm::cross(points[1] - points[0], points[2] - points[0]);
        if (glm::length(normal) < kEpsilon)
            normal = glm::cross(points[1] - points[0], points[3] - points[0]);

        std::array<ProjectedPoint, 4> projected = {};

        if (glm::length(normal) >= kEpsilon)
        {
            glm::vec3 axisU = glm::normalize(points[1] - points[0]);
            if (glm::length(axisU) < kEpsilon)
                axisU = glm::normalize(points[2] - points[0]);
            if (glm::length(axisU) < kEpsilon)
                axisU = glm::vec3(1.0f, 0.0f, 0.0f);
            glm::vec3 axisV = glm::normalize(glm::cross(normal, axisU));

            for (int index = 0; index < 4; ++index)
            {
                glm::vec2 pos = projectToPlane(points[index], centroid, axisU, axisV);
                projected[index] = { index, pos, std::atan2(pos.y, pos.x) };
            }
        }
        else
        {
            std::array<int, 2> axes = selectProjectionAxes(points);
            for (int index = 0; index < 4; ++index)
            {
                glm::vec2 pos(0.0f);
                pos.x = points[index][axes[0]] - centroid[axes[0]];
                pos.y = points[index][axes[1]] - centroid[axes[1]];
                projected[index] = { index, pos, std::atan2(pos.y, pos.x) };
            }
        }

        std::array<int, 4> order = { 0, 1, 2, 3 };
        std::sort(order.begin(), order.end(), [&projected](int lhs, int rhs)
        {
            return projected[lhs].angle < projected[rhs].angle;
        });

        std::array<Color32, 4> ordered = colors;
        for (int index = 0; index < 4; ++index)
        {
            ordered[index] = colors[projected[order[index]].index];
        }

        return ordered;
    }

    OrderedColorSet buildOrderedColorSet(const ColorimetricFilterSettings& settings)
    {
        OrderedColorSet result;
        int count = 0;
        for (int index = 0; index < 4; ++index)
        {
            if (settings.colorsEnabled[index])
            {
                result.colors[count] = settings.colors[index];
                ++count;
            }
        }
        result.count = count;

        if (result.count == 4)
            result.colors = reorderQuadColors(result.colors);

        return result;
    }
}
