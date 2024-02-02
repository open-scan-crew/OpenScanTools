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
// This file contains the class AcGeCompositeCurve3d - A mathematical
// entity used to represent a composite curve. Curve is parametrized 
// by Ravi-parametrization: that is n-th segment is parametrized by
// interval [ n, n + 1 ];

#ifndef AC_GECOMP3D_H
#define AC_GECOMP3D_H

#include "gecurv3d.h"
#include "gevptar.h"
#include "geintarr.h"
#pragma pack (push, 8)

class 

AcGeCompositeCurve3d : public AcGeCurve3d
{
public:
    GE_DLLEXPIMPORT AcGeCompositeCurve3d  ();
    GE_DLLEXPIMPORT AcGeCompositeCurve3d  (const AcGeVoidPointerArray& curveList);
    GE_DLLEXPIMPORT AcGeCompositeCurve3d  (const AcGeVoidPointerArray& curveList,
		                   const AcGeIntArray& isOwnerOfCurves);
    GE_DLLEXPIMPORT AcGeCompositeCurve3d  (const AcGeCompositeCurve3d& compCurve);

    // Definition of trimmed curve
    //
    GE_DLLEXPIMPORT void		          getCurveList       (AcGeVoidPointerArray& curveList) const;

    // Set methods
    //
    GE_DLLEXPIMPORT AcGeCompositeCurve3d& setCurveList       (const AcGeVoidPointerArray& curveList);
    GE_DLLEXPIMPORT AcGeCompositeCurve3d& setCurveList       (const AcGeVoidPointerArray& curveList,
		                                      const AcGeIntArray& isOwnerOfCurves);
	
	// Convert parameter on composite to parameter on component curve and vice-versa.
	//
	GE_DLLEXPIMPORT double				  globalToLocalParam ( double param, int& segNum ) const; 
	GE_DLLEXPIMPORT double				  localToGlobalParam ( double param, int segNum ) const; 

    // Assignment operator.
    //
    GE_DLLEXPIMPORT AcGeCompositeCurve3d& operator =         (const AcGeCompositeCurve3d& compCurve);
};

#pragma pack (pop)
#endif
