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


namespace Autodesk { namespace RealityComputing { namespace Managed {

    /// Non axis-aligned bound block, instead of RCBox, which is axis aligned.
    public ref struct RCBoundBlock3d
    {
    public:
        RCBoundBlock3d()
        {
            mBlock = new AcGeBoundBlock3d();
        }

        RCBoundBlock3d(const AcGeBoundBlock3d% other)
        {
            mBlock = new AcGeBoundBlock3d(other);
        }

        RCBoundBlock3d(RCVector3d basePoint, RCVector3d dir1,
            RCVector3d dir2, RCVector3d dir3)
        {
            mBlock = new AcGeBoundBlock3d(basePoint.ToAcGePoint3D(), dir1.ToAcGeVector3D(),
                dir2.ToAcGeVector3D(), dir3.ToAcGeVector3D());
        }

        void GetMinMaxPoints([Out] RCVector3d% min, [Out] RCVector3d% max)
        {
            AcGePoint3d _min, _max;
            mBlock->getMinMaxPoints(_min, _max);
            min = RCVector3d(_min);
            max = RCVector3d(_max);
        }

        void Get([Out] RCVector3d% basePoint, [Out] RCVector3d% dir1,
            [Out] RCVector3d% dir2, [Out] RCVector3d% dir3)
        {
            AcGePoint3d _base;
            AcGeVector3d _dir1, _dir2, _dir3;
            mBlock->get(_base, _dir1, _dir2, _dir3);
            basePoint = RCVector3d(_base);
            dir1 = RCVector3d(_dir1);
            dir2 = RCVector3d(_dir2);
            dir3 = RCVector3d(_dir3);
        }

        void Set(RCVector3d point1, RCVector3d point2)
        {
            mBlock->set(point1.ToAcGePoint3D(), point2.ToAcGePoint3D());
        }

        void Set(RCVector3d basePoint, RCVector3d dir1,
            RCVector3d dir2, RCVector3d dir3)
        {
            mBlock->set(basePoint.ToAcGePoint3D(), dir1.ToAcGeVector3D(),
                dir2.ToAcGeVector3D(), dir2.ToAcGeVector3D());
        }

    private:
        RCScopedPointer<AcGeBoundBlock3d> mBlock;
    };
}}}    // namespace Autodesk::RealityComputing::Managed