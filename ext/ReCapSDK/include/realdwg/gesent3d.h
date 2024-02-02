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
// This file contains the class AcGeSplineEnt3d - A mathematical entity
// used to represent a different types of spline curves in 3-space.

#ifndef AC_GESPNT3D_H
#define AC_GESPNT3D_H

#include "gecurv3d.h"
#include "gekvec.h"
#include "gept3dar.h"
#include "gevec3d.h"
#include "gepnt3d.h"
#include "gept3dar.h"
#pragma pack (push, 8)

class AcGeKnotVector;

class

AcGeSplineEnt3d : public AcGeCurve3d
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
                                             AcGeContext::gTol ) const;

    GE_DLLEXPIMPORT double            startParam            () const;
    GE_DLLEXPIMPORT double            endParam              () const;
    GE_DLLEXPIMPORT AcGePoint3d       startPoint            () const;
    GE_DLLEXPIMPORT AcGePoint3d       endPoint              () const;

    // Interpolation data
    //
    GE_DLLEXPIMPORT Adesk::Boolean    hasFitData            () const;

    // Editting
    //
    GE_DLLEXPIMPORT double            knotAt                (int idx) const;
    GE_DLLEXPIMPORT AcGeSplineEnt3d&  setKnotAt             (int idx, double val);

    GE_DLLEXPIMPORT AcGePoint3d       controlPointAt        (int idx) const;
    GE_DLLEXPIMPORT AcGeSplineEnt3d&  setControlPointAt     (int idx, const AcGePoint3d& pnt);


    // Assignment operator.
    //
    GE_DLLEXPIMPORT AcGeSplineEnt3d&  operator =            (const AcGeSplineEnt3d& spline);

protected:
    GE_DLLEXPIMPORT AcGeSplineEnt3d ();
    GE_DLLEXPIMPORT AcGeSplineEnt3d (const AcGeSplineEnt3d&);
};

#pragma pack (pop)
#endif

