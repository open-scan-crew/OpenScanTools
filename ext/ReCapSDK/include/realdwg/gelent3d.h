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
// This file contains the class AcGeLinearEnt3d - A mathematical entity
// used to represent a line in 3-space.

#ifndef AC_GELENT3D_H
#define AC_GELENT3D_H

#include "gecurv3d.h"
#include "gevec3d.h"
#include "gepnt3d.h"
#pragma pack (push, 8)

class AcGeLine3d;
class AcGeCircArc3d;
class AcGePlanarEnt;
class

AcGeLinearEnt3d : public AcGeCurve3d
{
public:
    // Intersection with other geometric objects.
    //
    GE_DLLEXPIMPORT Adesk::Boolean intersectWith (const AcGeLinearEnt3d& line,
                                  AcGePoint3d& intPt,
                                  const AcGeTol& tol = AcGeContext::gTol) const;
    GE_DLLEXPIMPORT Adesk::Boolean intersectWith (const AcGePlanarEnt& plane, AcGePoint3d& intPnt,
                                  const AcGeTol& tol = AcGeContext::gTol) const;

    // Projection-intersection with other geometric objects.
    //
    GE_DLLEXPIMPORT Adesk::Boolean projIntersectWith(const AcGeLinearEnt3d& line,
                                  const AcGeVector3d& projDir,
                                  AcGePoint3d& pntOnThisLine,
                                  AcGePoint3d& pntOnOtherLine,
                                  const AcGeTol& tol = AcGeContext::gTol) const;

    // Find the overlap with other AcGeLinearEnt object
    //
    GE_DLLEXPIMPORT Adesk::Boolean overlap       (const AcGeLinearEnt3d& line,
                                  AcGeLinearEnt3d*& overlap,
                                  const AcGeTol& tol = AcGeContext::gTol) const;

    // Containment tests.
    //
    GE_DLLEXPIMPORT Adesk::Boolean isOn          (const AcGePoint3d& pnt,
                                  const AcGeTol& tol = AcGeContext::gTol) const;
    GE_DLLEXPIMPORT Adesk::Boolean isOn          (const AcGePoint3d& pnt, double& param,
                                  const AcGeTol& tol = AcGeContext::gTol) const;
    GE_DLLEXPIMPORT Adesk::Boolean isOn          (double param,
                                  const AcGeTol& tol = AcGeContext::gTol) const;
    GE_DLLEXPIMPORT Adesk::Boolean isOn          (const AcGePlane& plane,
                                  const AcGeTol& tol = AcGeContext::gTol) const;

    // Direction tests.
    //
    GE_DLLEXPIMPORT Adesk::Boolean isParallelTo  (const AcGeLinearEnt3d& line,
                                  const AcGeTol& tol = AcGeContext::gTol) const;
    GE_DLLEXPIMPORT Adesk::Boolean isParallelTo  (const AcGePlanarEnt& plane,
                                  const AcGeTol& tol = AcGeContext::gTol) const;
    GE_DLLEXPIMPORT Adesk::Boolean isPerpendicularTo(const AcGeLinearEnt3d& line,
                                  const AcGeTol& tol = AcGeContext::gTol) const;
    GE_DLLEXPIMPORT Adesk::Boolean isPerpendicularTo(const AcGePlanarEnt& plane,
                                  const AcGeTol& tol = AcGeContext::gTol) const;

    // Test if two lines are colinear.
    //
    GE_DLLEXPIMPORT Adesk::Boolean isColinearTo  (const AcGeLinearEnt3d& line,
                                  const AcGeTol& tol = AcGeContext::gTol) const;

    // Perpendicular through a given point
    //
    GE_DLLEXPIMPORT void          getPerpPlane   (const AcGePoint3d& pnt, AcGePlane& plane) const;

    // Definition of line.
    //
    GE_DLLEXPIMPORT AcGePoint3d    pointOnLine   () const;
    GE_DLLEXPIMPORT AcGeVector3d   direction     () const;
    GE_DLLEXPIMPORT void           getLine       (AcGeLine3d&) const;

    // Assignment operator.
    //
    GE_DLLEXPIMPORT AcGeLinearEnt3d& operator =  (const AcGeLinearEnt3d& line);

protected:
    GE_DLLEXPIMPORT AcGeLinearEnt3d ();
    GE_DLLEXPIMPORT AcGeLinearEnt3d (const AcGeLinearEnt3d&);
};

#pragma pack (pop)
#endif
