#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

#include <glm/glm.hpp>

#include <limits>

template<typename T>
struct TBoundingBox
{
    T xMin;
    T xMax;
    T yMin;
    T yMax;
    T zMin;
    T zMax;

    void setEmpty();
    void setInfinite();

    void extend(const TBoundingBox<T>& rhs);
    void intersect(const TBoundingBox<T>& rhs);

    [[nodiscard]] TBoundingBox<T> rotate(glm::mat<3, 3, T, glm::defaultp> rotation_matrix) const;
    [[nodiscard]] TBoundingBox<double> transform(glm::dmat4 transfo_matrix) const;

    bool isValid() const; // non empty, non infinite, not a NAN
    glm::vec<3, T, glm::defaultp> size() const;
    glm::vec<3, T, glm::defaultp> center() const;
};

typedef TBoundingBox<float> BoundingBox;
typedef TBoundingBox<double> BoundingBoxD;

template<typename T>
void TBoundingBox<T>::setEmpty()
{
    xMin = std::numeric_limits<T>::infinity();
    xMax = -std::numeric_limits<T>::infinity();
    yMin = std::numeric_limits<T>::infinity();
    yMax = -std::numeric_limits<T>::infinity();
    zMin = std::numeric_limits<T>::infinity();
    zMax = -std::numeric_limits<T>::infinity();
}

template<typename T>
void TBoundingBox<T>::setInfinite()
{
    xMin = -std::numeric_limits<T>::infinity();
    xMax = std::numeric_limits<T>::infinity();
    yMin = -std::numeric_limits<T>::infinity();
    yMax = std::numeric_limits<T>::infinity();
    zMin = -std::numeric_limits<T>::infinity();
    zMax = std::numeric_limits<T>::infinity();
}

template<typename T>
void TBoundingBox<T>::extend(const TBoundingBox<T>& rhs)
{
    xMin = rhs.xMin < xMin ? rhs.xMin : xMin;
    xMax = rhs.xMax > xMax ? rhs.xMax : xMax;
    yMin = rhs.yMin < yMin ? rhs.yMin : yMin;
    yMax = rhs.yMax > yMax ? rhs.yMax : yMax;
    zMin = rhs.zMin < zMin ? rhs.zMin : zMin;
    zMax = rhs.zMax > zMax ? rhs.zMax : zMax;
}

template<typename T>
void TBoundingBox<T>::intersect(const TBoundingBox<T>& rhs)
{
    xMin = rhs.xMin > xMin ? rhs.xMin : xMin;
    xMax = rhs.xMax < xMax ? rhs.xMax : xMax;
    if (xMin > xMax)
    {
        xMin = std::numeric_limits<T>::infinity();
        xMax = -std::numeric_limits<T>::infinity();
    }

    yMin = rhs.yMin > yMin ? rhs.yMin : yMin;
    yMax = rhs.yMax < yMax ? rhs.yMax : yMax;
    if (yMin > yMax)
    {
        yMin = std::numeric_limits<T>::infinity();
        yMax = -std::numeric_limits<T>::infinity();
    }

    zMin = rhs.zMin > zMin ? rhs.zMin : zMin;
    zMax = rhs.zMax < zMax ? rhs.zMax : zMax;
    if (zMin > zMax)
    {
        zMin = std::numeric_limits<T>::infinity();
        zMax = -std::numeric_limits<T>::infinity();
    }
}

template<typename T>
[[nodiscard]] TBoundingBox<T> TBoundingBox<T>::rotate(glm::mat<3, 3, T, glm::defaultp> rotation_matrix) const
{
    TBoundingBox<T> ret_bbox;
    ret_bbox.setEmpty();

    for (int i = 0; i < 8; ++i)
    {
        glm::vec<3, T, glm::defaultp> c;
        c.x = (i % 2 == 0) ? xMin : xMax;
        c.y = ((i >> 1) % 2 == 0) ? yMin : yMax;
        c.z = ((i >> 2) % 2 == 0) ? zMin : zMax;
        c = rotation_matrix * c;
        ret_bbox.xMin = std::min(ret_bbox.xMin, c.x);
        ret_bbox.xMax = std::max(ret_bbox.xMax, c.x);
        ret_bbox.yMin = std::min(ret_bbox.yMin, c.y);
        ret_bbox.yMax = std::max(ret_bbox.yMax, c.y);
        ret_bbox.zMin = std::min(ret_bbox.zMin, c.z);
        ret_bbox.zMax = std::max(ret_bbox.zMax, c.z);
    }

    return ret_bbox;
}

template<typename T>
[[nodiscard]] BoundingBoxD TBoundingBox<T>::transform(glm::dmat4 transfo_matrix) const
{
    BoundingBoxD ret_bbox;
    ret_bbox.setEmpty();

    for (int i = 0; i < 8; ++i)
    {
        glm::dvec4 c;
        c.x = (i % 2 == 0) ? xMin : xMax;
        c.y = ((i >> 1) % 2 == 0) ? yMin : yMax;
        c.z = ((i >> 2) % 2 == 0) ? zMin : zMax;
        c.w = 1.0;
        c = transfo_matrix * c;
        ret_bbox.xMin = std::min(ret_bbox.xMin, c.x);
        ret_bbox.xMax = std::max(ret_bbox.xMax, c.x);
        ret_bbox.yMin = std::min(ret_bbox.yMin, c.y);
        ret_bbox.yMax = std::max(ret_bbox.yMax, c.y);
        ret_bbox.zMin = std::min(ret_bbox.zMin, c.z);
        ret_bbox.zMax = std::max(ret_bbox.zMax, c.z);
    }

    return ret_bbox;
}



template<typename T>
bool TBoundingBox<T>::isValid() const
{
    bool min_inferior_max = (xMin <= xMax) && (yMin <= yMax) && (zMin <= zMax);
    bool is_nan = std::isnan(xMin) || std::isnan(xMax) || std::isnan(yMin) || std::isnan(yMax) || std::isnan(zMin) || std::isnan(zMax);
    bool finite_number = std::isfinite(xMin) && std::isfinite(xMax) && std::isfinite(yMin) && std::isfinite(yMax) && std::isfinite(zMin) && std::isfinite(zMax);

    return min_inferior_max && !is_nan && finite_number;
}

template<typename T>
glm::vec<3, T, glm::defaultp> TBoundingBox<T>::size() const
{
    return glm::vec<3, T, glm::defaultp>((xMax - xMin), (yMax - yMin), (zMax - zMin));
}

template<typename T>
glm::vec<3, T, glm::defaultp> TBoundingBox<T>::center() const
{
    return glm::vec<3, T, glm::defaultp>((xMin + xMax) / 2, (yMin + yMax) / 2, (zMin + zMax) / 2);
}

#endif