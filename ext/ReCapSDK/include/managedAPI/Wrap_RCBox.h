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

#include "managedAPI/Wrap_RCVector.h"
#include "globals.h"
#include <foundation/RCBox.h>
#include <geray3d.h>

namespace Autodesk { namespace RealityComputing { namespace Managed {

    public value struct RCBox
    {
    public:

        RCBox(Autodesk::RealityComputing::Foundation::RCBox% recapBox)
        {
            Autodesk::RealityComputing::Foundation::RCVector3d min = recapBox.getMin();
            Autodesk::RealityComputing::Foundation::RCVector3d max = recapBox.getMax();
            minPoint = RCVector3d(min.x, min.y, min.z);
            maxPoint = RCVector3d(max.x, max.y, max.z);
        }

        NS_RevitDB::BoundingBoxXYZ^ ToRevitObject()
        {
            NS_RevitDB::BoundingBoxXYZ^ boundingBox = gcnew NS_RevitDB::BoundingBoxXYZ();
            boundingBox->Min = gcnew NS_RevitDB::XYZ(minPoint.x, minPoint.y, minPoint.z);
            boundingBox->Max = gcnew NS_RevitDB::XYZ(maxPoint.x, maxPoint.y, maxPoint.z);
            return boundingBox;
        }

        NS_RCFoundation::RCBox ToReCapObject()
        {
            return NS_RCFoundation::RCBox(minPoint.ToReCapObject(), maxPoint.ToReCapObject());
        }

        bool IntersectWithRay(RCVector3d rayOrigin, RCVector3d rayDirection)
        {
            return ToReCapObject().intersectWith(AcGeRay3d(rayOrigin.ToAcGePoint3D(), rayDirection.ToAcGeVector3D()));
        }

        RCVector3d minPoint;
        RCVector3d maxPoint;

        virtual System::String^ ToString() override
        {
            System::String^ result = "Min point:  " + minPoint.ToString() + ".\n";
            result += "Max point:  " + maxPoint.ToString();
            return result;
        }
    };
}}}    // namespace Autodesk::RealityComputing::Managed
