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
// This file contains the class AcGePlanarEnt. It is an abstract class
// representing an oriented plane in 3-space.

#ifndef AC_GEPLANAR_H
#define AC_GEPLANAR_H

#include "gesurf.h"
#include "gevec3d.h"
#pragma pack (push, 8)

class AcGeLineSeg3d;
class AcGeLinearEnt3d;
class AcGeCircArc3d;

class

AcGePlanarEnt : public AcGeSurface
{
public:
    // Intersection
    //
    GE_DLLEXPIMPORT Adesk::Boolean  intersectWith    (const AcGeLinearEnt3d& linEnt, AcGePoint3d& pnt,
                                      const AcGeTol& tol = AcGeContext::gTol) const;
    // Closest point
    //
    GE_DLLEXPIMPORT AcGePoint3d     closestPointToLinearEnt (const AcGeLinearEnt3d& line,
                                             AcGePoint3d& pointOnLine,
                                             const AcGeTol& tol
                                               = AcGeContext::gTol) const;
    GE_DLLEXPIMPORT AcGePoint3d     closestPointToPlanarEnt (const AcGePlanarEnt& otherPln,
                                             AcGePoint3d& pointOnOtherPln,
                                             const AcGeTol& tol
                                               = AcGeContext::gTol) const;
    // Direction tests.
    //
    GE_DLLEXPIMPORT Adesk::Boolean isParallelTo      (const AcGeLinearEnt3d& linEnt,
                                      const AcGeTol& tol = AcGeContext::gTol) const;
    GE_DLLEXPIMPORT Adesk::Boolean isParallelTo      (const AcGePlanarEnt& otherPlnEnt,
                                      const AcGeTol& tol = AcGeContext::gTol) const;
    GE_DLLEXPIMPORT Adesk::Boolean isPerpendicularTo (const AcGeLinearEnt3d& linEnt,
                                      const AcGeTol& tol = AcGeContext::gTol) const;
    GE_DLLEXPIMPORT Adesk::Boolean isPerpendicularTo (const AcGePlanarEnt& linEnt,
                                      const AcGeTol& tol = AcGeContext::gTol) const;

    // Point set equality.
    //
    GE_DLLEXPIMPORT Adesk::Boolean isCoplanarTo      (const AcGePlanarEnt& otherPlnEnt,
                                      const AcGeTol& tol = AcGeContext::gTol) const;

    // Get methods.
    //
    GE_DLLEXPIMPORT void              get            (AcGePoint3d&, AcGeVector3d& uVec,
                                      AcGeVector3d& vVec) const;
    GE_DLLEXPIMPORT void              get            (AcGePoint3d&, AcGePoint3d& origin,
                                      AcGePoint3d&) const;

    // Geometric properties.
    //
    GE_DLLEXPIMPORT AcGePoint3d    pointOnPlane      () const;
    GE_DLLEXPIMPORT AcGeVector3d   normal            () const;
    GE_DLLEXPIMPORT void           getCoefficients(double& a, double& b, double& c, double& d) const;
    GE_DLLEXPIMPORT void           getCoordSystem(AcGePoint3d& origin, AcGeVector3d& axis1,
                                  AcGeVector3d& axis2) const;
    // Assignment operator.
    //
    GE_DLLEXPIMPORT AcGePlanarEnt& operator =        (const AcGePlanarEnt& src);

protected:
    GE_DLLEXPIMPORT AcGePlanarEnt ();
    GE_DLLEXPIMPORT AcGePlanarEnt (const AcGePlanarEnt&);
};

#pragma pack (pop)
#endif
