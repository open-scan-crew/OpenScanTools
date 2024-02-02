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
// This file contains the class AcGeBoundBlock3d - An entity used to 
// represent a 3d bounding volume, a parallelepiped.
//

#ifndef AC_GEBLOCK3D_H
#define AC_GEBLOCK3D_H

#include "geent3d.h"
#pragma pack (push, 8)
class AcGePoint3d;
class AcGeVector3d;

class 

AcGeBoundBlock3d : public AcGeEntity3d
{
public:
                    
	GE_DLLEXPIMPORT AcGeBoundBlock3d ();
	GE_DLLEXPIMPORT AcGeBoundBlock3d (const AcGePoint3d& base, const AcGeVector3d& dir1,
					  const AcGeVector3d& dir2, const AcGeVector3d& dir3);
	GE_DLLEXPIMPORT AcGeBoundBlock3d (const AcGeBoundBlock3d& block);
    
	// Access methods.
    //    
    GE_DLLEXPIMPORT void              getMinMaxPoints  (AcGePoint3d& point1,
								        AcGePoint3d& point2) const;
    GE_DLLEXPIMPORT void              get              (AcGePoint3d& base,
								        AcGeVector3d& dir1,
								        AcGeVector3d& dir2,
								        AcGeVector3d& dir3) const;
	// Set methods.
    //    
    GE_DLLEXPIMPORT AcGeBoundBlock3d& set              (const AcGePoint3d& point1,
								        const AcGePoint3d& point2);
    GE_DLLEXPIMPORT AcGeBoundBlock3d& set              (const AcGePoint3d& base,
								        const AcGeVector3d& dir1,
								        const AcGeVector3d& dir2,
								        const AcGeVector3d& dir3);
    // Expand to contain point.
    //
    GE_DLLEXPIMPORT AcGeBoundBlock3d& extend           (const AcGePoint3d& point);
   
	// Expand by a specified distance.
    //
    GE_DLLEXPIMPORT AcGeBoundBlock3d& swell            (double distance);

    // Containment and intersection tests
    //
    GE_DLLEXPIMPORT Adesk::Boolean    contains         (const AcGePoint3d& point) const;
    GE_DLLEXPIMPORT Adesk::Boolean    isDisjoint       (const AcGeBoundBlock3d& block) const;

    // Assignment opearator
    //
    GE_DLLEXPIMPORT AcGeBoundBlock3d& operator =       (const AcGeBoundBlock3d& block);

    GE_DLLEXPIMPORT Adesk::Boolean     isBox    () const;
    GE_DLLEXPIMPORT AcGeBoundBlock3d&  setToBox (Adesk::Boolean);
};


#pragma pack (pop)
#endif
