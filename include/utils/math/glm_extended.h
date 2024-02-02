#ifndef _GLM_EXTENDED_H_
#define _GLM_EXTENDED_H_

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

struct FrustumPlanes
{
    FrustumPlanes() noexcept
        : topSide(glm::dvec4())
        , bottomSide(glm::dvec4())
        , rightSide(glm::dvec4())
        , leftSide(glm::dvec4())
        , nearSide(glm::dvec4())
        , farSide(glm::dvec4())
    { }

    glm::dvec4 topSide;
    glm::dvec4 bottomSide;
    glm::dvec4 rightSide;
    glm::dvec4 leftSide;
    glm::dvec4 nearSide;
    glm::dvec4 farSide;

    inline friend FrustumPlanes operator*(const glm::dmat4& m, FrustumPlanes& f)
    {
        f.topSide = m * f.topSide;
        f.bottomSide = m * f.bottomSide;
        f.rightSide = m * f.rightSide;
        f.leftSide = m * f.leftSide;
        f.nearSide = m * f.nearSide;
        f.farSide = m * f.farSide;
        return (f);
    }

    inline friend FrustumPlanes operator*(const glm::dmat4& _m, const FrustumPlanes& _f)
    {
        FrustumPlanes f;
        f.topSide = _m * _f.topSide;
        f.bottomSide = _m * _f.bottomSide;
        f.rightSide = _m * _f.rightSide;
        f.leftSide = _m * _f.leftSide;
        f.nearSide = _m * _f.nearSide;
        f.farSide = _m * _f.farSide;
        return (f);
    }

};

namespace glm_extended
{
    // Angle between 2 glm::vec3 Objects
    float angleBetweenV3(const glm::vec3 a, const glm::vec3 b);

    // Angle between 2 glm::dvec3 Objects
    double angleBetweenDV3(const glm::dvec3 a, const glm::dvec3 b);

    // Projection Calculation of a onto b
    glm::vec3 projV3(const glm::vec3 a, const glm::vec3 b);

    // A test to see if P1 is on the same side as P2 of a line segment ab
    bool sameSide(glm::vec3 p1, glm::vec3 p2, glm::vec3 a, glm::vec3 b);

    // Generate a cross produect normal for a triangle
    glm::vec3 genTriNormal(glm::vec3 t1, glm::vec3 t2, glm::vec3 t3);

    // Check to see if a glm::vec3 Point is within a 3 glm::vec3 Triangle
    bool inTriangle(glm::vec3 point, glm::vec3 tri1, glm::vec3 tri2, glm::vec3 tri3);

	// Check if a point has a NaN coordinate
	bool isnan(const glm::vec3 point);

	bool isnan(const glm::dmat4 mat);

	// 2D Check if a segment intersect an axis aligned rectangle
	bool is2DSegmIntersectRectangle(glm::vec2 pos1, glm::vec2 pos2, float minX, float maxX, float minY, float maxY);
}
#endif