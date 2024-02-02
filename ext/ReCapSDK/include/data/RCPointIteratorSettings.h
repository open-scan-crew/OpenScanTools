//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Copyright 2019 by Autodesk, Inc.
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

#include <data/RCProjectIODef.h>

namespace Autodesk { namespace RealityComputing { namespace Data {

    /// \brief Settings for creating a point iterator
    class RC_PROJECTIO_API RCPointIteratorSettings
    {
    public:
        /// \brief Default constructor
        RCPointIteratorSettings();

        /// \brief Default destructor
        ~RCPointIteratorSettings();

        /// \brief Copy constructor
        RCPointIteratorSettings(const RCPointIteratorSettings& other);

        /// \brief Assignment operator
        RCPointIteratorSettings& operator=(const RCPointIteratorSettings& other);

        /// \brief  Check if to include only visible points.
        ///         Default value is false (i.e. the point iterator will be created to include all points).
        /// \return \b true if the point iterator includes only visible points. \b false otherwise.
        bool getIsVisiblePointsOnly() const;

        /// \brief  Set if to include only visible points.
        ///         Default value is false (i.e. the point iterator will be created to include all points).
        /// \param  value \b true to include only visible points. \b false otherwise.
        void setIsVisiblePointsOnly(bool value);

        /// \brief  Get the point density with which the point iterator will be created.
        /// \details The point density controls the number of points obtained from the point cloud by specifying
        ///          the edge length of the smallest cubic volume that a single point can occupy. The value
        ///          represents the cubic volume of a single point in millimeters. Lower values increase
        ///          the number of points that are imported and improve the density of the point cloud.
        ///          If the density value is less than or equal to 0, or by default, the point iterator
        ///          will be created with the highest density.
        /// \return The point density.
        double getDensity() const;

        /// \brief  Set the point density with which the point iterator will be created.
        /// \details The point density controls the number of points obtained from the point cloud by specifying
        ///          the edge length of the smallest cubic volume that a single point can occupy. The value
        ///          represents the cubic volume of a single point in millimeters. Lower values increase
        ///          the number of points that are imported and improve the density of the point cloud.
        ///          If the density value is less than or equal to 0, or by default, the point iterator
        ///          will be created with the highest density.
        /// \param  value The new density to set.
        void setDensity(double value);

        /// \brief  Check if a read-only flag is set in the settings.
        /// \return \b true if a read-only flag is set in the settings. \b false otherwise.
        /// \note A read-only point iterator will be created by default.
        bool getIsReadOnly() const;

        /// \brief  Set if a read-only point iterator will be created.
        /// \param  value \b true to create a read-only point iterator. \b false otherwise.
        /// \note The read-only flag is set by default, if this method is not called.
        void setIsReadOnly(bool value);

    private:
        class Impl;
        Impl* mImpl;
    };
}}}    // namespace Autodesk::RealityComputing::Data
