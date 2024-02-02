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
// This file contains the class AcGeNurbCurve3d - A mathematical entity
// used to represent a different types of spline curves in 3-space.

#ifndef AC_GENURB3D_H
#define AC_GENURB3D_H

#include "gecurv3d.h"
#include "geintrvl.h"
#include "gekvec.h"
#include "gept3dar.h"
#include "gevec3d.h"
#include "gepnt3d.h"
#include "gesent3d.h"
#include "geplin3d.h"
#include "gedblar.h"
#include "gept3dar.h"
#include "gevc3dar.h"
#pragma pack (push, 8)

class AcGeEllipArc3d;
class AcGeLineSeg3d;

class 

AcGeNurbCurve3d : public AcGeSplineEnt3d
{
public:
    // Construct spline from control points.
    //
    GE_DLLEXPIMPORT AcGeNurbCurve3d ();
    GE_DLLEXPIMPORT AcGeNurbCurve3d (const AcGeNurbCurve3d& src );
    GE_DLLEXPIMPORT AcGeNurbCurve3d (int degree, const AcGeKnotVector& knots,
                     const AcGePoint3dArray& cntrlPnts, 
                     Adesk::Boolean isPeriodic = Adesk::kFalse );
    GE_DLLEXPIMPORT AcGeNurbCurve3d (int degree, const AcGeKnotVector& knots,
                     const AcGePoint3dArray& cntrlPnts, 
                     const AcGeDoubleArray&  weights,
                     Adesk::Boolean isPeriodic = Adesk::kFalse );

    // Construct spline from interpolation data.
    //
    GE_DLLEXPIMPORT AcGeNurbCurve3d (int degree, const AcGePolyline3d& fitPolyline,
                     Adesk::Boolean isPeriodic = Adesk::kFalse );

    GE_DLLEXPIMPORT AcGeNurbCurve3d (const AcGePoint3dArray& fitPoints, 
                     const AcGeVector3d& startTangent, 
                     const AcGeVector3d& endTangent,
                     Adesk::Boolean startTangentDefined = Adesk::kTrue,
                     Adesk::Boolean endTangentDefined   = Adesk::kTrue,
                     const AcGeTol& fitTolerance = AcGeContext::gTol); 

    // specify the fitting points and the wanted knot parameterization
    GE_DLLEXPIMPORT AcGeNurbCurve3d (const AcGePoint3dArray& fitPoints, 
                     const AcGeVector3d& startTangent, 
                     const AcGeVector3d& endTangent,
                     Adesk::Boolean startTangentDefined,
                     Adesk::Boolean endTangentDefined,
                     AcGe::KnotParameterization knotParam,
                     const AcGeTol& fitTolerance = AcGeContext::gTol);

    GE_DLLEXPIMPORT AcGeNurbCurve3d (const AcGePoint3dArray& fitPoints, 
                     const AcGeTol& fitTolerance = AcGeContext::gTol);

    GE_DLLEXPIMPORT AcGeNurbCurve3d (const AcGePoint3dArray& fitPoints, 
                     const AcGeVector3dArray& fitTangents,
                     const AcGeTol& fitTolerance = AcGeContext::gTol,
                     Adesk::Boolean isPeriodic = Adesk::kFalse);   

    // Construct a cubic spline approximating the curve
    GE_DLLEXPIMPORT AcGeNurbCurve3d(const AcGeCurve3d& curve, 
                    double epsilon = AcGeContext::gTol.equalPoint());


    // Spline representation of ellipse
    //
    GE_DLLEXPIMPORT AcGeNurbCurve3d (const AcGeEllipArc3d&  ellipse);

    // Spline representation of line segment
    //
    GE_DLLEXPIMPORT AcGeNurbCurve3d (const AcGeLineSeg3d& linSeg);

    // Query methods.
    //
    GE_DLLEXPIMPORT int             numFitPoints      () const;
    GE_DLLEXPIMPORT Adesk::Boolean  getFitPointAt     (int index, AcGePoint3d& point) const;
    GE_DLLEXPIMPORT Adesk::Boolean  getFitTolerance   (AcGeTol& fitTolerance) const;
    GE_DLLEXPIMPORT Adesk::Boolean  getFitTangents    (AcGeVector3d& startTangent, 
                                       AcGeVector3d& endTangent) const;
    GE_DLLEXPIMPORT Adesk::Boolean  getFitTangents    (AcGeVector3d& startTangent, 
                                       AcGeVector3d& endTangent,
                                       Adesk::Boolean& startTangentDefined,
                                       Adesk::Boolean& endTangentDefined) const;
    GE_DLLEXPIMPORT Adesk::Boolean  getFitKnotParameterization(KnotParameterization& knotParam) const;
    GE_DLLEXPIMPORT Adesk::Boolean  getFitData        (AcGePoint3dArray& fitPoints,
                                       AcGeTol& fitTolerance,
                                       Adesk::Boolean& tangentsExist,
                                       AcGeVector3d& startTangent, 
                                       AcGeVector3d& endTangent) const;
    GE_DLLEXPIMPORT Adesk::Boolean  getFitData        (AcGePoint3dArray& fitPoints,
                                       AcGeTol& fitTolerance,
                                       Adesk::Boolean& tangentsExist,
                                       AcGeVector3d& startTangent, 
                                       AcGeVector3d& endTangent,
                                       KnotParameterization& knotParam) const;
    GE_DLLEXPIMPORT void            getDefinitionData (int& degree, Adesk::Boolean& rational,
                                       Adesk::Boolean& periodic,
                                       AcGeKnotVector& knots,
                                       AcGePoint3dArray& controlPoints,
                                       AcGeDoubleArray& weights) const;
    GE_DLLEXPIMPORT int             numWeights        () const;
    GE_DLLEXPIMPORT double          weightAt          (int idx) const;
    GE_DLLEXPIMPORT Adesk::Boolean  evalMode          () const;        
    GE_DLLEXPIMPORT Adesk::Boolean  getParamsOfC1Discontinuity (AcGeDoubleArray& params,
                                                const AcGeTol& tol 
                                                = AcGeContext::gTol) const;
    GE_DLLEXPIMPORT Adesk::Boolean  getParamsOfG1Discontinuity (AcGeDoubleArray& params,
                                                const AcGeTol& tol 
                                                = AcGeContext::gTol) const;

    // Modification methods.
    //
    GE_DLLEXPIMPORT Adesk::Boolean   setFitPointAt    (int index, const AcGePoint3d& point);
    GE_DLLEXPIMPORT Adesk::Boolean   addFitPointAt    (int index, const AcGePoint3d& point);
    GE_DLLEXPIMPORT Adesk::Boolean   deleteFitPointAt (int index);
    GE_DLLEXPIMPORT Adesk::Boolean   setFitTolerance  (const AcGeTol& fitTol=AcGeContext::gTol);
    GE_DLLEXPIMPORT Adesk::Boolean   setFitTangents   (const AcGeVector3d& startTangent, 
                                       const AcGeVector3d& endTangent);
    GE_DLLEXPIMPORT Adesk::Boolean   setFitTangents   (const AcGeVector3d& startTangent, 
                                       const AcGeVector3d& endTangent,
                                       Adesk::Boolean startTangentDefined,
                                       Adesk::Boolean endTangentDefined) const;
    GE_DLLEXPIMPORT Adesk::Boolean   setFitKnotParameterization(KnotParameterization knotParam);
    GE_DLLEXPIMPORT AcGeNurbCurve3d& setFitData       (const AcGePoint3dArray& fitPoints,                                             
                                       const AcGeVector3d& startTangent, 
                                       const AcGeVector3d& endTangent,
                                       const AcGeTol& fitTol=AcGeContext::gTol);
    GE_DLLEXPIMPORT AcGeNurbCurve3d& setFitData       (const AcGePoint3dArray& fitPoints,                                             
                                       const AcGeVector3d& startTangent, 
                                       const AcGeVector3d& endTangent,
                                       KnotParameterization knotParam,
                                       const AcGeTol& fitTol=AcGeContext::gTol);
    GE_DLLEXPIMPORT AcGeNurbCurve3d& setFitData       (const AcGeKnotVector& fitKnots,
                                       const AcGePoint3dArray& fitPoints,
                                       const AcGeVector3d& startTangent, 
                                       const AcGeVector3d& endTangent,                                         
                                       const AcGeTol& fitTol=AcGeContext::gTol,
                                       Adesk::Boolean isPeriodic=Adesk::kFalse);
    GE_DLLEXPIMPORT AcGeNurbCurve3d&  setFitData      (int degree, 
                                       const AcGePoint3dArray& fitPoints,
                                       const AcGeTol& fitTol=AcGeContext::gTol);
    GE_DLLEXPIMPORT Adesk::Boolean    purgeFitData    ();
    GE_DLLEXPIMPORT Adesk::Boolean    buildFitData    ();
    GE_DLLEXPIMPORT Adesk::Boolean    buildFitData    (KnotParameterization kp);
    GE_DLLEXPIMPORT AcGeNurbCurve3d&  addKnot         (double newKnot);
    GE_DLLEXPIMPORT AcGeNurbCurve3d&  insertKnot      (double newKnot);
    GE_DLLEXPIMPORT AcGeSplineEnt3d&  setWeightAt     (int idx, double val);
    GE_DLLEXPIMPORT AcGeNurbCurve3d&  setEvalMode     (Adesk::Boolean evalMode=Adesk::kFalse );
    GE_DLLEXPIMPORT AcGeNurbCurve3d&  joinWith        (const AcGeNurbCurve3d& curve);
    GE_DLLEXPIMPORT AcGeNurbCurve3d&  hardTrimByParams(double newStartParam, 
                                       double newEndParam);
    GE_DLLEXPIMPORT AcGeNurbCurve3d&  makeRational    (double weight = 1.0);
    GE_DLLEXPIMPORT AcGeNurbCurve3d&  makeClosed      ();
    GE_DLLEXPIMPORT AcGeNurbCurve3d&  makePeriodic    ();
    GE_DLLEXPIMPORT AcGeNurbCurve3d&  makeNonPeriodic ();
    GE_DLLEXPIMPORT AcGeNurbCurve3d&  makeOpen        ();
    GE_DLLEXPIMPORT AcGeNurbCurve3d&  elevateDegree   (int plusDegree);

    // add/remove control point.
    GE_DLLEXPIMPORT Adesk::Boolean    addControlPointAt(double newKnot, const AcGePoint3d& point, double weight = 1.0);
    GE_DLLEXPIMPORT Adesk::Boolean    deleteControlPointAt(int index);

    // Assignment operator.
    //
    GE_DLLEXPIMPORT AcGeNurbCurve3d&  operator =      (const AcGeNurbCurve3d& spline);
};

#pragma pack (pop)
#endif
