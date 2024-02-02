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
// This file contains the class AcGeLinearEnt2d - A mathematical entity
// used to represent a line in 2-space.

#ifndef AC_GELENT2D_H
#define AC_GELENT2D_H

#include "gecurv2d.h"
#include "gepnt2d.h"
#include "gevec2d.h"
#pragma pack (push, 8)

class AcGeCircArc2d;

class

AcGeLinearEnt2d : public AcGeCurve2d
{
public:
    // Intersection with other geometric objects.
    //
    GE_DLLEXPIMPORT Adesk::Boolean   intersectWith  (const AcGeLinearEnt2d& line, AcGePoint2d& intPnt,
                                     const AcGeTol& tol = AcGeContext::gTol) const;

    // Find the overlap with other AcGeLinearEnt object
    //
    GE_DLLEXPIMPORT Adesk::Boolean   overlap        (const AcGeLinearEnt2d& line,
                                     AcGeLinearEnt2d*& overlap,
                                     const AcGeTol& tol = AcGeContext::gTol) const;
    // Direction tests.
    //
    GE_DLLEXPIMPORT Adesk::Boolean   isParallelTo   (const AcGeLinearEnt2d& line,
                                     const AcGeTol& tol = AcGeContext::gTol) const;
    GE_DLLEXPIMPORT Adesk::Boolean   isPerpendicularTo(const AcGeLinearEnt2d& line,
                                      const AcGeTol& tol = AcGeContext::gTol) const;
    // Test if two lines are colinear.
    //
    GE_DLLEXPIMPORT Adesk::Boolean   isColinearTo   (const AcGeLinearEnt2d& line,
                                     const AcGeTol& tol = AcGeContext::gTol) const;
    // Perpendicular through a given point
    //
    GE_DLLEXPIMPORT void             getPerpLine    (const AcGePoint2d& pnt, AcGeLine2d& perpLine) const;

    // Definition of line.
    //
    GE_DLLEXPIMPORT AcGePoint2d      pointOnLine    () const;
    GE_DLLEXPIMPORT AcGeVector2d     direction      () const;
    GE_DLLEXPIMPORT void             getLine        (AcGeLine2d& line) const;

    // Assignment operator.
    //
    GE_DLLEXPIMPORT AcGeLinearEnt2d& operator =     (const AcGeLinearEnt2d& line);

protected:
    GE_DLLEXPIMPORT AcGeLinearEnt2d ();
    GE_DLLEXPIMPORT AcGeLinearEnt2d (const AcGeLinearEnt2d&);
};

#pragma pack (pop)
#endif
