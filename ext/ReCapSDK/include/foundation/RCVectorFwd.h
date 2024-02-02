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

namespace Autodesk { namespace RealityComputing { namespace Foundation {
    template <class T>
    struct RC_COMMON_API RCVector2T;
    typedef RCVector2T<double> RCVector2d;
    typedef RCVector2T<float> RCVector2f;

    template <class T>
    struct RC_COMMON_API RCVector3T;
    typedef RCVector3T<double> RCVector3d;
    typedef RCVector3T<float> RCVector3f;
    typedef RCVector3T<unsigned char> RCVector3ub;

    template <class T>
    struct RC_COMMON_API RCVector4T;
    typedef RCVector4T<double> RCVector4d;
    typedef RCVector4T<float> RCVector4f;
    typedef RCVector4T<unsigned char> RCVector4ub;

}}}    // namespace Autodesk::RealityComputing::Foundation
