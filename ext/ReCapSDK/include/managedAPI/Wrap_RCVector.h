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

#include <foundation/RCVector.h>
#include "globals.h"

bool areDoublesEqual(const double doubleValue1, const double doubleValue2);

namespace Autodesk { namespace RealityComputing { namespace Managed {
    public value struct RCVector3d
    {
        RCVector3d(double dx, double dy, double dz) : x(dx), y(dy), z(dz) {}

        RCVector3d(const Autodesk::RealityComputing::Foundation::RCVector3d% recapVector)
        {
            x = recapVector.x;
            y = recapVector.y;
            z = recapVector.z;
        }

        RCVector3d(Autodesk::Revit::DB::XYZ^ revitVector)
        {
            x = revitVector->X;
            y = revitVector->Y;
            z = revitVector->Z;
        }

        RCVector3d(const AcGeVector3d% acGeVector)
        {
            x = acGeVector.x;
            y = acGeVector.y;
            z = acGeVector.z;
        }

        RCVector3d(const AcGePoint3d% acGePoint)
        {
            x = acGePoint.x;
            y = acGePoint.y;
            z = acGePoint.z;
        }

        static RCVector3d operator + (RCVector3d v1, RCVector3d v2) 
        {
            return RCVector3d(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
        }

        static RCVector3d operator - (RCVector3d v1, RCVector3d v2)
        {
            return RCVector3d(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
        }

        static RCVector3d operator * (double s, RCVector3d v2)
        {
            return RCVector3d(s * v2.x, s * v2.y, s * v2.z);
        }

        NS_RevitDB::XYZ^ ToRevitObject()
        {
            return gcnew NS_RevitDB::XYZ(x, y, z);
        }

        NS_RCFoundation::RCVector3d ToReCapObject()
        {
            return NS_RCFoundation::RCVector3d(x, y, z);
        }

        AcGePoint3d ToAcGePoint3D()
        {
            return AcGePoint3d(x, y, z);
        }

        AcGeVector3d ToAcGeVector3D()
        {
            return AcGeVector3d(x, y, z);
        }

        double DistanceSqTo(RCVector3d v)
        {
            auto dif = RCVector3d(v.x - x, v.y - y, v.z - z);
            return dif.x * dif.x + dif.y * dif.y + dif.z * dif.z;
        }

        void Normalize()
        {
            double l = Length();
            if (!areDoublesEqual(l, 0.0))
            {
                x = x / l;
                y = y / l;
                z = z / l;
            }
        }

        double Length()
        {
            return sqrt(x * x + y * y + z * z);
        }

        double LengthSq()
        {
            return x * x + y * y + z * z;
        }

        double Dot(RCVector3d v)
        {
            return x * v.x + y * v.y + z * v.z;
        }

        bool Equality(RCVector3d v)
        {
            return areDoublesEqual(DistanceSqTo(v), 0.0);
        }

        static RCVector3d Min(RCVector3d v1, RCVector3d v2)
        {
            return RCVector3d
            {
                v1.x < v2.x ? v1.x : v2.x,
                v1.y < v2.y ? v1.y : v2.y,
                v1.z < v2.z ? v1.z : v2.z
            };
        }

        static RCVector3d Max(RCVector3d v1, RCVector3d v2)
        {
            return RCVector3d
            {
                v1.x > v2.x ? v1.x : v2.x,
                v1.y > v2.y ? v1.y : v2.y,
                v1.z > v2.z ? v1.z : v2.z
            };
        }

        virtual System::String^ ToString() override
        {
            return System::String::Format("({0:0.0000}, {1:0.0000}, {2:0.0000})", x, y, z);
        }

        double x;
        double y;
        double z;
    };

    public value struct RCVector2d
    {
        RCVector2d(double dx, double dy) : x(dx), y(dy) {}

        RCVector2d(const Autodesk::RealityComputing::Foundation::RCVector2d% recapVector)
        {
            x = recapVector.x;
            y = recapVector.y;
        }

        RCVector2d(const AcGeVector2d% acGeVector)
        {
            x = acGeVector.x;
            y = acGeVector.y;
        }

        static RCVector2d operator + (RCVector2d v1, RCVector2d v2)
        {
            return RCVector2d(v1.x + v2.x, v1.y + v2.y);
        }

        static RCVector2d operator - (RCVector2d v1, RCVector2d v2)
        {
            return RCVector2d(v1.x - v2.x, v1.y - v2.y);
        }

        static RCVector2d operator * (double s, RCVector2d v2)
        {
            return RCVector2d(s * v2.x, s * v2.y);
        }

        NS_RCFoundation::RCVector2d ToReCapObject()
        {
            return NS_RCFoundation::RCVector2d(x, y);
        }

        AcGeVector2d ToAcGeVector3D()
        {
            return AcGeVector2d(x, y);
        }

        virtual System::String^ ToString() override
        {
            return gcnew System::String("(" + x + ", " + y + ")");
        }

        double x;
        double y;
    };

    public value struct RCColor
    {
        RCColor(unsigned char ur, unsigned char ug, unsigned char ub, unsigned char ua) : r(ur), g(ug), b(ub), a(ua) {}

        RCColor(const NS_RCFoundation::RCVector4ub% recapColor)
        {
            r = recapColor[0];
            g = recapColor[1];
            b = recapColor[2];
            a = recapColor[3];
        }

        NS_RCFoundation::RCVector4ub ToReCapObject()
        {
            return NS_RCFoundation::RCVector4ub(r, g, b, a);
        }

        NS_RevitDB::ColorWithTransparency^ ToRevitObject()
        {
            return gcnew NS_RevitDB::ColorWithTransparency(r, g, b, a);
        }

        virtual System::String^ ToString() override
        {
            return gcnew System::String("(" + r + ", " + g + ", " + b + ", " + a + ")");
        }

        unsigned char r;
        unsigned char g;
        unsigned char b;
        unsigned char a;
    };
}}}    // namespace Autodesk::RealityComputing::Managed