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
// This file contains the class AcGePolyline2d - A mathematical entity
// used to represent a different types of spline curves in 3-space.

#ifndef AC_GEPLIN2D_H
#define AC_GEPLIN2D_H

#include "gecurv2d.h"
#include "gekvec.h"
#include "gept2dar.h"
#include "gevec2d.h"
#include "gepnt2d.h"
#include "gesent2d.h"
#pragma pack (push, 8)

class

AcGePolyline2d : public AcGeSplineEnt2d
{
public:
    GE_DLLEXPIMPORT AcGePolyline2d();
    GE_DLLEXPIMPORT AcGePolyline2d(const AcGePolyline2d& src);
    GE_DLLEXPIMPORT AcGePolyline2d(const AcGePoint2dArray&);
    GE_DLLEXPIMPORT AcGePolyline2d(const AcGeKnotVector& knots,
                   const AcGePoint2dArray& points);

    // Approximate curve with polyline
    //
    GE_DLLEXPIMPORT AcGePolyline2d(const AcGeCurve2d& crv, double apprEps);

    // Interpolation data
    //
    GE_DLLEXPIMPORT int              numFitPoints () const;
    GE_DLLEXPIMPORT AcGePoint2d      fitPointAt   (int idx) const;
    GE_DLLEXPIMPORT AcGeSplineEnt2d& setFitPointAt(int idx, const AcGePoint2d& point);

    // Assignment operator.
    //
    GE_DLLEXPIMPORT AcGePolyline2d& operator =     (const AcGePolyline2d& pline);
};

#pragma pack (pop)
#endif
