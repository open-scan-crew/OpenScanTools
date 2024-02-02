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
// This file contains the class AcGeCircArc2d - A mathematical entity
// used to represent a circular arc in 2-space.

#ifndef AC_GEARC2D_H
#define AC_GEARC2D_H

#include "gecurv2d.h"
#include "gepnt2d.h"
#include "gevec2d.h"
#pragma pack (push, 8)

class AcGeLine2d;
class AcGeLinearEnt2d;

class

AcGeCircArc2d : public AcGeCurve2d
{
public:
    GE_DLLEXPIMPORT AcGeCircArc2d();
    GE_DLLEXPIMPORT AcGeCircArc2d(const AcGeCircArc2d& arc);
    GE_DLLEXPIMPORT AcGeCircArc2d(const AcGePoint2d& cent, double radius);
    GE_DLLEXPIMPORT AcGeCircArc2d(const AcGePoint2d& cent, double radius,
                  double startAngle, double endAngle,
                  const AcGeVector2d& refVec = AcGeVector2d::kXAxis,
                  Adesk::Boolean isClockWise = Adesk::kFalse);
    GE_DLLEXPIMPORT AcGeCircArc2d(const AcGePoint2d& startPoint, const AcGePoint2d& point, 
                  const AcGePoint2d& endPoint);

	// If bulgeFlag is kTrue, then bulge is interpreted to be the maximum
	// distance between the arc and the chord between the two input points.
	// If bulgeFlag is kFalse, then bulge is interpreted to be tan(ang/4),
	// where ang is the angle of the arc segment between the two input points.
    GE_DLLEXPIMPORT AcGeCircArc2d(const AcGePoint2d& startPoint, const AcGePoint2d& endPoint, double bulge, 
                  Adesk::Boolean bulgeFlag = Adesk::kTrue);


    // Intersection with other geometric objects.
    //
    GE_DLLEXPIMPORT Adesk::Boolean intersectWith  (const AcGeLinearEnt2d& line, int& intn,
                                   AcGePoint2d& p1, AcGePoint2d& p2,
                                   const AcGeTol& tol = AcGeContext::gTol) const;
    GE_DLLEXPIMPORT Adesk::Boolean intersectWith  (const AcGeCircArc2d& arc, int& intn,
                                   AcGePoint2d& p1, AcGePoint2d& p2,
                                   const AcGeTol& tol = AcGeContext::gTol) const;

    // Tangent line to the circular arc at given point.
    //
    GE_DLLEXPIMPORT Adesk::Boolean tangent        (const AcGePoint2d& pnt, AcGeLine2d& line,
                                   const AcGeTol& tol = AcGeContext::gTol) const;
    GE_DLLEXPIMPORT Adesk::Boolean tangent        (const AcGePoint2d& pnt, AcGeLine2d& line,
                                   const AcGeTol& tol, AcGeError& error) const;
		 // Possible error conditions:  kArg1TooBig, kArg1InsideThis, 
		 // kArg1OnThis
     

    // Test if point is inside circle.
    //
    GE_DLLEXPIMPORT Adesk::Boolean isInside       (const AcGePoint2d& pnt,
                                   const AcGeTol& tol = AcGeContext::gTol) const;

    // Definition of circular arc
    //
    GE_DLLEXPIMPORT AcGePoint2d    center         () const;
    GE_DLLEXPIMPORT double         radius         () const;
    GE_DLLEXPIMPORT double         startAng       () const;
    GE_DLLEXPIMPORT double         endAng         () const;
    GE_DLLEXPIMPORT Adesk::Boolean isClockWise    () const;
    GE_DLLEXPIMPORT AcGeVector2d   refVec         () const;
    GE_DLLEXPIMPORT AcGePoint2d    startPoint     () const;
    GE_DLLEXPIMPORT AcGePoint2d    endPoint       () const;

    GE_DLLEXPIMPORT AcGeCircArc2d& setCenter      (const AcGePoint2d& cent);
    GE_DLLEXPIMPORT AcGeCircArc2d& setRadius      (double radius);
    GE_DLLEXPIMPORT AcGeCircArc2d& setAngles      (double startAng, double endAng);
    GE_DLLEXPIMPORT AcGeCircArc2d& setToComplement();
    GE_DLLEXPIMPORT AcGeCircArc2d& setRefVec      (const AcGeVector2d& vec);
    GE_DLLEXPIMPORT AcGeCircArc2d& set            (const AcGePoint2d& cent, double radius);
    GE_DLLEXPIMPORT AcGeCircArc2d& set            (const AcGePoint2d& cent, double radius,
                                   double ang1, double ang2,
                                   const AcGeVector2d& refVec =
                                   AcGeVector2d::kXAxis,
                                   Adesk::Boolean isClockWise = Adesk::kFalse);
    GE_DLLEXPIMPORT AcGeCircArc2d& set            (const AcGePoint2d& startPoint, const AcGePoint2d& pnt,
                                   const AcGePoint2d& endPoint);
    GE_DLLEXPIMPORT AcGeCircArc2d& set            (const AcGePoint2d& startPoint, const AcGePoint2d& pnt,
                                   const AcGePoint2d& endPoint, AcGeError& error);
		 // Possible errors:  kEqualArg1Arg2, kEqualArg1Arg3, kEqualArg2Arg3, 
		 // kLinearlyDependentArg1Arg2Arg3.
		 // Degenerate results: none.
		 // On error, the object is unchanged.

	// If bulgeFlag is kTrue, then bulge is interpreted to be the maximum
	// distance between the arc and the chord between the two input points.
	// If bulgeFlag is kFalse, then bulge is interpreted to be tan(ang/4),
	// where ang is the angle of the arc segment between the two input points.
    GE_DLLEXPIMPORT AcGeCircArc2d& set            (const AcGePoint2d& startPoint, 
                                   const AcGePoint2d& endPoint,
                                   double bulge, Adesk::Boolean bulgeFlag = Adesk::kTrue);
    GE_DLLEXPIMPORT AcGeCircArc2d& set            (const AcGeCurve2d& curve1,
                                   const AcGeCurve2d& curve2,
                                   double radius, double& param1, double& param2,
								   Adesk::Boolean& success);
		// On success, this arc becomes the fillet of the given radius between the two curves,
	    // whose points of tangency are nearest param1 and param2 respectively.
    GE_DLLEXPIMPORT AcGeCircArc2d& set            (const AcGeCurve2d& curve1,
                                   const AcGeCurve2d& curve2,
                                   const AcGeCurve2d& curve3,
                                   double& param1, double& param2, double& param3,
								   Adesk::Boolean& success);
    // Assignment operator.
    //
    GE_DLLEXPIMPORT AcGeCircArc2d& operator =     (const AcGeCircArc2d& arc);
};

#pragma pack (pop)
#endif
