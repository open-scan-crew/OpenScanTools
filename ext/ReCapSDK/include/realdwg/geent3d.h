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
// This file contains the class GeEntity3d - An abstract base class
// for all Geometry Library 3d entities.

#ifndef AC_GEENT3D_H
#define AC_GEENT3D_H

#include "gegbl.h"
#include "gepnt3d.h"
#include "geent2d.h"
#include "geintrvl.h"
#include "gegblnew.h"
#pragma pack (push, 8)

class

AcGeEntity3d
{
public:
    GE_DLLEXPIMPORT ~AcGeEntity3d();

    // Run time type information.
    //
    GE_DLLEXPIMPORT Adesk::Boolean   isKindOf    (AcGe::EntityId entType) const;
    GE_DLLEXPIMPORT AcGe::EntityId   type        () const;

    // Make a copy of the entity.
    //
    GE_DLLEXPIMPORT AcGeEntity3d*    copy        () const;
    GE_DLLEXPIMPORT AcGeEntity3d&    operator =  (const AcGeEntity3d& entity);

    // Equivalence
    //
    GE_DLLEXPIMPORT Adesk::Boolean   operator == (const AcGeEntity3d& entity) const;
    GE_DLLEXPIMPORT Adesk::Boolean   operator != (const AcGeEntity3d& entity) const;
    GE_DLLEXPIMPORT Adesk::Boolean   isEqualTo   (const AcGeEntity3d& ent,
                                  const AcGeTol& tol = AcGeContext::gTol) const;
    // Matrix multiplication
    //
    GE_DLLEXPIMPORT AcGeEntity3d&    transformBy (const AcGeMatrix3d& xfm);
    GE_DLLEXPIMPORT AcGeEntity3d&    translateBy (const AcGeVector3d& translateVec);
    GE_DLLEXPIMPORT AcGeEntity3d&    rotateBy    (double angle, const AcGeVector3d& vec,
                                  const AcGePoint3d& wrtPoint = AcGePoint3d::kOrigin);
    GE_DLLEXPIMPORT AcGeEntity3d&    mirror      (const AcGePlane& plane);
    GE_DLLEXPIMPORT AcGeEntity3d&    scaleBy     (double scaleFactor,
                                  const AcGePoint3d& wrtPoint
                                  = AcGePoint3d::kOrigin);
    // Point containment
    //
    GE_DLLEXPIMPORT Adesk::Boolean   isOn        (const AcGePoint3d& pnt,
                                  const AcGeTol& tol = AcGeContext::gTol) const;
protected:
    friend class AcGeImpEntity3d;
    class AcGeImpEntity3d  *mpImpEnt;
    int              mDelEnt;
    GE_DLLEXPIMPORT AcGeEntity3d ();
    GE_DLLEXPIMPORT AcGeEntity3d (const AcGeEntity3d&);
    GE_DLLEXPIMPORT AcGeEntity3d (AcGeImpEntity3d&, int);
    GE_DLLEXPIMPORT AcGeEntity3d (AcGeImpEntity3d*);
    GE_DLLEXPIMPORT AcGeEntity2d* newEntity2d (AcGeImpEntity3d*) const;
    GE_DLLEXPIMPORT AcGeEntity2d* newEntity2d (AcGeImpEntity3d&, int) const;
    GE_DLLEXPIMPORT AcGeEntity3d* newEntity3d (AcGeImpEntity3d*) const;
    GE_DLLEXPIMPORT AcGeEntity3d* newEntity3d (AcGeImpEntity3d&, int) const;
};

inline AcGeEntity2d*
AcGeEntity3d::newEntity2d (AcGeImpEntity3d *impEnt ) const
{
    return GENEWLOC( AcGeEntity2d, this) ( impEnt );
}

inline AcGeEntity3d*
AcGeEntity3d::newEntity3d (AcGeImpEntity3d *impEnt ) const
{
    return GENEWLOC( AcGeEntity3d, this) ( impEnt );
}

inline AcGeEntity3d*
AcGeEntity3d::newEntity3d(AcGeImpEntity3d& impEnt, int dummy) const
{
    return GENEWLOC( AcGeEntity3d, this)(impEnt, dummy);
}

inline AcGeEntity2d*
AcGeEntity3d::newEntity2d(AcGeImpEntity3d& impEnt, int dummy) const
{
    return GENEWLOC( AcGeEntity2d, this)(impEnt, dummy);
}

#pragma pack (pop)
#endif
