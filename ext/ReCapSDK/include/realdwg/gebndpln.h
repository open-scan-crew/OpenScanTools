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
// This file contains the class AcGeBoundedPlane - A mathematical entity
// used to represent a 4-side bounded plane in 3-space.

#ifndef AC_GEBNDPLN_H
#define AC_GEBNDPLN_H

#include "geplanar.h"
#include "geplane.h"
#pragma pack (push, 8)

class AcGePlane;
class AcGeVector3d;
class AcGePoint3d;
class AcGePoint2d;
class AcGeLineSeg3d;

class

AcGeBoundedPlane : public AcGePlanarEnt
{
public:
    GE_DLLEXPIMPORT AcGeBoundedPlane();
    GE_DLLEXPIMPORT AcGeBoundedPlane(const AcGeBoundedPlane& plane);
    GE_DLLEXPIMPORT AcGeBoundedPlane(const AcGePoint3d& origin, const AcGeVector3d& uVec,
                     const AcGeVector3d& vVec);
    GE_DLLEXPIMPORT AcGeBoundedPlane(const AcGePoint3d& p1, const AcGePoint3d& origin,
                     const AcGePoint3d& p2);

    // Intersection.
    //
    GE_DLLEXPIMPORT Adesk::Boolean    intersectWith (const AcGeLinearEnt3d& linEnt, AcGePoint3d& point,
                                     const AcGeTol& tol = AcGeContext::gTol) const;
    GE_DLLEXPIMPORT Adesk::Boolean    intersectWith (const AcGePlane& plane, AcGeLineSeg3d& results,
                                     const AcGeTol& tol = AcGeContext::gTol) const;
    GE_DLLEXPIMPORT Adesk::Boolean    intersectWith (const AcGeBoundedPlane& plane, AcGeLineSeg3d& result,
                                     const AcGeTol& tol = AcGeContext::gTol) const;

    // Set methods.
    //
    GE_DLLEXPIMPORT AcGeBoundedPlane& set           (const AcGePoint3d& origin,
                                     const AcGeVector3d& uVec,
                                     const AcGeVector3d& vVec);
    GE_DLLEXPIMPORT AcGeBoundedPlane& set           (const AcGePoint3d& p1,
                                     const AcGePoint3d& origin,
                                     const AcGePoint3d& p2);
    // Assignment operator.
    //
    GE_DLLEXPIMPORT AcGeBoundedPlane& operator =    (const AcGeBoundedPlane& bplane);
};

#pragma pack (pop)
#endif
