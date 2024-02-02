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

#pragma once

#include <foundation/RCCommonDef.h>
#include <foundation/RCMath.h>

namespace Autodesk { namespace RealityComputing { namespace Foundation {

    const static double TWO_PI = 2.0 * Constants::MATH_PI;

    /// \brief Class for angle operation
    class RC_COMMON_API RCAngle
    {
    public:
        /// \brief Default constructor
        RCAngle() = default;

        /// \brief Copy constructor
        RCAngle(const RCAngle& angle) noexcept;

        /// \brief Default destructor
        ~RCAngle(){};

        /// \brief Get the radian value of the angle
        /// \return The radian value of the angle
        double getRadians() const noexcept;

        /// \brief Set the radian value of the angle
        /// \param radians The radian value of the angle to set
        void setRadians(double radians) noexcept;

        ///\brief Get degree value of the angle
        ///\return Degree of the angle
        double getDegrees() const noexcept;

        /// \brief Override operator =
        /// \other The other \b RCAngle to copy from
        /// \return This \b RCAngle
        RCAngle& operator=(const RCAngle& other) noexcept;

        /// \brief Add the given angle \p v to this angle
        /// \param v The input angle to be added
        /// \return The result of the addition of the angles
        RCAngle operator+(const RCAngle& v) const noexcept;

        /// \brief Subtract the given angle \p v from this matrix
        /// \param v The input angle to be subtracted
        /// \return The result of the subtraction of the angles
        RCAngle operator-(const RCAngle& v) const noexcept;

        /// \brief Add the given angle \p m to this angle
        /// \param v The input angle to be added
        /// \return The result of the addition of the angle
        RCAngle& operator+=(const RCAngle& v) noexcept;

        /// \brief Subtract the given angle \p m from this matrix
        /// \param v The input angle to be subtracted
        /// \return The result of the subtraction of the angles
        RCAngle& operator-=(const RCAngle& v) noexcept;

        /// \brief Multiply this RCAngle by \p val and return the result
        /// \param val The value to multiply
        /// \return The result of multiplication
        RCAngle multiplyBy(double val) const noexcept;

        /// \brief Multiply this RCAngle with \p val, round to
        ///        the nearest integer and return the result
        /// \param val The value to multiply
        /// \return The rounded result from multiplication
        RCAngle multiplyByAndRound(double val) const noexcept;

    public:
        /// \brief Create angle from a radian value
        /// \param radians The Radian value for the angle
        /// \return \b RCAngle constructed from the input \p radians
        static RCAngle createFromRadians(const double radians) noexcept;

        /// \brief Create angle from a degree value
        /// \param degrees The degree value for the angle
        /// \return \b RCAngle constructed from the input \p degrees
        static RCAngle createFromDegrees(const double degrees) noexcept;

    private:
        RCAngle(double radians) noexcept;

        void sanitize() noexcept;

    private:
        double mRadians = 0.0;
    };

    inline RCAngle::RCAngle(const RCAngle& angle) noexcept : mRadians(angle.getRadians())
    {
    }

    inline RCAngle::RCAngle(double radians) noexcept : mRadians(radians)
    {
        sanitize();
    }

    inline RCAngle RCAngle::createFromRadians(const double radians) noexcept
    {
        return RCAngle(radians);
    }

    inline RCAngle RCAngle::createFromDegrees(const double degrees) noexcept
    {
        return RCAngle(RCMath::toRadians(degrees));
    }

    inline double RCAngle::getRadians() const noexcept
    {
        return mRadians;
    }

    inline void RCAngle::setRadians(double radians) noexcept
    {
        mRadians = radians;
    }

    inline double RCAngle::getDegrees() const noexcept
    {
        return RCMath::toDegrees(mRadians);
    }

    inline RCAngle& RCAngle::operator=(const RCAngle& other) noexcept
    {
        this->mRadians = other.mRadians;
        return *this;
    }

    inline RCAngle RCAngle::operator+(const RCAngle& v) const noexcept
    {
        return RCAngle(mRadians + v.getRadians());
    }

    inline RCAngle RCAngle::operator-(const RCAngle& v) const noexcept
    {
        return RCAngle(mRadians - v.getRadians());
    }

    inline RCAngle& RCAngle::operator+=(const RCAngle& v) noexcept
    {
        mRadians += v.getRadians();
        sanitize();
        return *this;
    }

    inline RCAngle& RCAngle::operator-=(const RCAngle& v) noexcept
    {
        mRadians -= v.getRadians();
        sanitize();
        return *this;
    }

    inline RCAngle RCAngle::multiplyByAndRound(double val) const noexcept
    {
        RCAngle ret(mRadians);
        ret.setRadians(std::round(ret.getRadians() * val));
        return ret;
    }

    inline RCAngle RCAngle::multiplyBy(double val) const noexcept
    {
        RCAngle ret(mRadians * val);
        return ret;
    }

    inline void RCAngle::sanitize() noexcept
    {
        // For an extra large radian value, the following code can
        // quickly change it to be within a reasonable value range
        if (mRadians > Constants::MATH_PI || mRadians < -Constants::MATH_PI)
            mRadians -= std::floor(mRadians / TWO_PI) * TWO_PI;
        
        while (mRadians > Constants::MATH_PI)
            mRadians -= TWO_PI;
        while (mRadians < -Constants::MATH_PI)
            mRadians += TWO_PI;
    }

}}}    // namespace Autodesk::RealityComputing::Foundation
