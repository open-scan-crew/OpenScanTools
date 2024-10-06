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
    return glm::vec<3, T, glm::defaultp>((xMin + xMax) / 2, (yMin + yMax) / 2, (zMin + zMin) / 2);
}

#endif