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

#include "RCScopedPointer.h"
#include "managedAPI/Wrap_RCTransform.h"
#include "managedAPI/Wrap_RCVector.h"
#include "globals.h"
#include <foundation/RCCylinder.h>

namespace Autodesk { namespace RealityComputing { namespace Managed {

    ///
    /// \brief Represents a bounded or an infinite cylinder
    ///
    public ref struct RCCylinder
    {
    public:
        RCCylinder()
        {
            mCylinder = new NS_RCFoundation::RCCylinder();
        }

        RCCylinder(const NS_RCFoundation::RCCylinder% other)
        {
            mCylinder = new NS_RCFoundation::RCCylinder(other);
        }

        /// \brief Construct a cylinder of infinite length
        /// \param radius Radius of the cylinder
        /// \param origin Center point of the base of the cylinder
        /// \param axisOfSymmetry Normal vector to the cross section of the cylinder
        RCCylinder(double radius, RCVector3d origin, RCVector3d axisOfSymmetry)
        {
            mCylinder = new NS_RCFoundation::RCCylinder(radius, origin.ToAcGePoint3D(),
                axisOfSymmetry.ToAcGeVector3D());
        }

        /// TODO: Can't find this constructor with current libraries, causes link errors.
        /// \brief Construct a cylinder of finite length
        /// \param radius Radius of the cylinder
        /// \param origin Center point of the base of the cylinder
        /// \param axisOfSymmetry Normal vector to the cross section of the cylinder
        /// \param height Height of cylinder
        /*RCCylinder(double radius, RCVector3d origin, RCVector3d axisOfSymmetry, double height)
        {
            mCylinder = new NS_RCFoundation::RCCylinder(radius, origin->ToAcGePoint3D(),
                axisOfSymmetry->ToAcGeVector3D(), height);
        }*/

        property RCVector3d Origin
        {
            RCVector3d get()
            {
                return RCVector3d(mCylinder->getOrigin());
            }
            void set(RCVector3d _origin)
            {
                mCylinder->setOrigin(_origin.ToAcGePoint3D());
            }
        }

        property RCVector3d AxisOfSymmetry
        {
            RCVector3d get()
            {
                return RCVector3d(mCylinder->getAxisOfSymmetry());
            }
            void set(RCVector3d _axisOfSymmetry)
            {
                mCylinder->setAxisOfSymmetry(_axisOfSymmetry.ToAcGeVector3D());
            }
        }

        property double Radius
        {
            double get()
            {
                return mCylinder->getRadius();
            }
            void set(double _radius)
            {
                mCylinder->setRadius(_radius);
            }
        }

        property double Height
        {
            double get()
            {
                return mCylinder->getHeight();
            }
            void set(double _height)
            {
                mCylinder->setHeight(_height);
            }
        }

        /// \brief Check if the cylinder is bounded (finite length) or not
        bool IsBounded()
        {
            return mCylinder->isBounded();
        }

        /// \brief Transform and mutate the cylinder
        /// \param transform Transformation matrix
        /// \return Reference to the transformed and mutated self
        RCCylinder^ TransformBy(RCTransform% transform)
        {
            mCylinder->transformBy(transform.ToAcGeMatrix3d());
            return this;
        }

        /// \brief Get teh closest point on the surface of the cylinder to a point in space
        RCVector3d GetClosestPointTo(RCVector3d point)
        {
            return RCVector3d(mCylinder->getClosestPointTo(point.ToAcGePoint3D()));
        }

        virtual System::String^ ToString() override
        {
            System::String^ result = "Origin:  " + Origin + ".\n";
            result += "Radius:  " + Radius + ".\n";
            result += "Axis of Symmetry:  " + AxisOfSymmetry + ".\n";
            result += "Height:  " + Height;
            return result;
        }

        NS_RCData::RCCylinder ToReCapObject()
        {
            auto cylinder = NS_RCFoundation::RCCylinder(Radius, Origin.ToAcGePoint3D(), AxisOfSymmetry.ToAcGeVector3D());
            cylinder.setHeight(Height);
            return cylinder;
        }

    private:
        RCScopedPointer<NS_RCFoundation::RCCylinder> mCylinder;
    };
}}}    // namespace Autodesk::RealityComputing::Managed