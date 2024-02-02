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
// This file contains the class AcGeAugPolyline3d - A mathematical entity
// used to represent a different types of spline curves in 3-space.

#ifndef AC_GEAPLN3D_H
#define AC_GEAPLN3D_H

#include "gecurv3d.h"
#include "gekvec.h"
#include "gept3dar.h"
#include "gevc3dar.h"
#include "gevec3d.h"
#include "gepnt3d.h"
#include "geplin3d.h"

#pragma pack (push, 8)

class 

AcGeAugPolyline3d : public AcGePolyline3d
{
public:
    GE_DLLEXPIMPORT AcGeAugPolyline3d();
    GE_DLLEXPIMPORT AcGeAugPolyline3d(const AcGeAugPolyline3d& apline);
    GE_DLLEXPIMPORT AcGeAugPolyline3d(const AcGeKnotVector& knots,
                      const AcGePoint3dArray& cntrlPnts,
                      const AcGeVector3dArray& vecBundle);
    GE_DLLEXPIMPORT AcGeAugPolyline3d(const AcGePoint3dArray& cntrlPnts,
                      const AcGeVector3dArray& vecBundle);

    // Approximation constructor
    //
    GE_DLLEXPIMPORT AcGeAugPolyline3d(const AcGeCurve3d& curve,
                      double fromParam, double toParam, 
		              double apprEps);

    // Assignment operator.
    //
    GE_DLLEXPIMPORT AcGeAugPolyline3d& operator = (const AcGeAugPolyline3d& apline);

    // Points
    //
    GE_DLLEXPIMPORT AcGePoint3d           getPoint(int idx) const;
    GE_DLLEXPIMPORT AcGeAugPolyline3d&    setPoint(int idx, AcGePoint3d pnt);
    GE_DLLEXPIMPORT void                  getPoints(AcGePoint3dArray& pnts) const;

    // Tangent bundle 
    //
    GE_DLLEXPIMPORT AcGeVector3d          getVector(int idx) const;
    GE_DLLEXPIMPORT AcGeAugPolyline3d&    setVector(int idx, AcGeVector3d pnt);
    GE_DLLEXPIMPORT void                  getD1Vectors(AcGeVector3dArray& tangents) const;

    // D2 Tangent bundle 
    //
    GE_DLLEXPIMPORT AcGeVector3d          getD2Vector(int idx) const;
    GE_DLLEXPIMPORT AcGeAugPolyline3d&    setD2Vector(int idx, AcGeVector3d pnt);
    GE_DLLEXPIMPORT void                  getD2Vectors(AcGeVector3dArray& d2Vectors) const;

	// Approximation tolerance
	//
	GE_DLLEXPIMPORT double                approxTol      () const;
    GE_DLLEXPIMPORT AcGeAugPolyline3d&    setApproxTol   (double approxTol);

};

#pragma pack (pop)
#endif
