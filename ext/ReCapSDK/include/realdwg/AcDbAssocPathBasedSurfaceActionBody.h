//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2020 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include "AcDbAssocEdgeActionParam.h"
#include "AcDbAssocSurfaceActionBody.h"
#include "AcDbAssocVertexActionParam.h"
#pragma pack (push, 8)


/// <summary>
/// Base action body class for surface creation actions that take as input one 
/// or more paths. It just provides utility methods to get/set the 
/// AcDbAssocPathActionParams.
/// </summary>
///
class  AcDbAssocPathBasedSurfaceActionBody : public AcDbAssocSurfaceActionBody
{
public: 
    ACRX_DECLARE_MEMBERS_EXPIMP(AcDbAssocPathBasedSurfaceActionBody, ACDB_PORT);

    /// <summary> Default constructor. </summary>
    /// <param name="createImpObject"> See AcDbAssocCreateImpObject. </param>
    ///
    ACDB_PORT explicit AcDbAssocPathBasedSurfaceActionBody(AcDbAssocCreateImpObject createImpObject = kAcDbAssocCreateImpObject);

    /// <summary> Removes all AcDbAssocPathActionParams. </summary>
    ///
    ACDB_PORT Acad::ErrorStatus removeAllPathParams();

    /// <summary> Gets all AcDbAssocPathActionParams. </summary>
    ///
    ACDB_PORT Acad::ErrorStatus getInputPathParams(AcDbObjectIdArray& pathParamIds) const;

    /// <summary> 
    /// Gets the current geometry of all input paths. The outermost array is for
    /// every path, the second array is for the segments of each path, and the 
    /// inner-most array is for each segment of each path. Because the referenced
    /// entity might have changed, a single original edge in the original entity
    /// may generally correspond to any number of edges in the current entity, or 
    /// to no edge at all (though, it most cases it will be just a single edge).
    /// </summary>
    ///
    ACDB_PORT Acad::ErrorStatus getInputPaths(AcArray<AcArray<AcArray<AcDbEdgeRef> > >&) const;

    /// <summary> 
    /// Sets all input paths. Each path is specified by an array of AcDbEdgeRefs.
    /// </summary>
    ///
    ACDB_PORT Acad::ErrorStatus setInputPaths(const AcArray<AcArray<AcDbEdgeRef> >&);

    /// <summary> Sets all input paths. </summary>
    ///
    ACDB_PORT Acad::ErrorStatus setInputPaths(const AcArray<AcDbPathRef>&);

    /// <summary> 
    /// Changes the single input path specified by its pathIndex. The path with
    /// the given index must already exist.
    /// </summary>
    ///
    ACDB_PORT Acad::ErrorStatus updateInputPath(int pathIndex, const AcDbPathRef&);

    /// <summary> Removes all AcDbAssocVertexActionParams. </summary>
    ///
    ACDB_PORT Acad::ErrorStatus removeAllVertexParams();

    /// <summary> Gets all AcDbAssocVertexActionParams. </summary>
    ///
    ACDB_PORT Acad::ErrorStatus getInputVertexParams(AcDbObjectIdArray& vertexParamIds) const;

    /// <summary> 
    /// Sets all input points. Points can sometimes be used instead of profile
    /// paths.
    /// </summary>
    ///
    ACDB_PORT Acad::ErrorStatus setInputPoints(const AcArray<AcDbVertexRef>&);

    /// <summary> Gets all input points. </summary>
    ///
    ACDB_PORT Acad::ErrorStatus getInputPoints(AcArray<AcDbVertexRef>&) const;

    /// <summary>
    /// This method is to simplify the user selection of the underlying path 
    /// geometry that is the input for the action. The resulting surface usually 
    /// overlaps the input path geometry and it would be difficult for the user 
    /// to select the geometry that is the input to the action. Calling this method 
    /// makes the resutling surface be drawn under the input paths so that 
    /// the input paths are easier to select by the user.
    /// </summary>
    ///
    ACDB_PORT Acad::ErrorStatus makeInputPathsDrawOnTopOfResultingSurface() const;

}; // class AcDbAssocPathBasedSurfaceActionBody

#pragma pack (pop)

