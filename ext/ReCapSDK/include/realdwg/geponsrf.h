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
// class AcGePointOnSurface to keep information of a point belonging
// to a surface.

#ifndef AC_GEPONSRF_H
#define AC_GEPONSRF_H

#include "gepent3d.h"
#pragma pack (push, 8)

class AcGeSurface;

class

AcGePointOnSurface : public AcGePointEnt3d
{
public:
    GE_DLLEXPIMPORT AcGePointOnSurface();
    GE_DLLEXPIMPORT AcGePointOnSurface(const AcGeSurface& surf);
    GE_DLLEXPIMPORT AcGePointOnSurface(const AcGeSurface& surf, const AcGePoint2d& param);
    GE_DLLEXPIMPORT AcGePointOnSurface(const AcGePointOnSurface& src);

    // Assignment operator.
    //
    GE_DLLEXPIMPORT AcGePointOnSurface& operator =     (const AcGePointOnSurface& src);

    // Query functions.
    //
    GE_DLLEXPIMPORT const AcGeSurface*  surface        () const;
    GE_DLLEXPIMPORT AcGePoint2d         parameter      () const;

    // Functions to evaluate a point.
    //
    GE_DLLEXPIMPORT AcGePoint3d         point          () const;
    GE_DLLEXPIMPORT AcGePoint3d         point          (const AcGePoint2d& param );
    GE_DLLEXPIMPORT AcGePoint3d         point          (const AcGeSurface& surf,
                                        const AcGePoint2d& param);

    // Functions to evaluate surface normal.
    //
    GE_DLLEXPIMPORT AcGeVector3d        normal         () const;
    GE_DLLEXPIMPORT AcGeVector3d        normal         (const AcGePoint2d& param );
    GE_DLLEXPIMPORT AcGeVector3d        normal         (const AcGeSurface& surf,
                                        const AcGePoint2d& param);
    // Functions to evaluate derivatives.
    //
    GE_DLLEXPIMPORT AcGeVector3d        uDeriv         (int order) const;
    GE_DLLEXPIMPORT AcGeVector3d        uDeriv         (int order, const AcGePoint2d& param);
    GE_DLLEXPIMPORT AcGeVector3d        uDeriv         (int order, const AcGeSurface& surf,
                                        const AcGePoint2d& param);

    GE_DLLEXPIMPORT AcGeVector3d        vDeriv         (int order) const;
    GE_DLLEXPIMPORT AcGeVector3d        vDeriv         (int order, const AcGePoint2d& param);
    GE_DLLEXPIMPORT AcGeVector3d        vDeriv         (int order, const AcGeSurface& surf,
                                        const AcGePoint2d& param);

    // Functions to evaluate the mixed partial.
    //
    GE_DLLEXPIMPORT AcGeVector3d        mixedPartial   () const;
    GE_DLLEXPIMPORT AcGeVector3d        mixedPartial   (const AcGePoint2d& param);
    GE_DLLEXPIMPORT AcGeVector3d        mixedPartial   (const AcGeSurface& surf,
                                        const AcGePoint2d& param);

    // Functions to compute the tangent vector in a given direction.
    //
    GE_DLLEXPIMPORT AcGeVector3d        tangentVector  (const AcGeVector2d& vec) const;
    GE_DLLEXPIMPORT AcGeVector3d        tangentVector  (const AcGeVector2d& vec,
                                        const AcGePoint2d& param);
    GE_DLLEXPIMPORT AcGeVector3d        tangentVector  (const AcGeVector2d& vec,
                                        const AcGeSurface& vecSurf,
                                        const AcGePoint2d& param);

    // Functions to invert a tangent vector to parameter space.
    //
    GE_DLLEXPIMPORT AcGeVector2d        inverseTangentVector  (const AcGeVector3d& vec) const;
    GE_DLLEXPIMPORT AcGeVector2d        inverseTangentVector  (const AcGeVector3d& vec,
                                               const AcGePoint2d& param);
    GE_DLLEXPIMPORT AcGeVector2d        inverseTangentVector  (const AcGeVector3d& vec,
                                               const AcGeSurface& surf,
                                               const AcGePoint2d& param);
    // Set functions.
    //
    GE_DLLEXPIMPORT AcGePointOnSurface& setSurface     (const AcGeSurface& surf);
    GE_DLLEXPIMPORT AcGePointOnSurface& setParameter   (const AcGePoint2d& param);
};

#pragma pack (pop)
#endif

