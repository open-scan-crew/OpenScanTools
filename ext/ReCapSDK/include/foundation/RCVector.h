//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Copyright 2018 by Autodesk, Inc.
//
// The information contained herein is confidential, proprietary to Autodesk,
// Inc., and considered a trade secret as defined in section 499C of the
// penal code of the State of California.  Use of this information by anyone
// other than authorized employees of Autodesk, Inc. is granted only under a
// written non-disclosure agreement, expressly prescribing the scope and
// manner of such use.
//
//        AUTODESK PROVIDES THIS PROGRAM "AS IS" AND WITH ALL FAULTS.
//        AUTODESK SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF
//        MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  AUTODESK, INC.
//        DOES NOT WARRANT THAT THE OPERATION OF THE PROGRAM WILL BE
//        UNINTERRUPTED OR ERROR FREE.
//
//////////////////////////////////////////////////////////////////////////////
/**
@file RCVector.h
*/
#pragma once

#include <foundation/RCCommonDef.h>
#include <foundation/RCMath.h>

/**
@namespace Autodesk

We generally put all the Autodesk related API stuff into this namespace. This is
the RCVector.h variant

@namespace RealityComputing

We generally put all the RS related API stuff into this namespace. This is the
RCVector.h variant

@namespace Foundation

We generally put all the Foundation API stuff into this namespace. This
is the RCVector.h variant
*/
namespace Autodesk { namespace RealityComputing { namespace Foundation {
    /// \brief Two-dimensional vector template.
    /**
    @tparam T
    The value type of the RCVector
    */
    template <class T>
    struct RC_COMMON_API RCVector2T
    {
        T x;
        T y;

        RCVector2T();
        RCVector2T(T x, T y);
        RCVector2T(const T (&v)[2]);

        operator T*();
        operator const T*() const;

        RCVector2T operator+(const RCVector2T& v) const;
        RCVector2T operator-(const RCVector2T& v) const;
        RCVector2T operator*(T s) const;
        RCVector2T operator/(T s) const;
        T operator*(const RCVector2T& v) const;

        RCVector2T& operator+=(const RCVector2T& v);
        RCVector2T& operator-=(const RCVector2T& v);
        RCVector2T& operator*=(T s);
        RCVector2T& operator/=(T s);
        const RCVector2T operator-() const;    // negate or invert

        bool operator==(const RCVector2T& v) const;
        bool equals(const RCVector2T& v, T eps) const;

        void zero();
        double length() const;
        double lengthSqrd() const;
        double dot(const RCVector2T<T>& v) const;
        double distance(const RCVector2T& v) const;
        double distanceSqrd(const RCVector2T& v) const;

        RC_COMMON_API_INLINE T cross(const RCVector2T& v) const
        {
            return (x * v.y - y * v.x);
        }

        RCVector2T& normalize();
        RCVector2T getNormalized() const;
        bool isNormalized() const;
        void lerp(const RCVector2T<T>& v1, const RCVector2T<T>& v2, T t);

        template <typename VT>
        RC_COMMON_API_INLINE void convertFrom(const VT& vec)
        {
            x = static_cast<T>(vec.x);
            y = static_cast<T>(vec.y);
        }

        template <typename VecType>
        RC_COMMON_API_INLINE RCVector2T<VecType> convertTo() const
        {
            RCVector2T<VecType> vec;
            vec.convertFrom(*this);
            return vec;
        }

        RC_COMMON_API_INLINE T operator[](int i) const
        {
            return (&x)[i];
        }
        RC_COMMON_API_INLINE T& operator[](int i)
        {
            return (&x)[i];
        }
    };

    /// \brief Three-dimensional vector template.
    ///
    template <class T>
    struct RC_COMMON_API RCVector3T
    {
        T x;
        T y;
        T z;

        RC_COMMON_API_INLINE RCVector3T() : x(0), y(0), z(0)
        {
        }
        RC_COMMON_API_INLINE RCVector3T(T xx, T yy, T zz) : x(xx), y(yy), z(zz)
        {
        }
        RC_COMMON_API_INLINE RCVector3T(const RCVector3T& v) : x(v.x), y(v.y), z(v.z)
        {
        }
        RC_COMMON_API_INLINE RCVector3T(RCVector3T&& v) : x(v.x), y(v.y), z(v.z)
        {
        }
        RC_COMMON_API_INLINE RCVector3T(const T (&v)[3]) : x(v[0]), y(v[1]), z(v[2])
        {
        }

        RC_COMMON_API_INLINE RCVector3T operator=(const RCVector3T& v)
        {
            x = v.x;
            y = v.y;
            z = v.z;
            return *this;
        }
        RC_COMMON_API_INLINE RCVector3T operator=(RCVector3T&& v)
        {
            x = v.x;
            y = v.y;
            z = v.z;
            return *this;
        }

        RC_COMMON_API_INLINE RCVector3T operator+(const RCVector3T& v) const
        {
            return RCVector3T<T>(*this) += v;
        }
        RC_COMMON_API_INLINE RCVector3T operator-(const RCVector3T& v) const
        {
            return RCVector3T<T>(*this) -= v;
        }
        RC_COMMON_API_INLINE RCVector3T operator*(T s) const
        {
            return RCVector3T<T>(*this) *= s;
        }
        RC_COMMON_API_INLINE RCVector3T operator/(T s) const
        {
            return RCVector3T<T>(*this) /= s;
        }

        RC_COMMON_API_INLINE RCVector3T& operator+=(const RCVector3T& v)
        {
            x += v.x;
            y += v.y;
            z += v.z;
            return *this;
        }
        RC_COMMON_API_INLINE RCVector3T& operator-=(const RCVector3T& v)
        {
            x -= v.x;
            y -= v.y;
            z -= v.z;
            return *this;
        }
        RC_COMMON_API_INLINE RCVector3T& operator*=(T s)
        {
            x *= s;
            y *= s;
            z *= s;
            return *this;
        }
        RC_COMMON_API_INLINE RCVector3T& operator/=(T s)
        {
            x /= s;
            y /= s;
            z /= s;
            return *this;
        }
        RC_COMMON_API_INLINE const RCVector3T operator-() const
        {
            return RCVector3T<T>(-x, -y, -z);
        }

        RC_COMMON_API_INLINE bool operator==(const RCVector3T& v) const
        {
            return equals(v, static_cast<T>(Constants::EPSILON));
        }
        RC_COMMON_API_INLINE bool equals(const RCVector3T& v, T eps) const
        {
            return (RCMath::equals(x, v.x, eps) && RCMath::equals(y, v.y, eps) && RCMath::equals(z, v.z, eps));
        }

        RC_COMMON_API_INLINE bool operator<(const RCVector3T& v) const
        {
            if (x < v.x)
                return true;
            else if (x > v.x)
                return false;

            if (y < v.y)
                return true;
            else if (y > v.y)
                return false;

            if (z < v.z)
                return true;
            else
                return false;
        }
        RC_COMMON_API_INLINE bool operator>(const RCVector3T& v) const
        {
            if (x > v.x)
                return true;
            else if (x < v.x)
                return false;

            if (y > v.y)
                return true;
            else if (y < v.y)
                return false;

            if (z > v.z)
                return true;
            else
                return false;
        }

        RC_COMMON_API_INLINE bool operator>=(const RCVector3T& v) const
        {
            return !(*this < v);
        }
        RC_COMMON_API_INLINE bool operator<=(const RCVector3T& v) const
        {
            return !(*this > v);
        }

        RC_COMMON_API_INLINE void zero()
        {
            *this = RCVector3T();
        }

        RC_COMMON_API_INLINE double length() const
        {
            return sqrt(lengthSqrd());
        }

        RC_COMMON_API_INLINE double lengthSqrd() const
        {
            return static_cast<double>(x) * x + static_cast<double>(y) * y + static_cast<double>(z) * z;
        }

        RC_COMMON_API_INLINE double dot(const RCVector3T& v) const
        {
            return static_cast<double>(x) * v.x + static_cast<double>(y) * v.y + static_cast<double>(z) * v.z;
        }

        RC_COMMON_API_INLINE RCVector3T cross(const RCVector3T& v) const
        {
            return RCVector3T(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
        }

        RC_COMMON_API_INLINE double distance(const RCVector3T& v) const
        {
            return sqrt(distanceSqrd(v));
        }

        RC_COMMON_API_INLINE double distanceSqrd(const RCVector3T& v) const
        {
            return (x - v.x) * (x - v.x) + (y - v.y) * (y - v.y) + (z - v.z) * (z - v.z);
        }

        RC_COMMON_API_INLINE RCVector3T& normalize()
        {
            double len = this->length();
            if (len > 0)
                *this /= static_cast<T>(len);
            else
                zero();

            return *this;
        }

        RC_COMMON_API_INLINE RCVector3T getNormalized() const
        {
            return RCVector3T<T>(*this).normalize();
        }

        RC_COMMON_API_INLINE bool isNormalized() const
        {
            return std::abs(this->length() - 1.0) < Constants::DISTANCE_TOLERANCE;
        }
        RC_COMMON_API_INLINE void lerp(const RCVector3T& v1, const RCVector3T& v2, const T t)
        {
            if (t <= 0)
            {
                (*this) = v1;
            }
            else if (t >= T(1.0))
            {
                (*this) = v2;
            }
            else
            {
                (*this) = v1 + (v2 - v1) * t;
            }
        }

        template <typename VecType>
        RC_COMMON_API_INLINE void convertFrom(const VecType& vec)
        {
            x = static_cast<T>(vec.x);
            y = static_cast<T>(vec.y);
            z = static_cast<T>(vec.z);
        }

        template <typename VecType>
        RC_COMMON_API_INLINE RCVector3T<VecType> convertTo() const
        {
            RCVector3T<VecType> vec;
            vec.convertFrom(*this);
            return vec;
        }

        RC_COMMON_API_INLINE T operator[](int i) const
        {
            return (&x)[i];
        }
        RC_COMMON_API_INLINE T& operator[](int i)
        {
            return (&x)[i];
        }

        RC_COMMON_API_INLINE RCVector2T<T> xy() const
        {
            return RCVector2T<T>(x, y);
        }
        RC_COMMON_API_INLINE RCVector2T<T> yz() const
        {
            return RCVector2T<T>(y, z);
        }
        RC_COMMON_API_INLINE RCVector2T<T> xz() const
        {
            return RCVector2T<T>(x, z);
        }

        ///
        /// ALL methods after this line are for alVector porting, and
        /// will be removed once all the code is ported
        ///
        RC_COMMON_API_INLINE const T* toDoublePointer() const
        {
            return reinterpret_cast<const T*>(&x);
        }
    };

    /// \brief Four-dimensional vector template.
    ///
    template <class T>
    struct RC_COMMON_API RCVector4T
    {
        T x;
        T y;
        T z;
        T w;

        RCVector4T();
        RCVector4T(T xx, T yy, T zz, T ww);
        RCVector4T(const RCVector3T<T>& v, T ww = 0);
        RCVector4T(const T (&v)[4]);
        RCVector4T(const T (&v)[3]);

        inline operator T*();
        inline operator const T*() const;

        RCVector4T operator+(const RCVector4T& v) const;
        RCVector4T operator-(const RCVector4T& v) const;
        RCVector4T operator*(T s) const;
        RCVector4T operator/(T s) const;
        T operator*(const RCVector4T& v) const;

        RCVector4T& operator+=(const RCVector4T& v);
        RCVector4T& operator-=(const RCVector4T& v);
        RCVector4T& operator*=(T s);
        RCVector4T& operator/=(T s);
        bool operator==(const RCVector4T& v) const;
        bool operator!=(const RCVector4T& v) const;

        void zero();
        double length() const;
        double lengthSqrd() const;
        double dot(const RCVector4T& v) const;
        RCVector4T& normalize();
        RCVector4T getNormalized() const;
        bool isNormalized() const;
        RCVector3T<T> truncate() const;
        void lerp(const RCVector4T& v1, const RCVector4T& v2, const T t);

        template <typename VecType>
        RC_COMMON_API_INLINE void convertFrom(const VecType& vec)
        {
            x = static_cast<T>(vec.x);
            y = static_cast<T>(vec.y);
            z = static_cast<T>(vec.z);
            w = static_cast<T>(vec.w);
        }

        template <typename VecType>
        RC_COMMON_API_INLINE RCVector4T<VecType> convertTo() const
        {
            RCVector4T<VecType> vec;
            vec.convertFrom(*this);
            return vec;
        }

        RC_COMMON_API_INLINE T operator[](int i) const
        {
            return (&x)[i];
        }
        RC_COMMON_API_INLINE T& operator[](int i)
        {
            return (&x)[i];
        }
    };

    typedef RCVector2T<float> RCVector2f;

    /**
    \typedef RCVector2d
    This is the 2-tuple double-precision floating point type that corresponds
    to Vec2's of other linear algebra libraries.
    */
    typedef RCVector2T<double> RCVector2d;

    typedef RCVector3T<float> RCVector3f;
    typedef RCVector3T<double> RCVector3d;
    typedef RCVector3T<unsigned char> RCVector3ub;
    typedef RCVector4T<int> RCVector4i;
    typedef RCVector4T<float> RCVector4f;
    typedef RCVector4T<double> RCVector4d;
    typedef RCVector4T<unsigned char> RCVector4ub;

    const RCVector4f RC_COMMON_API operator-(const RCVector4f& v);
    const RCVector4d RC_COMMON_API operator-(const RCVector4d& v);

    template <typename T>
    RC_COMMON_API_INLINE RCVector2T<T> operator*(double d, const RCVector2T<T>& v)
    {
        return v * d;
    }

    template <typename T>
    RC_COMMON_API_INLINE RCVector3T<T> operator*(double d, const RCVector3T<T>& v)
    {
        return v * d;
    }

    template <typename T>
    RC_COMMON_API_INLINE RCVector4T<T> operator*(double d, const RCVector4T<T>& v)
    {
        return v * d;
    }

}}}    // namespace Autodesk::RealityComputing::Foundation
