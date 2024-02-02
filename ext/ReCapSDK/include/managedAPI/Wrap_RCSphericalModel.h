//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Copyright 2020 by Autodesk, Inc.
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

#include "globals.h"

#include "RCScopedPointer.h"
#include "managedAPI/Wrap_RCVector.h"

using namespace System;

namespace Autodesk { namespace RealityComputing { namespace Managed {
    public ref class RCSphericalModel
    {
    public:
        RCSphericalModel(NS_RCFoundation::RCSharedPtr<NS_RCFoundation::RCSphericalModel> recapModel);
        ///\brief Get number of columns in this spherical model
        ///\return Number of columns in this spherical model
        UInt32 GetColumnCount();

        ///\brief Get number of rows in this spherical model
        ///\return Number of rows in this spherical model
        UInt32 GetRowCount();

        ///\brief Structure to store the angle information including range and step
        value struct AngularParameters
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
            virtual System::String^ ToString() override
            {
                System::String^ result = "Start:  " + start.ToString() + ". End: " + end.ToString() + ". Step: " + step.ToString();
                return result;
            }
        };

        ///\brief Get angle parameters in azimuth direction
        ///\return Angle parameters in azimuth direction
        AngularParameters GetAzimuthParameters();
        ///\brief Get angle parameters in elevation direction
        ///\return Angle parameters in elevation direction
        AngularParameters GetElevationParameters();

        ///\brief Structure to represent pixel location using pair of indices
        ///       The first one is along azimuth direction
        ////      The second one is along elevation direction
        value struct PixelIndex
        {
            PixelIndex(UInt32 _x, UInt32 _y) : x(_x), y(_y)
            {
            }
            UInt32 x;
            UInt32 y;
        };

        ///\brief Get the image coordinates from a Cartesian point.  Makes a local copy.
        ///\param xyz Input Cartesian point
        PixelIndex ImageFromCartesian(RCVector3d xyz);

        ///\brief Get the (normalized) Cartesian point at corner of pixel.
        ///\param p Input pixel index
        RCVector3d CartesianFromImage(PixelIndex p);

        ///\brief Structure to represent pixel location using pair of angles
        ///       The first one is along azimuth direction
        ////      The second one is along elevation direction
        value struct PolarIndex
        {
            PolarIndex(double first, double second) : azimuth(first), elevation(second) {}
            double azimuth;
            double elevation;
            virtual System::String^ ToString() override
            {
                System::String^ result = "Azimuth:  " + azimuth.ToString() + ". Elevation: " + elevation.ToString();
                return result;
            }
        };

        ///\brief  Check if the specified polar angle is within model bounds?
        ///\param  p Input polar angle
        ///\return True if the input polar angle is within model bound. Otherwise False
        bool Valid(PolarIndex p);

        ///\brief Get the polar angles at minimum corner of pixel.
        ///\param p Input pixel index
        ///\return Result polar angles
        PolarIndex PolarFromImage(PixelIndex p);
        ///\brief Get the pixel index of the specified polar angle
        ///\param p Input polar angle
        ///\return Pixel index of the specified polar angle
        PixelIndex ImageFromPolar(PolarIndex p);

        ///\brief Compute scaling factor that can be represented with the specified number of bits.
        double ComputeTargetPrecision(const unsigned int bits);

        ///\brief Compute positive deviation from actual to corner of provided pixel.
        ///       Use this instead of trying to compute manually!
        PolarIndex GetAngularDeviation(PixelIndex p, PolarIndex actual);
        PolarIndex GetPixelSize();

        ///\brief Get the angle information of this spherical model
        ///\param azStart Start of azimuth angle
        ///\param azStride Stride of azimuth angle: the difference between the neighboring pixels on azimuth direction
        ///\param elStart Start of elevation angle
        ///\param elStride Stride of elevation angle: the difference between the neighboring pixels on elevation direction
        void GetAngleInformation(double% azStart, double% azStride, double% elStart, double% elStride);

    private:
        RCScopedPointer<NS_RCFoundation::RCSphericalModel> mSphericalModel;
    };
}}}    // namespace Autodesk::RealityComputing::Managed
