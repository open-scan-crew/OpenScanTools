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
// The class AcGePointOnCurve3d which is used to keep information of
// a point belonging to a 3d curve

#ifndef AC_GEPONC3D_H
#define AC_GEPONC3D_H

#include "gepent3d.h"
#pragma pack (push, 8)

class AcGeCurve3d;

class

AcGePointOnCurve3d : public AcGePointEnt3d
{
public:
    GE_DLLEXPIMPORT AcGePointOnCurve3d();
    GE_DLLEXPIMPORT AcGePointOnCurve3d(const AcGeCurve3d& crv);
    GE_DLLEXPIMPORT AcGePointOnCurve3d(const AcGeCurve3d& crv, double param);
    GE_DLLEXPIMPORT AcGePointOnCurve3d(const AcGePointOnCurve3d& src);

    // Assignment operator.
    //
    GE_DLLEXPIMPORT AcGePointOnCurve3d& operator =     (const AcGePointOnCurve3d& src);

    // Query functions.
    //
    GE_DLLEXPIMPORT const AcGeCurve3d*  curve          () const;
    GE_DLLEXPIMPORT double              parameter      () const;

    // Functions to evaluate a point.
    //
    GE_DLLEXPIMPORT AcGePoint3d         point          () const;
    GE_DLLEXPIMPORT AcGePoint3d         point          (double param);
    GE_DLLEXPIMPORT AcGePoint3d         point          (const AcGeCurve3d& crv, double param);

    // Functions to evaluate the derivatives.
    //
    GE_DLLEXPIMPORT AcGeVector3d        deriv          (int order) const;
    GE_DLLEXPIMPORT AcGeVector3d        deriv          (int order, double param);
    GE_DLLEXPIMPORT AcGeVector3d        deriv          (int order, const AcGeCurve3d& crv,
                                        double param);
    // Singularity
    //
    GE_DLLEXPIMPORT Adesk::Boolean      isSingular     (const AcGeTol& tol =
	                                AcGeContext::gTol) const;
    GE_DLLEXPIMPORT Adesk::Boolean     	curvature      (double& res);
    GE_DLLEXPIMPORT Adesk::Boolean     	curvature      (double param, double& res);

    // Set functions.
    //
    GE_DLLEXPIMPORT AcGePointOnCurve3d& setCurve       (const AcGeCurve3d& crv);
    GE_DLLEXPIMPORT AcGePointOnCurve3d& setParameter   (double param);
};

#pragma pack (pop)
#endif

