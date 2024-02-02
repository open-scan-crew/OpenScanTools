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
// Description: class  AcGeCurveCurveInt2d to hold data for intersectios
// of two 2d curves

#ifndef AC_GECINT2D_H
#define AC_GECINT2D_H

#include "adesk.h"
#include "gegbl.h"
#include "geent2d.h"
#include "geponc2d.h"
#include "geintrvl.h"
#pragma pack (push, 8)

class AcGeCurve2d;


class  

AcGeCurveCurveInt2d : public AcGeEntity2d
{

public:
    // Constructors.
    //
    GE_DLLEXPIMPORT AcGeCurveCurveInt2d ();
    GE_DLLEXPIMPORT AcGeCurveCurveInt2d (const AcGeCurve2d& curve1, const AcGeCurve2d& curve2,
                         const AcGeTol& tol = AcGeContext::gTol );
    GE_DLLEXPIMPORT AcGeCurveCurveInt2d (const AcGeCurve2d& curve1, const AcGeCurve2d& curve2,
                         const AcGeInterval& range1, const AcGeInterval& range2,
                         const AcGeTol& tol = AcGeContext::gTol);
    GE_DLLEXPIMPORT AcGeCurveCurveInt2d (const AcGeCurveCurveInt2d& src);

    // General query functions.
    //
    GE_DLLEXPIMPORT const AcGeCurve2d  *curve1          () const;
    GE_DLLEXPIMPORT const AcGeCurve2d  *curve2          () const;
    GE_DLLEXPIMPORT void               getIntRanges     (AcGeInterval& range1,
                                         AcGeInterval& range2) const;
    GE_DLLEXPIMPORT AcGeTol            tolerance        () const;

    // Intersection query methods.
    //
    GE_DLLEXPIMPORT int                numIntPoints     () const;
    GE_DLLEXPIMPORT AcGePoint2d        intPoint         (int intNum) const;
    GE_DLLEXPIMPORT void               getIntParams     (int intNum,
                                         double& param1, double& param2) const;
    GE_DLLEXPIMPORT void               getPointOnCurve1 (int intNum, AcGePointOnCurve2d&) const;
    GE_DLLEXPIMPORT void               getPointOnCurve2 (int intNum, AcGePointOnCurve2d&) const;
    GE_DLLEXPIMPORT void			   getIntConfigs    (int intNum, AcGe::AcGeXConfig& config1wrt2, 
                                         AcGe::AcGeXConfig& config2wrt1) const;
    GE_DLLEXPIMPORT Adesk::Boolean     isTangential     (int intNum) const;
    GE_DLLEXPIMPORT Adesk::Boolean     isTransversal    (int intNum) const;
    GE_DLLEXPIMPORT double             intPointTol      (int intNum) const;
    GE_DLLEXPIMPORT int                overlapCount     () const;
	GE_DLLEXPIMPORT Adesk::Boolean	   overlapDirection () const;
    GE_DLLEXPIMPORT void               getOverlapRanges (int overlapNum,
                                         AcGeInterval& range1,
                                         AcGeInterval& range2) const;

    // Curves change their places
    //
    GE_DLLEXPIMPORT void               changeCurveOrder (); 
        
    // Order with respect to parameter on the first/second curve.
    //
    GE_DLLEXPIMPORT AcGeCurveCurveInt2d& orderWrt1  ();    
    GE_DLLEXPIMPORT AcGeCurveCurveInt2d& orderWrt2  ();
    
	// Set functions.
    //
    GE_DLLEXPIMPORT AcGeCurveCurveInt2d& set        (const AcGeCurve2d& curve1,
                                     const AcGeCurve2d& curve2,
                                     const AcGeTol& tol = AcGeContext::gTol);
    GE_DLLEXPIMPORT AcGeCurveCurveInt2d& set        (const AcGeCurve2d& curve1,
                                     const AcGeCurve2d& curve2,
                                     const AcGeInterval& range1,
                                     const AcGeInterval& range2,
                                     const AcGeTol& tol = AcGeContext::gTol);

    // Assignment operator.
    //
    GE_DLLEXPIMPORT AcGeCurveCurveInt2d& operator = (const AcGeCurveCurveInt2d& src);
};

#pragma pack (pop)
#endif
