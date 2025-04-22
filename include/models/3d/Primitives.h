#ifndef _PRIMITIVES_H_
#define _PRIMITIVES_H_

#include <vector>
#include "glm/glm.hpp"
#include "models/data/Marker.h"

namespace scs::primitives
{
	bool generateMarkerShape(std::vector<glm::vec2>& outVertices, std::vector<glm::vec2>& outUVs, const scs::MarkerShape& type);
    bool generateScanMarker(std::vector<glm::vec2>& outVertices, std::vector<glm::vec2>& outUVs, const float& radius=0.3f);
    bool generateTagMarker(std::vector<glm::vec2>& outVertices, std::vector<glm::vec2>& outUVs, const int& sideCount=15, const float& scale=0.2f, const float& base=0.3f, const float& height=1.f, const bool& pointy=true);
	bool generateTagMarkerTextured(std::vector<glm::vec2>& outVertices, std::vector<glm::vec2>& outUVs, const int& sideCount = 4, const float& scale = 1.f, const float& base = 0.2f, const float& height = 1.f);
    bool generateSquareTextured(std::vector<glm::vec2>& outVertices, std::vector<glm::vec2>& outUVs, const float& size, const float& pointyH);
	bool generateDiamondMarker(std::vector<glm::vec2>& outVertices, std::vector<glm::vec2>& outUVs);
}

#endif