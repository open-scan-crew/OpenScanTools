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
#include <foundation/RCVector.h>

namespace Autodesk { namespace RealityComputing { namespace Foundation {
    class RCRotationMatrix;
    class RCTransform;

    /// \brief Represents a unit quaternion (q = w + xi + yj + zk) consisting
    /// of scalar (w) and a 3D vector (x,y,z)
    class RC_COMMON_API RCQuaternion
    {
    public:
        RCVector4d values;

    public:
        RCQuaternion();
        RCQuaternion(double w, const RCVector3d& vector);
        RCQuaternion(const RCVector4d& vals);

        RCQuaternion operator*(const RCQuaternion& rhs) const;
        RCQuaternion& operator*=(const RCQuaternion& rhs);

        RCVector3d operator*(const RCVector3d& vec) const;

        /// \brief Returns the negation of this quaternion, i.e. (-w, -x, -y,
        /// -z).
        const RCQuaternion operator-() const;

        bool operator==(const RCQuaternion& rhs) const;
        bool operator!=(const RCQuaternion& rhs) const;
        bool equals(const RCQuaternion& rhs, double epsilon) const;

        /// \brief Returns the conjugate of this quaternion, i.e. (w, -x, -y,
        /// -z).
        void conjugate();
        double length() const;
        double lengthSqrd() const;
        void identity();
        void normalize();

        RCTransform toTransform() const;
        RCRotationMatrix toMatrix() const;
        void toAxisAngle(RCVector3d& axis, double& angle) const;
        static RCQuaternion slerp(const RCQuaternion& q1, const RCQuaternion& q2, double distance);
    };
}}}    // namespace Autodesk::RealityComputing::Foundation
