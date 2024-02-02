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

namespace Autodesk { namespace RealityComputing { namespace Managed {
    public value struct RCPlane
    {
        void FromAcGePlane(AcGePlane plane)
        {
            AcGePoint3d orgUV;
            AcGeVector3d uAxis, vAxis, normal = plane.normal();
            plane.getCoordSystem(orgUV, uAxis, vAxis);
            double a, b, c, d;
            plane.getCoefficients(a, b, c, d);

            _normal = RCVector3d(normal);
            _origin = RCVector3d(orgUV);
            _uAxis = RCVector3d(uAxis);
            _vAxis = RCVector3d(vAxis);
            _a = a;
            _b = b;
            _c = c;
            _d = d;
        }

        RCPlane(const AcGePlane% plane)
        {
            FromAcGePlane(plane);
        }

        RCPlane(RCVector3d origin, RCVector3d normal)
        {
            FromAcGePlane(AcGePlane(origin.ToAcGePoint3D(), normal.ToAcGeVector3D()));
        }

        RCPlane(RCVector3d origin, RCVector3d uAxis, RCVector3d vAxis)
        {
            FromAcGePlane(AcGePlane(origin.ToAcGePoint3D(), uAxis.ToAcGeVector3D(), vAxis.ToAcGeVector3D()));
        }

        RCPlane(Double a, Double b, Double c, Double d)
        {
            FromAcGePlane(AcGePlane(a, b, c, d));
        }

        RCPlane(NS_RevitDB::Plane^ revitPlane)
        {
            if (revitPlane == nullptr || !revitPlane->IsValidObject)
            {
                throw gcnew NullReferenceException("Revit Plane object is null or invalid.");
            }
            RCVector3d orgn(revitPlane->Origin);
            RCVector3d nrml(revitPlane->Normal);
            FromAcGePlane(AcGePlane(orgn.ToAcGePoint3D(), nrml.ToAcGeVector3D()));
        }

        AcGePlane ToReCapObject()
        {
            return AcGePlane(_a, _b, _c, _d);
        }

        NS_RevitDB::Plane^ ToRevitObject()
        {
            return NS_RevitDB::Plane::CreateByNormalAndOrigin(Normal.ToRevitObject(), Origin.ToRevitObject());
        }

        property RCVector3d Origin { RCVector3d get() { return _origin; } void set(RCVector3d or ) { _origin = or ; } };
        property RCVector3d Normal { RCVector3d get() { return _normal; } void set(RCVector3d norm) { _normal = norm; } };
        property RCVector3d UAxis { RCVector3d get() { return _uAxis; } void set(RCVector3d u) { _uAxis = u; } };
        property RCVector3d VAxis { RCVector3d get() { return _vAxis; } void set(RCVector3d v) { _vAxis = v; } };
        property double A { double get() { return _a; } void set(double a) { _a = a; } };
        property double B { double get() { return _b; } void set(double b) { _b = b; } };
        property double C { double get() { return _c; } void set(double c) { _c = c; } };
        property double D { double get() { return _d; } void set(double d) { _d = d; } };

    private:
        RCVector3d _origin;
        RCVector3d _normal;
        RCVector3d _uAxis;
        RCVector3d _vAxis;
        double _a, _b, _c, _d;
    };
}}}    // namespace Autodesk::RealityComputing::Managed