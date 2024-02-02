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
// Name:        dbsubd.h
//
// Description: AcDbSubDMesh api class declaration
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "dbmain.h"
#include "gevc3dar.h"

#pragma pack(push, 8)

class AcDbSurface;
class AcGiFaceData;
class AcGiMapper;

///////////////////////////////////////////////////////////////////////////////
// class AcDbSubDMesh
//
class  AcDbSubDMesh: public AcDbEntity
{
    ACDB_DECLARE_MEMBERS_EXPIMP(AcDbSubDMesh, ACDB_PORT);

public:
    ACDB_PORT AcDbSubDMesh();
    ACDB_PORT ~AcDbSubDMesh();

    ///////////////////////////////////////////////////////////////////////////
    // Methods for AcDbSubDMesh
    ///////////////////////////////////////////////////////////////////////////
    //
    ACDB_PORT Acad::ErrorStatus           setSubDMesh             (const AcGePoint3dArray& vertexArray,
                                                         const AcArray<Adesk::Int32>& faceArray,
                                                         int subDLevel);

    ACDB_PORT Acad::ErrorStatus           setSphere               (double radius,
                                                         int divAxis,
                                                         int divHeight,
                                                         int subDLevel);

    ACDB_PORT Acad::ErrorStatus           setCylinder             (double majorRadius,
                                                         double minorRadius,
                                                         double height,
                                                         int divAxis,
                                                         int divHeight,
                                                         int divCap,
                                                         int subDLevel);

    ACDB_PORT Acad::ErrorStatus           setCone                 (double majorRadius,
                                                         double minorRadius,
                                                         double height,
                                                         int divAxis,
                                                         int divHeight,
                                                         int divCap,
                                                         double radiusRatio,
                                                         int subDLevel);

    ACDB_PORT Acad::ErrorStatus           setTorus                (double majorRadius,
                                                         int divSection,
                                                         int divSweepPath,
                                                         double sectionRadiusRatio,
                                                         double sectionRotate,
                                                         int subDLevel);

    ACDB_PORT Acad::ErrorStatus           setBox                  (double xLen,
                                                         double yLen,
                                                         double zLen,
                                                         int divX,
                                                         int divY,
                                                         int divZ,
                                                         int subDLevel);

    ACDB_PORT Acad::ErrorStatus           setWedge                (double xLen,
                                                         double yLen,
                                                         double zLen,
                                                         int divLength,
                                                         int divWidth,
                                                         int divHeight,
                                                         int divSlope,
                                                         int divCap,
                                                         int subDLevel);

    ACDB_PORT Acad::ErrorStatus           setPyramid              (double radius,
                                                         double height,
                                                         int divLength,
                                                         int divHeight,
                                                         int divCap,
                                                         int nSides,
                                                         double radiusRatio,
                                                         int subDLevel);

    ACDB_PORT Acad::ErrorStatus           subdDivideUp            ();
    ACDB_PORT Acad::ErrorStatus           subdDivideDown          ();
    ACDB_PORT Acad::ErrorStatus           subdRefine              ();
    ACDB_PORT Acad::ErrorStatus           subdRefine              (const AcDbFullSubentPathArray& subentPaths);
    ACDB_PORT Acad::ErrorStatus           subdLevel               (Adesk::Int32& result) const;

    ACDB_PORT Acad::ErrorStatus           splitFace               (const AcDbSubentId& subentFaceId,
                                                         const AcDbSubentId& subent0,
                                                         const AcGePoint3d& point0,
                                                         const AcDbSubentId& subent1,
                                                         const AcGePoint3d& point1);

    ACDB_PORT Acad::ErrorStatus           extrudeFaces            (const AcDbFullSubentPathArray& subentPaths,
                                                         double length,
                                                         const AcGeVector3d& dir,
                                                         double taper);

    ACDB_PORT Acad::ErrorStatus           extrudeFaces            (const AcDbFullSubentPathArray& subentPaths,
                                                         const AcGePoint3dArray& alongPath,
                                                         double taper);

    ACDB_PORT Acad::ErrorStatus           extrudeConnectedFaces   (const AcDbFullSubentPathArray& subentPaths,
                                                         double length,
                                                         const AcGeVector3d& dir,
                                                         double taper);

    ACDB_PORT Acad::ErrorStatus           extrudeConnectedFaces   (const AcDbFullSubentPathArray& subentPaths,
                                                         const AcGePoint3dArray& alongPath,
                                                         double taper);

    ACDB_PORT Acad::ErrorStatus           mergeFaces              (const AcDbFullSubentPathArray& subentPaths);
    ACDB_PORT Acad::ErrorStatus           collapse                (const AcDbSubentId& subent);
    ACDB_PORT Acad::ErrorStatus           cap                     (const AcDbFullSubentPathArray& edgePaths);
    ACDB_PORT Acad::ErrorStatus           spin                    (const AcDbSubentId& subent);

    ACDB_PORT Acad::ErrorStatus           isWatertight            (bool& result) const;

    ACDB_PORT Acad::ErrorStatus           numOfFaces              (Adesk::Int32& result) const;
    ACDB_PORT Acad::ErrorStatus           numOfSubDividedFaces    (Adesk::Int32& result) const;
    ACDB_PORT Acad::ErrorStatus           numOfSubDividedFacesAt  (const AcDbFullSubentPathArray& subentPaths, Adesk::Int32& result) const;
    ACDB_PORT Acad::ErrorStatus           numOfVertices           (Adesk::Int32& result) const;
    ACDB_PORT Acad::ErrorStatus           numOfSubDividedVertices (Adesk::Int32& result) const;
    ACDB_PORT Acad::ErrorStatus           numOfEdges              (Adesk::Int32& result) const;

    ACDB_PORT Acad::ErrorStatus           getVertices             (AcGePoint3dArray& vertexArray) const;
    ACDB_PORT Acad::ErrorStatus           getEdgeArray            (AcArray<Adesk::Int32>& edgeArray) const;
    ACDB_PORT Acad::ErrorStatus           getFaceArray            (AcArray<Adesk::Int32>& faceArray) const;
    ACDB_PORT Acad::ErrorStatus           getNormalArray          (AcGeVector3dArray& normalArray) const;

    ACDB_PORT Acad::ErrorStatus           getSubDividedVertices   (AcGePoint3dArray& vertexArray) const;
    ACDB_PORT Acad::ErrorStatus           getSubDividedFaceArray  (AcArray<Adesk::Int32>& faceArray) const;
    ACDB_PORT Acad::ErrorStatus           getSubDividedNormalArray(AcGeVector3dArray& normalArray) const;

    ACDB_PORT Acad::ErrorStatus           getVertexAt             (Adesk::Int32 nIndex, AcGePoint3d& vertex) const;
    ACDB_PORT Acad::ErrorStatus           setVertexAt             (Adesk::Int32 nIndex, const AcGePoint3d& vertex);
    ACDB_PORT Acad::ErrorStatus           getVertexAt             (const AcDbSubentId& id, AcGePoint3d& vertex) const;
    ACDB_PORT Acad::ErrorStatus           setVertexAt             (const AcDbSubentId& id, const AcGePoint3d& vertex);

    ACDB_PORT Acad::ErrorStatus           getSubDividedVertexAt   (Adesk::Int32 nIndex, AcGePoint3d& vertex) const;
    ACDB_PORT Acad::ErrorStatus           getSubDividedVertexAt   (const AcDbSubentId& id, AcGePoint3d& vertex) const;

    ACDB_PORT Acad::ErrorStatus           setCrease               (double creaseVal);
    ACDB_PORT Acad::ErrorStatus           setCrease               (const AcDbFullSubentPathArray& subentPaths, double creaseVal);
    ACDB_PORT Acad::ErrorStatus           getCrease               (const AcDbFullSubentPathArray& subentPaths, AcArray<double>& result) const;
    ACDB_PORT Acad::ErrorStatus           getCrease               (const AcDbSubentId& id, double& result) const;

    ACDB_PORT Acad::ErrorStatus           getAdjacentSubentPath   (const AcDbFullSubentPath& path,
                                                         AcDb::SubentType type,
                                                         AcDbFullSubentPathArray& subentPaths) const;

    ACDB_PORT Acad::ErrorStatus           getSubentPath           (Adesk::Int32 nIndex,
                                                         AcDb::SubentType type,
                                                         AcDbFullSubentPathArray& subentPaths) const;

    ACDB_PORT Acad::ErrorStatus           convertToSurface        (bool bConvertAsSmooth, const AcDbSubentId& id, AcDbSurface*& pSurface) const;
    ACDB_PORT Acad::ErrorStatus           convertToSurface        (bool bConvertAsSmooth, bool optimize, AcDbSurface*& pSurface) const;
    ACDB_PORT Acad::ErrorStatus           convertToSolid          (bool bConvertAsSmooth, bool optimize, AcDb3dSolid*& pSolid) const;

    ACDB_PORT Acad::ErrorStatus           getSubentColor          (const AcDbSubentId& id, AcCmColor& color) const;
    ACDB_PORT Acad::ErrorStatus           setSubentColor          (const AcDbSubentId& id, const AcCmColor& color);
    ACDB_PORT Acad::ErrorStatus           getSubentMaterial       (const AcDbSubentId& id, AcDbObjectId& material) const;
    ACDB_PORT Acad::ErrorStatus           setSubentMaterial       (const AcDbSubentId& id, const AcDbObjectId& material);
    ACDB_PORT Acad::ErrorStatus           getSubentMaterialMapper (const AcDbSubentId& id, AcGiMapper& mapper) const;
    ACDB_PORT Acad::ErrorStatus           setSubentMaterialMapper (const AcDbSubentId& id, const AcGiMapper& mapper);

    ACDB_PORT Acad::ErrorStatus           getFacePlane            (const AcDbSubentId& id, AcGePlane& facePlane) const;

    ACDB_PORT Acad::ErrorStatus           computeVolume           (double &retVolume) const;
    ACDB_PORT Acad::ErrorStatus           computeSurfaceArea      (double &retSurfArea) const;

    ///////////////////////////////////////////////////////////////////////////
    // Overridden methods from AcDbEntity
    ///////////////////////////////////////////////////////////////////////////
    //
    ACDB_PORT void                        dragStatus              (const AcDb::DragStat status) override;
    ACDB_PORT Acad::ErrorStatus           subGetClassID           (CLSID* pClsid) const override;

    ///////////////////////////////////////////////////////////////////////////
    // Overridden methods from AcGiDrawable
    ///////////////////////////////////////////////////////////////////////////
    //
    ACDB_PORT bool                        bounds                  ( AcDbExtents& retBounds ) const override;

    ///////////////////////////////////////////////////////////////////////////
    // Internal use
    ///////////////////////////////////////////////////////////////////////////
    //
    ACDB_PORT Acad::ErrorStatus           setSphere               (const AcGeMatrix3d& xForm,
                                                         int divAxis,
                                                         int divHeight,
                                                         int subDLevel);

    ACDB_PORT Acad::ErrorStatus           setCylinder             (const AcGeMatrix3d& xForm,
                                                         int divAxis,
                                                         int divHeight,
                                                         int divCap,
                                                         int subDLevel);

    ACDB_PORT Acad::ErrorStatus           setCone                 (const AcGeMatrix3d& xForm,
                                                         int divAxis,
                                                         int divHeight,
                                                         int divCap,
                                                         double radiusRatio,
                                                         int subDLevel);

    ACDB_PORT Acad::ErrorStatus           setTorus                (const AcGeMatrix3d& xForm,
                                                         int divSection,
                                                         int divSweepPath,
                                                         double sectionRadiusRatio,
                                                         double sectionRotate,
                                                         int subDLevel);

    ACDB_PORT Acad::ErrorStatus           setBox                  (const AcGeMatrix3d& xForm,
                                                         int divX,
                                                         int divY,
                                                         int divZ,
                                                         int subDLevel);

    ACDB_PORT Acad::ErrorStatus           setWedge                (const AcGeMatrix3d& xForm,
                                                         int divLength,
                                                         int divWidth,
                                                         int divHeight,
                                                         int divSlope,
                                                         int divCap,
                                                         int subDLevel);

    ACDB_PORT Acad::ErrorStatus           setPyramid              (const AcGeMatrix3d& xForm,
                                                         int divLength,
                                                         int divHeight,
                                                         int divCap,
                                                         int nSides,
                                                         double radiusRatio,
                                                         int subDLevel);

    ACDB_PORT Acad::ErrorStatus           computeRayIntersection  (const AcGePoint3d &rayStart,
                                                         const AcGeVector3d &rayDir,
                                                         AcArray<AcDbSubentId> &retSubents,
                                                         AcArray<double> &retIntersectDist,
                                                         AcGePoint3dArray& retIntersectPoint) const;

    ACDB_PORT Acad::ErrorStatus           setVertexNormalArray(AcGeVector3dArray &arr);
    ACDB_PORT Acad::ErrorStatus           setVertexTextureArray(AcGePoint3dArray &arr);
    ACDB_PORT Acad::ErrorStatus           setVertexColorArray(AcArray<AcCmEntityColor> &arr);

    ACDB_PORT Acad::ErrorStatus           getVertexNormalArray(AcGeVector3dArray &arr);
    ACDB_PORT Acad::ErrorStatus           getVertexTextureArray(AcGePoint3dArray &arr);
    ACDB_PORT Acad::ErrorStatus           getVertexColorArray(AcArray<AcCmEntityColor> &arr);
};

///////////////////////////////////////////////////////////////////////////
//  Global API functions 
///////////////////////////////////////////////////////////////////////////
//
struct MeshFaceterSettings
{
    double faceterDevSurface;
    double faceterDevNormal;
    double faceterGridRatio;
    double faceterMaxEdgeLength;
    Adesk::UInt16 faceterMaxGrid;
    Adesk::UInt16 faceterMinUGrid; // Unused
    Adesk::UInt16 faceterMinVGrid; // Unused
    Adesk::Int16 faceterMeshType;
};

typedef struct MeshFaceterSettings  AcDbFaceterSettings;

ACDB_PORT  Acad::ErrorStatus acdbGetObjectMesh(AcDbObject *pObj, 
                                               const AcDbFaceterSettings *faceter,
                                               AcGePoint3dArray& vertexArray, 
                                               AcArray<Adesk::Int32>& faceArray,
                                               AcGiFaceData*& faceData); 

#pragma pack(pop)
