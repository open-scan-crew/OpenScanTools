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
// This file contains:
// The AcGePointOnCurve2d class which is used to keep information of a point
// belonging to a 2d curve

#ifndef AC_GEPONC2D_H
#define AC_GEPONC2D_H

#include "gepent2d.h"
#pragma pack (push, 8)

class AcGeCurve2d;

class

AcGePointOnCurve2d : public AcGePointEnt2d
{
public:
    GE_DLLEXPIMPORT AcGePointOnCurve2d  ();
    GE_DLLEXPIMPORT AcGePointOnCurve2d  (const AcGeCurve2d& crv);
    GE_DLLEXPIMPORT AcGePointOnCurve2d  (const AcGeCurve2d& crv, double param);
    GE_DLLEXPIMPORT AcGePointOnCurve2d  (const AcGePointOnCurve2d& src);

    // Assignment operator.
    //
    GE_DLLEXPIMPORT AcGePointOnCurve2d& operator =     (const AcGePointOnCurve2d& src);

    // Query functions.
    //
    GE_DLLEXPIMPORT const AcGeCurve2d*  curve          () const;
    GE_DLLEXPIMPORT double              parameter      () const;

    // Functions to evaluate a point.
    //
    GE_DLLEXPIMPORT AcGePoint2d         point          () const;
    GE_DLLEXPIMPORT AcGePoint2d         point          (double param);
    GE_DLLEXPIMPORT AcGePoint2d         point          (const AcGeCurve2d& crv, double param);

    // Functions to evaluate the derivatives.
    //
    GE_DLLEXPIMPORT AcGeVector2d        deriv          (int order) const;
    GE_DLLEXPIMPORT AcGeVector2d        deriv          (int order, double param);
    GE_DLLEXPIMPORT AcGeVector2d        deriv          (int order, const AcGeCurve2d& crv,
                                        double param);
    // Singularity
    //
    GE_DLLEXPIMPORT Adesk::Boolean      isSingular     (const AcGeTol&  tol =
                                        AcGeContext::gTol) const;
    GE_DLLEXPIMPORT Adesk::Boolean      curvature      (double& res);
    GE_DLLEXPIMPORT Adesk::Boolean      curvature      (double param, double& res);
    // Set functions.
    //
    GE_DLLEXPIMPORT AcGePointOnCurve2d& setCurve       (const AcGeCurve2d& crv);
    GE_DLLEXPIMPORT AcGePointOnCurve2d& setParameter   (double param);
};

#pragma pack (pop)
#endif

