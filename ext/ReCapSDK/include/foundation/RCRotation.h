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

#include <foundation/RCTransform.h>
#include <foundation/RCVectorFwd.h>

namespace Autodesk { namespace RealityComputing { namespace Foundation {
    class RCGenericRotationImpl;

    // This class represents an abstract notion of rotation.
    // Since there are many different that ways to represent rotation,
    // and converting between representations incurs error, this class
    // attempts to abstract away those representation, and minimize the
    // amount of conversions needed.
    class RC_COMMON_API RCGenericRotation
    {
    public:
        RCGenericRotation();
        ~RCGenericRotation();

        void setAsEuler(const RCVector3d& euler);
        void setAsAxisAngle(const RCVector3d& axis, double angle);
        void setAsMatrix(const RCRotationMatrix& mat);

        void getAsEulerAngles(RCVector3d& eulerOut) const;
        void getAsMatrix(RCRotationMatrix& rotMat) const;

    private:
        RCGenericRotation(const RCGenericRotation&);
        RCGenericRotation& operator=(const RCGenericRotation&);

        RCGenericRotationImpl* mRotImpl;
    };

}}}    // namespace Autodesk::RealityComputing::Foundation
