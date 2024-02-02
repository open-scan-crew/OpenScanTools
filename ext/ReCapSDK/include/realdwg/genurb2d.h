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
// This file contains the class AcGeNurbCurve2d - A mathematical entity
// used to represent a different types of spline curves in 2-space.

#ifndef AC_GENURB2d_H
#define AC_GENURB2d_H

#include "gecurv2d.h"
#include "geintrvl.h"
#include "gekvec.h"
#include "gept2dar.h"
#include "gevec2d.h"
#include "gepnt2d.h"
#include "gesent2d.h"
#include "geplin2d.h"
#include "gedblar.h"
#include "gept2dar.h"
#include "gevc2dar.h"
#pragma pack (push, 8)

class AcGeEllipArc2d;
class AcGeLineSeg2d;

class 

AcGeNurbCurve2d : public AcGeSplineEnt2d
{
public:
    // Construct spline from control points.
	//
    GE_DLLEXPIMPORT AcGeNurbCurve2d ();
    GE_DLLEXPIMPORT AcGeNurbCurve2d (const AcGeNurbCurve2d& src );
    GE_DLLEXPIMPORT AcGeNurbCurve2d (int degree, const AcGeKnotVector& knots,
                     const AcGePoint2dArray& cntrlPnts, 
                     Adesk::Boolean isPeriodic = Adesk::kFalse );
    GE_DLLEXPIMPORT AcGeNurbCurve2d (int degree, const AcGeKnotVector& knots,
                     const AcGePoint2dArray& cntrlPnts, 
                     const AcGeDoubleArray&  weights,
                     Adesk::Boolean isPeriodic = Adesk::kFalse );

    // Construct spline from interpolation data.
    //
    GE_DLLEXPIMPORT AcGeNurbCurve2d (int degree, const AcGePolyline2d& fitPolyline,
                     Adesk::Boolean isPeriodic = Adesk::kFalse );

    GE_DLLEXPIMPORT AcGeNurbCurve2d (const AcGePoint2dArray& fitPoints, 
				     const AcGeVector2d& startTangent, 
				     const AcGeVector2d& endTangent,
				     Adesk::Boolean startTangentDefined = Adesk::kTrue,
					 Adesk::Boolean endTangentDefined   = Adesk::kTrue,
				     const AcGeTol& fitTolerance = AcGeContext::gTol);

    // specify the fitting points and the wanted knot parameterization
    GE_DLLEXPIMPORT AcGeNurbCurve2d (const AcGePoint2dArray& fitPoints, 
				     const AcGeVector2d& startTangent, 
				     const AcGeVector2d& endTangent,
				     Adesk::Boolean startTangentDefined,
					 Adesk::Boolean endTangentDefined,
                     AcGe::KnotParameterization knotParam,
				     const AcGeTol& fitTolerance = AcGeContext::gTol);

    GE_DLLEXPIMPORT AcGeNurbCurve2d (const AcGePoint2dArray& fitPoints, 
				     const AcGeTol& fitTolerance = AcGeContext::gTol);

    GE_DLLEXPIMPORT AcGeNurbCurve2d (const AcGePoint2dArray& fitPoints, 
                     const AcGeVector2dArray& fitTangents,
				     const AcGeTol& fitTolerance = AcGeContext::gTol,
				     Adesk::Boolean isPeriodic = Adesk::kFalse);
    
    // Spline representation of ellipse
	//
	GE_DLLEXPIMPORT AcGeNurbCurve2d (const AcGeEllipArc2d&  ellipse);

    // Spline representation of line segment
	//
	GE_DLLEXPIMPORT AcGeNurbCurve2d (const AcGeLineSeg2d& linSeg);

    // Construct a cubic spline approximating the curve
    GE_DLLEXPIMPORT AcGeNurbCurve2d(const AcGeCurve2d& curve, 
                    double epsilon = AcGeContext::gTol.equalPoint());

	// Query methods.
	//
    GE_DLLEXPIMPORT int             numFitPoints      () const;
    GE_DLLEXPIMPORT Adesk::Boolean  getFitPointAt     (int index, AcGePoint2d& point) const;
    GE_DLLEXPIMPORT Adesk::Boolean  getFitTolerance   (AcGeTol& fitTolerance) const;
    GE_DLLEXPIMPORT Adesk::Boolean  getFitTangents    (AcGeVector2d& startTangent, 
				                       AcGeVector2d& endTangent) const;
    GE_DLLEXPIMPORT Adesk::Boolean  getFitKnotParameterization(KnotParameterization& knotParam) const;
    GE_DLLEXPIMPORT Adesk::Boolean  getFitData        (AcGePoint2dArray& fitPoints,
		                               AcGeTol& fitTolerance,
				                       Adesk::Boolean& tangentsExist,
				                       AcGeVector2d& startTangent, 
				                       AcGeVector2d& endTangent) const;
    GE_DLLEXPIMPORT Adesk::Boolean  getFitData        (AcGePoint2dArray& fitPoints,
		                               AcGeTol& fitTolerance,
				                       Adesk::Boolean& tangentsExist,
				                       AcGeVector2d& startTangent, 
				                       AcGeVector2d& endTangent,
                                       KnotParameterization& knotParam) const;
    GE_DLLEXPIMPORT void            getDefinitionData (int& degree, Adesk::Boolean& rational,
								       Adesk::Boolean& periodic,
			                           AcGeKnotVector& knots,
			                           AcGePoint2dArray& controlPoints,
			                           AcGeDoubleArray& weights) const;
    GE_DLLEXPIMPORT int             numWeights        () const;
    GE_DLLEXPIMPORT double          weightAt          (int idx) const;
    GE_DLLEXPIMPORT Adesk::Boolean  evalMode          () const;        
	GE_DLLEXPIMPORT Adesk::Boolean  getParamsOfC1Discontinuity (AcGeDoubleArray& params,
				                                const AcGeTol& tol 
					                            = AcGeContext::gTol) const;
	GE_DLLEXPIMPORT Adesk::Boolean	getParamsOfG1Discontinuity (AcGeDoubleArray& params,
					                            const AcGeTol& tol 
					                            = AcGeContext::gTol) const;

	// Modification methods.
	//
    GE_DLLEXPIMPORT Adesk::Boolean   setFitPointAt    (int index, const AcGePoint2d& point);
    GE_DLLEXPIMPORT Adesk::Boolean   addFitPointAt    (int index, const AcGePoint2d& point);
    GE_DLLEXPIMPORT Adesk::Boolean   deleteFitPointAt (int index);
    GE_DLLEXPIMPORT Adesk::Boolean   setFitTolerance  (const AcGeTol& fitTol=AcGeContext::gTol);
    GE_DLLEXPIMPORT Adesk::Boolean   setFitTangents   (const AcGeVector2d& startTangent, 
	                        	       const AcGeVector2d& endTangent);
    GE_DLLEXPIMPORT Adesk::Boolean   setFitKnotParameterization(KnotParameterization knotParam);
    GE_DLLEXPIMPORT AcGeNurbCurve2d& setFitData       (const AcGePoint2dArray& fitPoints,                                             
				                       const AcGeVector2d& startTangent, 
				                       const AcGeVector2d& endTangent,
				                       const AcGeTol& fitTol=AcGeContext::gTol);
    GE_DLLEXPIMPORT AcGeNurbCurve2d& setFitData       (const AcGePoint2dArray& fitPoints,                                             
				                       const AcGeVector2d& startTangent, 
				                       const AcGeVector2d& endTangent,
                                       KnotParameterization knotParam,
				                       const AcGeTol& fitTol=AcGeContext::gTol);
    GE_DLLEXPIMPORT AcGeNurbCurve2d& setFitData       (const AcGeKnotVector& fitKnots,
		                               const AcGePoint2dArray& fitPoints,
				                       const AcGeVector2d& startTangent, 
				                       const AcGeVector2d& endTangent,										 
                        			   const AcGeTol& fitTol=AcGeContext::gTol,
				                       Adesk::Boolean isPeriodic=Adesk::kFalse);
    GE_DLLEXPIMPORT AcGeNurbCurve2d&  setFitData      (int degree, 
                                       const AcGePoint2dArray& fitPoints,
				                       const AcGeTol& fitTol=AcGeContext::gTol);
    GE_DLLEXPIMPORT Adesk::Boolean    purgeFitData    ();
    GE_DLLEXPIMPORT Adesk::Boolean    buildFitData    ();
    GE_DLLEXPIMPORT Adesk::Boolean    buildFitData    (KnotParameterization kp);
    GE_DLLEXPIMPORT AcGeNurbCurve2d&  addKnot         (double newKnot);
    GE_DLLEXPIMPORT AcGeNurbCurve2d&  insertKnot      (double newKnot);
    GE_DLLEXPIMPORT AcGeSplineEnt2d&  setWeightAt     (int idx, double val);
    GE_DLLEXPIMPORT AcGeNurbCurve2d&  setEvalMode     (Adesk::Boolean evalMode=Adesk::kFalse );
	GE_DLLEXPIMPORT AcGeNurbCurve2d&  joinWith        (const AcGeNurbCurve2d& curve);
	GE_DLLEXPIMPORT AcGeNurbCurve2d&  hardTrimByParams(double newStartParam, 
		                               double newEndParam);
    GE_DLLEXPIMPORT AcGeNurbCurve2d&  makeRational    (double weight = 1.0);
    GE_DLLEXPIMPORT AcGeNurbCurve2d&  makeClosed      ();
    GE_DLLEXPIMPORT AcGeNurbCurve2d&  makePeriodic    ();
    GE_DLLEXPIMPORT AcGeNurbCurve2d&  makeNonPeriodic ();
    GE_DLLEXPIMPORT AcGeNurbCurve2d&  makeOpen        ();
    GE_DLLEXPIMPORT AcGeNurbCurve2d&  elevateDegree   (int plusDegree);

    // add/remove control point.
    GE_DLLEXPIMPORT Adesk::Boolean    addControlPointAt(double newKnot, const AcGePoint2d& point, double weight = 1.0);
    GE_DLLEXPIMPORT Adesk::Boolean    deleteControlPointAt(int index);

    // Assignment operator.
    //
    GE_DLLEXPIMPORT AcGeNurbCurve2d&  operator =      (const AcGeNurbCurve2d& spline);
};

#pragma pack (pop)
#endif
