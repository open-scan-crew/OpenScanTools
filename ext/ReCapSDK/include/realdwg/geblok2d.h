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
// This file contains the class AcGeBoundBlock2d - An entity used to 
// represent a 2d bounding volume, a parallelogram.
//

#ifndef AC_GEBLOCK2D_H
#define AC_GEBLOCK2D_H

#include "geent2d.h"
#pragma pack (push, 8)
class AcGePoint2d;
class AcGeVector2d;

class 

AcGeBoundBlock2d : public AcGeEntity2d
{
public:
                    
	GE_DLLEXPIMPORT AcGeBoundBlock2d ();
	GE_DLLEXPIMPORT AcGeBoundBlock2d (const AcGePoint2d& point1, const AcGePoint2d& point2);
	GE_DLLEXPIMPORT AcGeBoundBlock2d (const AcGePoint2d& base,
                      const AcGeVector2d& dir1, const AcGeVector2d& dir2);
	GE_DLLEXPIMPORT AcGeBoundBlock2d (const AcGeBoundBlock2d& block);
    
	// Access methods.
    //    
    GE_DLLEXPIMPORT void              getMinMaxPoints  (AcGePoint2d& point1,
								        AcGePoint2d& point2) const;
    GE_DLLEXPIMPORT void              get              (AcGePoint2d& base,
								        AcGeVector2d& dir1,
								        AcGeVector2d& dir2) const;
    
	// Set methods.
    //    
    GE_DLLEXPIMPORT AcGeBoundBlock2d& set         (const AcGePoint2d& point1,
                                   const AcGePoint2d& point2);
    GE_DLLEXPIMPORT AcGeBoundBlock2d& set         (const AcGePoint2d& base,
                                   const AcGeVector2d& dir1,
                                   const AcGeVector2d& dir2);
    // Expand to contain point.
    //
    GE_DLLEXPIMPORT AcGeBoundBlock2d& extend      (const AcGePoint2d& point);
   
	// Expand by a specified distance.
    //
    GE_DLLEXPIMPORT AcGeBoundBlock2d& swell       (double distance);

    // Containment and intersection tests
    //
    GE_DLLEXPIMPORT Adesk::Boolean    contains    (const AcGePoint2d& point) const;
    GE_DLLEXPIMPORT Adesk::Boolean    isDisjoint  (const AcGeBoundBlock2d& block)
                                                 const;
    // Assignment operator
    //
    GE_DLLEXPIMPORT AcGeBoundBlock2d& operator =  (const AcGeBoundBlock2d& block);

	GE_DLLEXPIMPORT Adesk::Boolean    isBox     () const;
	GE_DLLEXPIMPORT AcGeBoundBlock2d& setToBox  (Adesk::Boolean);
};


#pragma pack (pop)
#endif
