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

#include <gevec3d.h>
#include <gepnt3d.h>
#include <gemat3d.h>

#include <foundation/RCMacros.h>
#include <foundation/RCCommonDef.h>

namespace Autodesk { namespace RealityComputing { namespace Foundation {

    ///
    /// \brief Represents a bounded or an infinite cylinder
    ///
    class RC_COMMON_API RCCylinder
    {
        RC_PIMPL_DECLARATION(RCCylinder)

    public:
        /// \brief Construct a cylinder of infinite length
        /// \param radius Radius of the cylinder
        /// \param origin Center point of the base of the cylinder
        /// \param axisOfSymmetry Normal vector to the cross section of the cylinder
        RCCylinder(double radius, const AcGePoint3d& origin, const AcGeVector3d& axisOfSymmetry);

        /// \brief Construct a cylinder of finite length
        /// \param radius Radius of the cylinder
        /// \param origin Center point of the base of the cylinder
        /// \param axisOfSymmetry Normal vector to the cross section of the cylinder
        /// \param height Height of cylinder
        RCCylinder(double radius, const AcGePoint3d& origin, const AcGeVector3d& axisOfSymmetry, double height);

        /// \brief Get the center point of the base of the cylinder
        AcGePoint3d getOrigin() const;

        /// \brief Set the center point of the base of the cylinder
        void setOrigin(const AcGePoint3d& origin);

        /// \brief Get the unit normal vector to the cross section of the cylinder
        AcGeVector3d getAxisOfSymmetry() const;

        /// \brief Change the cylinder's orientation by setting the normal vector to its cross section
        bool setAxisOfSymmetry(const AcGeVector3d& axisOfSymmetry);

        /// \brief Get the radius of the cylinder
        double getRadius() const;

        /// \brief Set the radius of the cylinder
        bool setRadius(double radius);

        /// \brief Get the height of the cylinder
        /// \return Height of cylinder if it is bounded i.e. finite length. Otherwise return maximum numeric limits of type 'double' if the cylinder is
        /// unbounded
        double getHeight() const;

        /// \brief Set the height of the cylinder
        /// \param height Only positive values  of height are allowed
        /// \return true if the height was set successfully. false if it was not set because of negative or zero height value
        bool setHeight(double height);

        /// \brief Check if the cylinder is bounded (finite length) or not
        bool isBounded() const;

        /// \brief Transform and mutate the cylinder
        /// \param transform Transformation matrix
        /// \return Reference to the transformed and mutated self
        RCCylinder& transformBy(const AcGeMatrix3d& transform);

        /// \brief Get teh closest point on the surface of the cylinder to a point in space
        AcGePoint3d getClosestPointTo(const AcGePoint3d& point) const;
    };
}}}    // namespace Autodesk::RealityComputing::Foundation
