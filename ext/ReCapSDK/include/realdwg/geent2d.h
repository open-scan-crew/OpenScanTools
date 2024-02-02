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
// This file contains the class GeEntity2d - An abstract base class
// for all Geometry Library 2d entities.

#ifndef AC_GEENT2D_H
#define AC_GEENT2D_H

#include "gegbl.h"
#include "gepnt2d.h"
#include "geintrvl.h"
#include "gegblnew.h"
#pragma pack (push, 8)

class

AcGeEntity2d
{
public:
    GE_DLLEXPIMPORT ~AcGeEntity2d();

    // Run time type information.
    //
    GE_DLLEXPIMPORT Adesk::Boolean   isKindOf    (AcGe::EntityId entType) const;
    GE_DLLEXPIMPORT AcGe::EntityId   type        () const;

    // Make a copy of the entity.
    //
    GE_DLLEXPIMPORT AcGeEntity2d*    copy        () const;
    GE_DLLEXPIMPORT AcGeEntity2d&    operator =  (const AcGeEntity2d& entity);

    // Equivalence
    //
    GE_DLLEXPIMPORT Adesk::Boolean   operator == (const AcGeEntity2d& entity) const;
    GE_DLLEXPIMPORT Adesk::Boolean   operator != (const AcGeEntity2d& entity) const;
    GE_DLLEXPIMPORT Adesk::Boolean   isEqualTo   (const AcGeEntity2d& entity,
                                  const AcGeTol& tol = AcGeContext::gTol) const;
                                            
    // Matrix multiplication
    //
    GE_DLLEXPIMPORT AcGeEntity2d&    transformBy (const AcGeMatrix2d& xfm);
    GE_DLLEXPIMPORT AcGeEntity2d&    translateBy (const AcGeVector2d& translateVec);
    GE_DLLEXPIMPORT AcGeEntity2d&    rotateBy    (double angle, const AcGePoint2d& wrtPoint
                                  = AcGePoint2d::kOrigin);      
    GE_DLLEXPIMPORT AcGeEntity2d&    mirror      (const AcGeLine2d& line);
    GE_DLLEXPIMPORT AcGeEntity2d&    scaleBy     (double scaleFactor,
                                  const AcGePoint2d& wrtPoint
                                  = AcGePoint2d::kOrigin);
    // Point containment
    //
    GE_DLLEXPIMPORT Adesk::Boolean   isOn        (const AcGePoint2d& pnt,
                                  const AcGeTol& tol = AcGeContext::gTol) const;
protected:
    friend class AcGeEntity3d;
    friend class AcGeImpEntity3d;
    class AcGeImpEntity3d* mpImpEnt;
    int mDelEnt;
    GE_DLLEXPIMPORT AcGeEntity2d ();
    GE_DLLEXPIMPORT AcGeEntity2d (const AcGeEntity2d&);
    GE_DLLEXPIMPORT AcGeEntity2d (AcGeImpEntity3d&, int);
    GE_DLLEXPIMPORT AcGeEntity2d (AcGeImpEntity3d*);
    GE_DLLEXPIMPORT AcGeEntity2d*    newEntity2d (AcGeImpEntity3d*) const;
};


inline AcGeEntity2d*   
AcGeEntity2d::newEntity2d (AcGeImpEntity3d *impEnt ) const
{
    return GENEWLOC( AcGeEntity2d, this) ( impEnt );
}

#pragma pack (pop)
#endif
