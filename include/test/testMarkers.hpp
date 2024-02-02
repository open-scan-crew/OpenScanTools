#ifndef _TEST_MARKERS_HPP_
#define _TEST_MARKERS_HPP_

#include "models/project/Marker.h"

#include <vector>
#include <random>

#include <glm/glm.hpp>

namespace scs
{
    std::vector<scs::Marker> generateRandomMarkers(int _scanCount, int _tagCount, glm::vec3 _minCoord, glm::vec3 _maxCoord);
}

std::vector<scs::Marker> scs::generateRandomMarkers(int _scanCount, int _tagCount, glm::vec3 _minCoord, glm::vec3 _maxCoord)
{
    std::vector<scs::Marker> markerOut;

    std::uniform_real_distribution<double> unif_x(_minCoord.x, _maxCoord.x);
    std::uniform_real_distribution<double> unif_y(_minCoord.y, _maxCoord.y);
    std::uniform_real_distribution<double> unif_z(_minCoord.z, _maxCoord.z);

    std::uniform_int_distribution<int> uni_r(64, 255);
    std::uniform_int_distribution<int> uni_g(64, 255);
    std::uniform_int_distribution<int> uni_b(64, 255);

    std::default_random_engine re;

    for (int i = 0; i < _scanCount; i++)
    {
        double x = unif_x(re);
        double y = unif_y(re);
        double z = unif_z(re);

        markerOut.push_back({ i, 17, {2, 156, 32, 0}, "scan", {x, y, z}, scs::Scan, false, false });
    }

    for (int i = 0; i < _tagCount; i++)
    {
        double x = unif_x(re);
        double y = unif_y(re);
        double z = unif_z(re);

        uint8_t r = uni_r(re);
        uint8_t g = uni_g(re);
        uint8_t b = uni_b(re);

        markerOut.push_back({ i + _scanCount, 6, {r, g, b, 0}, "un tag", {x, y, z}, scs::Tag, false, false });
    }

    return markerOut;
};

#endif