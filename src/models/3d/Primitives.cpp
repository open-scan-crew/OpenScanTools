#include "models/3d/Primitives.h"

#define _USE_MATH_DEFINES
#include <math.h>

bool scs::primitives::generateMarkerShape(std::vector<glm::vec2>& outVertices, std::vector<glm::vec2>& outUVs, MarkerShape shape)
{
    float dy;

    switch (shape)
    {
    case MarkerShape::Centered:
        dy = 0.f;
        break;
    case MarkerShape::Top_No_Arrow:
        dy = -0.5f;
        break;
    case MarkerShape::Top_Arrow:
        dy = -1.0f;
        break;
    default:
        return false;
    }

    // NOTE - all the 'y' coordinates are in screen space with the y-axis pointing downward
    // top-left corner
    outVertices.push_back({ -0.5f, -0.5f + dy });
    outUVs.push_back({ 0.f, 0.f });
    // top-right corner
    outVertices.push_back({ 0.5f, -0.5f + dy });
    outUVs.push_back({ 1.f, 0.f });
    // bottom-left corner
    outVertices.push_back({ -0.5f, 0.5f + dy });
    outUVs.push_back({ 0.f, 1.f });
    // bottom-right corner
    outVertices.push_back({ 0.5f, 0.5f + dy });
    outUVs.push_back({ 1.f, 1.f });
    // arrow
    if (shape == MarkerShape::Top_Arrow)
    {
        outVertices.push_back({ 0.f, 0.f });
        outUVs.push_back({ 0.5f, 1.5f });
    }

    return (true);
}

bool scs::primitives::generateScanMarker(std::vector<glm::vec2>& outVertices, std::vector<glm::vec2>& outUVs, const float& radius)
{
    // V_0
    outVertices.push_back({ 0.f, 0.f });
    // V_1
    outVertices.push_back({ (float)cos(M_PI / 6) * radius, -((float)sin(M_PI / 6) + 1.f) * radius });
    // V_2
    outVertices.push_back({ (float)cos(5 * M_PI / 6) * radius , -((float)sin(5 * M_PI / 6) + 1.f) * radius });
    // V_3
    outVertices.push_back({ 0.f, 0.f });

    return true;
}

bool scs::primitives::generateTagMarkerTextured(std::vector<glm::vec2>& outVertices, std::vector<glm::vec2>& outUVs, const int& sideCount, const float& scale, const float& base, const float& height)
{
	// Valid parameters
	if ((scale <= 0.f) || (height < 0))
		return false;
	// NOTE - all the 'y' coordinates are in screen space with the y-axis pointing downward
    // V_0
    outVertices.push_back({0.f, -(1.0f + height) * scale });
    outUVs.push_back({ 0.5f, 0.5f});
    double offset = M_PI / 4;
	bool baseConstructed = false;
	for (int iterator = 0; iterator <= sideCount; iterator++)
	{
		float x = (float)sin(offset+(iterator * 2 * M_PI / sideCount));
		float y = (float)cos(offset+(iterator * 2 * M_PI / sideCount));
	
		if (!baseConstructed && (x > 0.0f) && (0.0f > (float)sin(offset + ((iterator + 1) * 2 * M_PI / sideCount))))
		{
			baseConstructed = true;
			if(sideCount%2==0)
			{ 
				outVertices.push_back({ x * scale, -(y + 1.f + height) * scale });
				outUVs.push_back({ (x + 1.0f) / 2.0f, (y + 1.0f) / 2.0f });
			}
			outVertices.push_back({ 0.f, -base });
            //outUVs.push_back({ -1.0f,-1.0f });
			outUVs.push_back({ 0.5f, -(height - base / scale) / 2.f });
		}
		else
		{
			outVertices.push_back({ x * scale, -(y + 1.f + height) * scale });
			outUVs.push_back({ (x + 1.0f) / 2.0f, (y + 1.0f) / 2.0f });
		}
	}
	return true;
}

bool scs::primitives::generateSquareTextured(std::vector<glm::vec2>& outVertices, std::vector<glm::vec2>& outUVs, const float& size, const float& pointyH)
{
    // Valid parameters
    if ((size <= 0.f) || (pointyH < 0))
        return false;
    // NOTE - all the 'y' coordinates are in screen space with the y-axis pointing downward
    // top-left angle
    outVertices.push_back({ -size / 2.f, -(size + pointyH) });
    outUVs.push_back({ 0.f, 0.f });
    // top-right angle
    outVertices.push_back({ size / 2.f, -(size + pointyH) });
    outUVs.push_back({ 1.f, 0.f });
    // bottom-left angle
    outVertices.push_back({ -size / 2.f, -pointyH });
    outUVs.push_back({ 0.f, 1.f });
    // bottom-right angle
    outVertices.push_back({ size / 2.f, -pointyH });
    outUVs.push_back({ 1.f, 1.f });
    // pointe
    if (pointyH > 0.f)
    {
        outVertices.push_back({ 0.f, 0.f });
        outUVs.push_back({ 0.5f, 1.f + pointyH / size });
    }

    return (true);
}

bool scs::primitives::generateTagMarker(std::vector<glm::vec2>& outVertices, std::vector<glm::vec2>& outUVs, const int& sideCount, const float& scale, const float& base, const float& height, const bool& pointy)
{
    // Valid parameters
    if ((sideCount < 3) || (scale <= 0.f) || (base < 0.f) || (base > 1.f) || (height < 0))
        return false;

    // NOTE - all the 'y' coordinates are in screen space with the y-axis pointing downward
    // V_0
    outVertices.push_back({ 0.f, - (1.0 + height) * scale });

    // Special case
    if (base == 0 || height == 0)
    {
        for (int i = 0; i < sideCount; i++) {
            // V_i+1
            float x = (float)sin(i * 2 * M_PI / sideCount) ;
            float y = -((float)cos(i * 2 * M_PI / sideCount) + 1.f + height);
            outVertices.push_back({ x * scale, y  * scale});
        }
    }
    // Normal case
    else
    {
        bool baseConstructed = false;

        for (int i = 0; i < sideCount; i++) {
            // V_i+1
            float x = (float)sin(i * 2 * M_PI / sideCount);
            float y = (float)cos(i * 2 * M_PI / sideCount);

            if ((x < base && y < 0.f) && (baseConstructed == false))
            {
                // Compute the precise ordinate of the circle's point with abscissa (base / 2)
                float y = -cos(asin(base));

                // push the base
                outVertices.push_back({ base * scale, -(y + 1.f + height) * scale });
                if (pointy)
                {
                    outVertices.push_back({ 0.f, 0.f });
                }
                else
                {
                    outVertices.push_back({ base * scale, 0.f });
                    outVertices.push_back({ -base * scale, 0.f });
                }
                outVertices.push_back({ - base * scale, -(y + 1.f + height) * scale });
                baseConstructed = true;

                // increase i if we must skip some sides
                i = sideCount - i;
            }
            else
            {
                outVertices.push_back({ x * scale, -(y + 1.f + height) * scale});
            }
        }
    }

    // Last vertex

    outVertices.push_back({ 0.f, -(2.f + height) * scale });

    return true;
}

bool scs::primitives::generateDiamondMarker(std::vector<glm::vec2>& outVertices, std::vector<glm::vec2>& outUVs)
{
	for (int i = 0; i < 5; i++)
	{
		float x = (float)cos(M_PI / 2 * i);
		float y = (float)sin(M_PI / 2 * i);
		outVertices.push_back({ x, y });
		outUVs.push_back({ (1.0f+x)/2.0f, (1.0f+y)/2.0f });
	}
	return true;
}