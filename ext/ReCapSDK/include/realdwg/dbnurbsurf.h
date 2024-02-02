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
//
// DESCRIPTION:
//
// The AcDbNurbSurface class is the interface class for representing
// ASM NURBS surfaces inside AutoCAD.  

#pragma once

#include "dbsurf.h"
class AcGeKnotVector;
class AcGeNurbSurface;

#pragma pack(push, 8)

class  AcDbNurbSurface: public AcDbSurface
{
public:

    ACDB_PORT AcDbNurbSurface();
    ACDB_PORT AcDbNurbSurface(int uDegree, int vDegree, bool rational,
                    int uNumControlPoints, int vNumControlPoints,
                    const AcGePoint3dArray& ctrlPts,
                    const AcGeDoubleArray& weights,
                    const AcGeKnotVector& uKnots, const AcGeKnotVector& vKnots );

    ACDB_PORT ~AcDbNurbSurface();
    ACDBCORE2D_DECLARE_MEMBERS(AcDbNurbSurface);

    //////////////////////////////////////////////////////////
    // NURBS surface methods
    //////////////////////////////////////////////////////////

    // get/set all data 
    ACDB_PORT Acad::ErrorStatus get (int& uDegree, int& vDegree, bool& rational,
                            int& uNumControlPoints, int& vNumControlPoints,
                            AcGePoint3dArray& ctrlPts, 
                            AcGeDoubleArray& weights,
                            AcGeKnotVector& uKnots, AcGeKnotVector& vKnots) const;
    ACDB_PORT Acad::ErrorStatus set (int uDegree, int vDegree, bool rational,
                            int uNumControlPoints, int vNumControlPoints,
                            const AcGePoint3dArray& ctrlPts,
                            const AcGeDoubleArray& weights,
                            const AcGeKnotVector& uKnots, const AcGeKnotVector& vKnots);    

    // get/set control points 
    ACDB_PORT Acad::ErrorStatus getControlPoints(int& uCount, int& vCount, AcGePoint3dArray& points) const;
    ACDB_PORT Acad::ErrorStatus setControlPoints(int uCount, int vCount, const AcGePoint3dArray& points);

    ACDB_PORT Acad::ErrorStatus getControlPointAt(int uIndex, int vIndex, AcGePoint3d& point) const;
    ACDB_PORT Acad::ErrorStatus setControlPointAt(int uIndex, int vIndex, const AcGePoint3d& point);

    // get the number of control points.
    ACDB_PORT Acad::ErrorStatus getNumberOfControlPointsInU(int& count) const;
    ACDB_PORT Acad::ErrorStatus getNumberOfControlPointsInV(int& count) const;

    // get knots
    ACDB_PORT Acad::ErrorStatus getUKnots(AcGeKnotVector& knots) const;
    ACDB_PORT Acad::ErrorStatus getVKnots(AcGeKnotVector& knots) const;

    // get the number of knots in u or v
    ACDB_PORT Acad::ErrorStatus getNumberOfKnotsInU(int& count) const;
    ACDB_PORT Acad::ErrorStatus getNumberOfKnotsInV(int& count) const;

    ACDB_PORT Acad::ErrorStatus getWeight(int uIndex, int vIndex, double& weight ) const;
    ACDB_PORT Acad::ErrorStatus setWeight(int uIndex, int vIndex, double weight );

    // Evaluate position, first and second derivatives
    ACDB_PORT Acad::ErrorStatus evaluate(double u, double v, AcGePoint3d& pos) const;
    ACDB_PORT Acad::ErrorStatus evaluate(double u, double v, AcGePoint3d& pos, AcGeVector3d& uDeriv, AcGeVector3d& vDeriv) const;
    ACDB_PORT Acad::ErrorStatus evaluate(double u, double v, AcGePoint3d& pos, AcGeVector3d& uDeriv, AcGeVector3d& vDeriv,
                                AcGeVector3d& uuDeriv, AcGeVector3d& uvDeriv, AcGeVector3d& vvDeriv) const;
    ACDB_PORT Acad::ErrorStatus evaluate(double u, double v, int derivDegree, AcGePoint3d& point, AcGeVector3dArray& derivatives) const;

    // get degree
    ACDB_PORT Acad::ErrorStatus getDegreeInU(int& degree) const;
    ACDB_PORT Acad::ErrorStatus getDegreeInV(int& degree) const;

    // is closed
    ACDB_PORT Acad::ErrorStatus isClosedInU(bool& isClosed) const;
    ACDB_PORT Acad::ErrorStatus isClosedInV(bool& isClosed) const;

    // is periodic
    ACDB_PORT Acad::ErrorStatus isPeriodicInU(bool& isPeriodic) const;
    ACDB_PORT Acad::ErrorStatus isPeriodicInV(bool& isPeriodic) const;

    // get period
    ACDB_PORT Acad::ErrorStatus getPeriodInU(double& period) const;
    ACDB_PORT Acad::ErrorStatus getPeriodInV(double& period) const;

    // test if rational 
    ACDB_PORT Acad::ErrorStatus isRational(bool& isRational) const;

    // test if planar
    ACDB_PORT Acad::ErrorStatus isPlanar(bool& isPlanar, AcGePoint3d& ptOnSurface, AcGeVector3d& normal) const;

    // test if a point is on the surface
    ACDB_PORT Acad::ErrorStatus isPointOnSurface(const AcGePoint3d& point, bool& onSurface) const;

    // get normal
    ACDB_PORT Acad::ErrorStatus getNormal(double u, double v, AcGeVector3d& normal) const;

    // get the simple patches in u v direction
    ACDB_PORT Acad::ErrorStatus getNumberOfSpansInU(int& span) const;
    ACDB_PORT Acad::ErrorStatus getNumberOfSpansInV(int& span) const;

    // get the u and v isolines.
    ACDB_PORT Acad::ErrorStatus getIsolineAtU(double u, AcArray<AcDbCurve*>& lineSegments) const;
    ACDB_PORT Acad::ErrorStatus getIsolineAtV(double v, AcArray<AcDbCurve*>& lineSegments) const;

    // knot insertion 
    ACDB_PORT Acad::ErrorStatus InsertKnotAtU(double u);
    ACDB_PORT Acad::ErrorStatus InsertKnotAtV(double v);

    // add / remove control points
    ACDB_PORT Acad::ErrorStatus InsertControlPointsAtU(double u, const AcGePoint3dArray& vCtrlPts, const AcGeDoubleArray& vWeights);
    ACDB_PORT Acad::ErrorStatus InsertControlPointsAtV(double v, const AcGePoint3dArray& uCtrlPts, const AcGeDoubleArray& uWeights);
    ACDB_PORT Acad::ErrorStatus RemoveControlPointsAtU(int uIndex);
    ACDB_PORT Acad::ErrorStatus RemoveControlPointsAtV(int vIndex);

    // rebuild
    ACDB_PORT Acad::ErrorStatus rebuild(int uDegree, int vDegree, int numUCtrlPts, int numVCtrlPts, bool bRestore = false);

    // adjust a point's location and tangent 
    ACDB_PORT Acad::ErrorStatus modifyPositionAndTangent(double u, double v, const AcGePoint3d& point,
                                 const AcGeVector3d* uDeriv = NULL, const AcGeVector3d* vDeriv = NULL);

    // get the u and v for a point
    ACDB_PORT Acad::ErrorStatus getParameterOfPoint(const AcGePoint3d& point, double& u, double& v) const;
   
    // Overridden methods from AcDbEntity
    ACDB_PORT void              dragStatus         (const AcDb::DragStat status) override;

    
protected:
    ACDB_PORT Acad::ErrorStatus subGetClassID(CLSID* pClsid) const override;
};

#pragma pack(pop)

