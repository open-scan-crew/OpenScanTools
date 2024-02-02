#include "utils/math/glm_extended.h"

namespace glm_extended
{
    // Angle between 2 glm::vec3 Objects
    float angleBetweenV3(const glm::vec3 a, const glm::vec3 b)
    {
        float angle = glm::dot(a, b);
        angle /= (glm::length(a) * glm::length(b));
		angle = angle > 1.0f ? 1.0f : (angle < -1.0f ? -1.0f : angle);
        return angle = acosf(angle);
    }

    // Angle between 2 glm::dvec3 Objects
    double angleBetweenDV3(const glm::dvec3 a, const glm::dvec3 b)
    {
        double angle = glm::dot(a, b);
        angle /= (glm::length(a) * glm::length(b));
        angle = angle > 1.0 ? 1.0 : (angle < -1.0 ? -1.0 : angle);
        return acos(angle);
    }

    // Projection Calculation of a onto b
    glm::vec3 projV3(const glm::vec3 a, const glm::vec3 b)
    {
        glm::vec3 bn = b / glm::length2(b);
        return bn * glm::dot(a, bn);
    }

    // A test to see if P1 is on the same side as P2 of a line segment ab
    bool sameSide(glm::vec3 p1, glm::vec3 p2, glm::vec3 a, glm::vec3 b)
    {
        glm::vec3 cp1 = glm::cross(b - a, p1 - a);
        glm::vec3 cp2 = glm::cross(b - a, p2 - a);

        if (glm::dot(cp1, cp2) >= 0)
            return true;
        else
            return false;
    }

    // Generate a cross produect normal for a triangle
    glm::vec3 genTriNormal(glm::vec3 t1, glm::vec3 t2, glm::vec3 t3)
    {
        glm::vec3 u = t2 - t1;
        glm::vec3 v = t3 - t1;

        glm::vec3 normal = glm::cross(u, v);

        return normal;
    }

    // Check to see if a glm::vec3 Point is within a 3 glm::vec3 Triangle
    bool inTriangle(glm::vec3 point, glm::vec3 tri1, glm::vec3 tri2, glm::vec3 tri3)
    {
        // Test to see if it is within an infinite prism that the triangle outlines.
        bool within_tri_prisim = sameSide(point, tri1, tri2, tri3) && sameSide(point, tri2, tri1, tri3)
            && sameSide(point, tri3, tri1, tri2);

        // If it isn't it will never be on the triangle
        if (!within_tri_prisim)
            return false;

        // Calulate Triangle's Normal
        glm::vec3 n = genTriNormal(tri1, tri2, tri3);

        // Project the point onto this normal
        glm::vec3 proj = projV3(point, n);

        // If the distance from the triangle to the point is 0
        //	it lies on the triangle
        if (glm::length2(proj) == 0)
            return true;
        else
            return false;
    }

	// Check if a point has a NaN coordinate
	bool isnan(const glm::vec3 point) {
		return (std::isnan(point.x) || std::isnan(point.y) || std::isnan(point.z));
	}

    bool isnan(const glm::dmat4 mat)
    {
        bool isnan = false;
        isnan |= glm::any(glm::isnan(mat[0]));
        isnan |= glm::any(glm::isnan(mat[1]));
        isnan |= glm::any(glm::isnan(mat[2]));
        isnan |= glm::any(glm::isnan(mat[3]));

        return isnan;
    }

	// 2D Check if a segment intersect an axis aligned rectangle
	bool is2DSegmIntersectRectangle(glm::vec2 segmPos1, glm::vec2 segmPos2, float rectMinX, float rectMaxX, float rectMinY, float rectMaxY)
	{
		float measureMinX, measureMinY, measureMaxX, measureMaxY;

		if (segmPos1.x > segmPos2.x)
			measureMaxX = segmPos1.x, measureMinX = segmPos2.x;
		else
			measureMaxX = segmPos2.x, measureMinX = segmPos1.x;

		if (segmPos1.y > segmPos2.y)
			measureMaxY = segmPos1.y, measureMinY = segmPos2.y;
		else
			measureMaxY = segmPos2.y, measureMinY = segmPos1.y;

		// if the measureRectangle intersect the hoverRectangle
		if (!(rectMaxX < measureMinX || measureMaxX < rectMinX || rectMaxY < measureMinY || measureMaxY < rectMinY))
		{
			glm::dvec2 diag = { segmPos2.x - segmPos1.x, segmPos2.y - segmPos1.y };
			std::vector<glm::dvec2> corners = { {rectMinX - segmPos1.x, rectMaxY - segmPos1.y}, {rectMaxX - segmPos1.x, rectMinY - segmPos1.y}, {rectMaxX - segmPos1.x, rectMaxY - segmPos1.y} };
			bool sens = (diag.x * (rectMinY - segmPos1.y) - (rectMinX - segmPos1.x) * diag.y) > 0;
			// if one corner is in an other side of the diag (compared to others corners)
			for (auto corner = corners.begin(); corner != corners.end(); corner++) {
				if (((diag.x * corner->y - corner->x * diag.y) > 0) != sens) 
					return true;
			}
		}
		return false;
	}
}