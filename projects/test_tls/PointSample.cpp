#include "PointSample.h"

std::vector<tls::Point> PointSample::Predefined()
{
    return {
    { -5.0f, 9.0f, -4.1f, 42, 105, 210, 49},
    { -3.0f, -4.0f, 7.3f, 142, 103, 156, 165},
    { 9.0f, 10.0f, 6.8f, 242, 117, 114, 176},
    { 1.0f, 5.3f, 4.4f, 158, 87, 180, 148},
    { 3.0f, 6.0f, 1.2f, 250, 173, 255, 237},
    { 1.0f, -6.0f, 7.3f, 200, 202, 246, 71},
    { 9.0f, 3.0f, 9.9f, 132, 70, 140, 213},
    { 8.0f, 4.0f, 0.0f, 241, 188, 150, 187},
    };
}

void generateBrickFace(std::vector<tls::Point>& ret_pts, int du, int dv, float w, glm::vec3 size, glm::vec3 position, float dist)
{
    w += position[(3 - du - dv) % 3];
    for (float u = position[du] - size[du] / 2; u <= position[du] + size[du] / 2; u += dist)
    {
        for (float v = position[dv] - size[dv] / 2; v <= position[dv] + size[dv] / 2; v += dist)
        {
            tls::Point pt;
            pt.x = du == 0 ? u : dv == 0 ? v : w;
            pt.y = du == 1 ? u : dv == 1 ? v : w;
            pt.z = du == 2 ? u : dv == 2 ? v : w;
            pt.i = 255;
            ret_pts.push_back(pt);
        }
    }
}

std::vector<tls::Point> PointSample::Brick(glm::vec3 size, glm::vec3 position, float density)
{
    std::vector<tls::Point> pts;
    float dist = density != 0.f ? 1.f / density : 1.f;

    generateBrickFace(pts, 0, 1, size.z / 2.f, size, position, dist);
    generateBrickFace(pts, 0, 1, -size.z / 2.f, size, position, dist);
    generateBrickFace(pts, 1, 2, size.x / 2.f, size, position, dist);
    generateBrickFace(pts, 1, 2, -size.x / 2.f, size, position, dist);
    generateBrickFace(pts, 0, 2, size.y / 2.f, size, position, dist);
    generateBrickFace(pts, 2, 0, -size.y / 2.f, size, position, dist);

    return pts;
}

std::vector<tls::Point> PointSample::Spheroid(glm::vec3 size, glm::vec3 position, uint32_t density)
{
    std::vector<tls::Point> pts;
    // TODO
    return pts;
}