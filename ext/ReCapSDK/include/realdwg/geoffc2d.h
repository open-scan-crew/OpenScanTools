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
// This file contains the class AcGeOffsetCurve2d - A mathematical
// entity used to represent an exact offset of a 2d curve.

#ifndef AC_GEOFFC2D_H
#define AC_GEOFFC2D_H

#include "gecurv2d.h"
#pragma pack (push, 8)

class 

AcGeOffsetCurve2d : public AcGeCurve2d
{
public:

    // Constructors
    //
    GE_DLLEXPIMPORT AcGeOffsetCurve2d (const AcGeCurve2d& baseCurve, double offsetDistance);
    GE_DLLEXPIMPORT AcGeOffsetCurve2d (const AcGeOffsetCurve2d& offsetCurve);

	// Query methods
	//
    GE_DLLEXPIMPORT const AcGeCurve2d*  curve             () const;
    GE_DLLEXPIMPORT double              offsetDistance    () const;
	GE_DLLEXPIMPORT Adesk::Boolean		paramDirection    () const;
	GE_DLLEXPIMPORT AcGeMatrix2d		transformation    () const;

	// Set methods
	//
    GE_DLLEXPIMPORT AcGeOffsetCurve2d&  setCurve          (const AcGeCurve2d& baseCurve);
    GE_DLLEXPIMPORT AcGeOffsetCurve2d&  setOffsetDistance (double distance);

    // Assignment operator.
    //
    GE_DLLEXPIMPORT AcGeOffsetCurve2d&  operator = (const AcGeOffsetCurve2d& offsetCurve);
};

#pragma pack (pop)
#endif
