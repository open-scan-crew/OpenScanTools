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
// This file contains the class AcGeSplineEnt2d - A mathematical entity
// used to represent a different types of spline curves in 3-space.

#ifndef AC_GESPNT2d_H
#define AC_GESPNT2d_H

#include "gecurv2d.h"
#include "gekvec.h"
#include "gept2dar.h"
#include "gevec2d.h"
#include "gepnt2d.h"
#include "gept2dar.h"
#pragma pack (push, 8)

class AcGeKnotVector;

class 

AcGeSplineEnt2d : public AcGeCurve2d
{
public:

    // Definition of spline
    //
    GE_DLLEXPIMPORT Adesk::Boolean    isRational            () const;
    GE_DLLEXPIMPORT int               degree                () const;
    GE_DLLEXPIMPORT int               order                 () const;
    GE_DLLEXPIMPORT int               numKnots              () const;
    GE_DLLEXPIMPORT const
    AcGeKnotVector&   knots                 () const;
    GE_DLLEXPIMPORT int               numControlPoints      () const;
    GE_DLLEXPIMPORT int               continuityAtKnot      (int idx, const AcGeTol& tol =
                                             AcGeContext::gTol) const;

    GE_DLLEXPIMPORT double            startParam            () const;
    GE_DLLEXPIMPORT double            endParam              () const;
    GE_DLLEXPIMPORT AcGePoint2d       startPoint            () const;
    GE_DLLEXPIMPORT AcGePoint2d       endPoint              () const;

    // Interpolation data
    //
    GE_DLLEXPIMPORT Adesk::Boolean    hasFitData            () const;

    // Editting
    //
    GE_DLLEXPIMPORT double            knotAt                (int idx) const;
    GE_DLLEXPIMPORT AcGeSplineEnt2d&  setKnotAt             (int idx, double val);

    GE_DLLEXPIMPORT AcGePoint2d       controlPointAt        (int idx) const;
    GE_DLLEXPIMPORT AcGeSplineEnt2d&  setControlPointAt     (int idx, const AcGePoint2d& pnt);

    // Assignment operator.
    //
    GE_DLLEXPIMPORT AcGeSplineEnt2d&  operator =            (const AcGeSplineEnt2d& spline);

protected:
    GE_DLLEXPIMPORT AcGeSplineEnt2d ();
    GE_DLLEXPIMPORT AcGeSplineEnt2d (const AcGeSplineEnt2d&);
};

#pragma pack (pop)
#endif

