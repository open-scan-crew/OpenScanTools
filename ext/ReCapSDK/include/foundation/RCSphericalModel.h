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
#include <foundation/RCAngle.h>
#include <foundation/RCMath.h>
#include <foundation/RCVectorFwd.h>

namespace Autodesk { namespace RealityComputing { namespace Foundation {

    ///\brief Spherical Model defines the mapping from 3D (Cartesian or polar) to Image space and back.
    class RC_COMMON_API RCSphericalModel
    {
    public:
        ///\brief Construct RCSphericalModel for 'standard' panorama (360x180, square pixels).
        ///\param pixelSize Angle between the neighboring pixels
        ///\return RCSphericalModel defined by the input pixel resolution
        static RCSphericalModel standardPanorama(const RCAngle& pixelSize);

        RCSphericalModel();
        // Custom constructor for non-standard constraints.
        RCSphericalModel(const RCAngle& azPixelSize, const RCAngle& elPixelSize, const RCAngle& azStart = RCAngle::createFromRadians(Constants::MATH_PI),
                         const RCAngle& azEnd   = RCAngle::createFromRadians(-Constants::MATH_PI),
                         const RCAngle& elStart = RCAngle::createFromRadians(Constants::MATH_PI / 2.0),
                         const RCAngle& elEnd   = RCAngle::createFromRadians(-Constants::MATH_PI / 2.0));

        virtual ~RCSphericalModel();

        RCSphericalModel(const RCSphericalModel& other);
        RCSphericalModel& operator=(const RCSphericalModel& other);

        ///\brief Get number of columns in this spherical model
        ///\return Number of columns in this spherical model
        size_t getColumnCount() const;

        ///\brief Get number of rows in this spherical model
        ///\return Number of rows in this spherical model
        size_t getRowCount() const;

        ///\brief Structure to store the angle information including range and step
        struct AngularParameters
        {
            ///\brief Start angle
            double start;
            ///\brief End angle
            double end;
            ///\brief angle difference between neighboring pixels
            double step;
            AngularParameters(double p0, double p1, double p2) : start(p0), end(p1), step(p2)
            {
            }
        };

        ///\brief Get angle parameters in azimuth direction
        ///\return Angle parameters in azimuth direction
        AngularParameters getAzimuthParameters() const;
        ///\brief Get angle parameters in elevation direction
        ///\return Angle parameters in elevation direction
        AngularParameters getElevationParameters() const;

        ///\brief Structure to represent pixel location using pair of indices
        ///       The first one is along azimuth direction
        ////      The second one is along elevation direction
        struct PixelIndex
        {
            PixelIndex() = default;
            PixelIndex(size_t _x, size_t _y) : x(_x), y(_y)
            {
            }
            size_t x = 0;
            size_t y = 0;
        };

        ///\brief Get the image coordinates from a Cartesian point.  Makes a local copy.
        ///\param xyz Input Cartesian point
        PixelIndex imageFromCartesian(RCVector3d xyz) const;

        ///\brief Get the (normalized) Cartesian point at corner of pixel.
        ///\param p Input pixel index
        RCVector3d cartesianFromImage(const PixelIndex& p) const;

        ///\brief Structure to represent pixel location using pair of angles
        ///       The first one is along azimuth direction
        ////      The second one is along elevation direction
        struct PolarIndex
        {
            PolarIndex() = default;
            PolarIndex(RCAngle azimuth, RCAngle elevation) : first(azimuth), second(elevation)
            {
            }
            RCAngle first;
            RCAngle second;
            RCAngle azimuth() const
            {
                return first;
            }
            RCAngle elevation() const
            {
                return second;
            }
        };

        ///\brief  Check if the specified polar angle is within model bounds?
        ///\param  p Input polar angle
        ///\return True if the input polar angle is within model bound. Otherwise False
        bool valid(const PolarIndex& p) const;

        ///\brief Get the polar angles at minimum corner of pixel.
        ///\param p Input pixel index
        ///\return Result polar angles
        PolarIndex polarFromImage(const PixelIndex& p) const;
        ///\brief Get the pixel index of the specified polar angle
        ///\param p Input polar angle
        ///\return Pixel index of the specified polar angle
        PixelIndex imageFromPolar(const PolarIndex& p) const;

        ///\brief Compute scaling factor that can be represented with the specified number of bits.
        double computeTargetPrecision(const unsigned int bits) const;

        ///\brief Compute positive deviation from actual to corner of provided pixel.
        ///       Use this instead of trying to compute manually!
        PolarIndex getAngularDeviation(const PixelIndex& p, const PolarIndex& actual) const;
        PolarIndex getPixelSize() const;

        ///\brief Get the angle information of this spherical model
        ///\param azStart Start of azimuth angle
        ///\param azStride Stride of azimuth angle: the difference between the neighboring pixels on azimuth direction
        ///\param elStart Start of elevation angle
        ///\param elStride Stride of elevation angle: the difference between the neighboring pixels on elevation direction
        void getAngleInformation(double& azStart, double& azStride, double& elStart, double& elStride) const;

    private:
        struct Impl;
        Impl* mImpl;
    };

    ///\brief Convenience functions for PolarIndex manipulations
    RC_COMMON_API const RCSphericalModel::PolarIndex operator+(const RCSphericalModel::PolarIndex& a, const RCSphericalModel::PolarIndex& b);
    RC_COMMON_API const RCSphericalModel::PolarIndex operator-(const RCSphericalModel::PolarIndex& a, const RCSphericalModel::PolarIndex& b);
}}}    // namespace Autodesk::RealityComputing::Foundation
