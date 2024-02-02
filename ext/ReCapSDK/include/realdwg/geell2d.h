//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2020 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
//////////////////////////////////////////////////////////////////////////////
//
// DESCRIPTION:
//
// This file contains the class AcGeEllipArc2d - A mathematical entity
// used to represent an ellipse in 2-space.

#ifndef AC_GEELL2D_H
#define AC_GEELL2D_H

#include "gecurv2d.h"
#include "gevec2d.h"
#include "gepnt2d.h"
#include "geponc2d.h"
#include "geintrvl.h"
#pragma pack (push, 8)

class AcGeCircArc2d;
class AcGePlanarEnt;
class AcGeEllipArc2d;
class AcGeLinearEnt2d;


class

AcGeEllipArc2d : public AcGeCurve2d
{
public:
    GE_DLLEXPIMPORT AcGeEllipArc2d();
    GE_DLLEXPIMPORT AcGeEllipArc2d(const AcGeEllipArc2d& ell);
    GE_DLLEXPIMPORT AcGeEllipArc2d(const AcGeCircArc2d& arc);
    GE_DLLEXPIMPORT AcGeEllipArc2d(const AcGePoint2d& cent, const AcGeVector2d& majorAxis,
                   const AcGeVector2d& minorAxis, double majorRadius,
                   double minorRadius);
    GE_DLLEXPIMPORT AcGeEllipArc2d(const AcGePoint2d& cent, const AcGeVector2d& majorAxis,
                   const AcGeVector2d& minorAxis, double majorRadius,
                   double minorRadius, double startAngle, double endAngle);

    // Intersection with other geometric objects.
    //
    GE_DLLEXPIMPORT Adesk::Boolean intersectWith (const AcGeLinearEnt2d& line, int& intn,
                                  AcGePoint2d& p1, AcGePoint2d& p2,
                                  const AcGeTol& tol = AcGeContext::gTol) const;
    // Inquiry Methods
    //
    GE_DLLEXPIMPORT Adesk::Boolean isCircular    (const AcGeTol& tol = AcGeContext::gTol) const;

    // Test if point is inside full ellipse
    //
    GE_DLLEXPIMPORT Adesk::Boolean isInside      (const AcGePoint2d& pnt,
                                  const AcGeTol& tol = AcGeContext::gTol) const;


    // Definition of ellipse
    //
    GE_DLLEXPIMPORT AcGePoint2d    center        () const;
    GE_DLLEXPIMPORT double         minorRadius   () const;
    GE_DLLEXPIMPORT double         majorRadius   () const;
    GE_DLLEXPIMPORT AcGeVector2d   minorAxis     () const;
    GE_DLLEXPIMPORT AcGeVector2d   majorAxis     () const;
    GE_DLLEXPIMPORT double         startAng      () const;
    GE_DLLEXPIMPORT double         endAng        () const;
    GE_DLLEXPIMPORT AcGePoint2d    startPoint    () const;
    GE_DLLEXPIMPORT AcGePoint2d    endPoint      () const;
    GE_DLLEXPIMPORT Adesk::Boolean isClockWise   () const;

    GE_DLLEXPIMPORT AcGeEllipArc2d& setCenter     (const AcGePoint2d& cent);
    GE_DLLEXPIMPORT AcGeEllipArc2d& setMinorRadius(double rad);
    GE_DLLEXPIMPORT AcGeEllipArc2d& setMajorRadius(double rad);
    GE_DLLEXPIMPORT AcGeEllipArc2d& setAxes       (const AcGeVector2d& majorAxis, const AcGeVector2d& minorAxis);
    GE_DLLEXPIMPORT AcGeEllipArc2d& setAngles     (double startAngle, double endAngle);
    GE_DLLEXPIMPORT AcGeEllipArc2d& set           (const AcGePoint2d& cent,
                                   const AcGeVector2d& majorAxis,
                                   const AcGeVector2d& minorAxis,
                                   double majorRadius, double minorRadius);
    GE_DLLEXPIMPORT AcGeEllipArc2d& set           (const AcGePoint2d& cent,
                                   const AcGeVector2d& majorAxis,
                                   const AcGeVector2d& minorAxis,
                                   double majorRadius, double minorRadius,
                                   double startAngle, double endAngle);
    GE_DLLEXPIMPORT AcGeEllipArc2d& set           (const AcGeCircArc2d& arc);

    // Assignment operator.
    //
    GE_DLLEXPIMPORT AcGeEllipArc2d& operator =    (const AcGeEllipArc2d& ell);
};

#pragma pack (pop)
#endif

