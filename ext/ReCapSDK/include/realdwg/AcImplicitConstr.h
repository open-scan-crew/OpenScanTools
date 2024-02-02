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
#include "AcConstrainedGeometry.h"
#include "AcGeomConstraint.h"
#pragma pack (push, 8)

/// <summary>
/// This class represents a Perpendicular constraint node in the owning AcDbAssoc2dConstraintGroup.
/// It can be applied between two lines.
/// </summary>
///
class  AcPerpendicularConstraint: public AcGeomConstraint 
{ 
public:
  ACRX_DECLARE_MEMBERS_EXPIMP(AcPerpendicularConstraint, ACDBCORE2D_PORT);

  /// <summary> default constructor. </summary>
  /// 
  ACDBCORE2D_PORT AcPerpendicularConstraint(bool bCreateImp = true);
};

/// <summary>
/// This class represents a Normal constraint node in the owning AcDbAssoc2dConstraintGroup.
/// Currently tt can only be applied between a line and circle(or arc).
/// </summary>
///
class  AcNormalConstraint: public AcGeomConstraint 
{ 
public:
  ACRX_DECLARE_MEMBERS_EXPIMP(AcNormalConstraint, ACDBCORE2D_PORT);

  /// <summary> default constructor. </summary>
  /// 
  ACDBCORE2D_PORT AcNormalConstraint(bool bCreateImp = true);
};

/// <summary>
/// This class represents a PointCurve (coincident) constraint node in the owning AcDbAssoc2dConstraintGroup.
/// It can be applied between a constrained point and a constrained curve.
/// </summary>
///
class  AcPointCurveConstraint: public AcGeomConstraint 
{ 
public:
  ACRX_DECLARE_MEMBERS_EXPIMP(AcPointCurveConstraint, ACDBCORE2D_PORT);

  /// <summary> default constructor. </summary>
  /// 
  ACDBCORE2D_PORT AcPointCurveConstraint(bool bCreateImp = true);
};

/// <summary>
/// This class represents a Colinear (coincident) constraint node in the owning AcDbAssoc2dConstraintGroup.
/// It can be applied between two constrained line.
/// </summary>
///
class  AcColinearConstraint: public AcGeomConstraint 
{ 
public:
  ACRX_DECLARE_MEMBERS_EXPIMP(AcColinearConstraint, ACDBCORE2D_PORT);

  /// <summary> default constructor. </summary>
  /// 
  ACDBCORE2D_PORT AcColinearConstraint(bool bCreateImp = true);
};

/// <summary>
/// This class represents a PointCoincidence (coincident) constraint node in the owning AcDbAssoc2dConstraintGroup.
/// It can be applied between two constrained point.
/// </summary>
///
class  AcPointCoincidenceConstraint: public AcGeomConstraint 
{ 
public:
  ACRX_DECLARE_MEMBERS_EXPIMP(AcPointCoincidenceConstraint, ACDBCORE2D_PORT);

  /// <summary> default constructor. </summary>
  /// 
  ACDBCORE2D_PORT AcPointCoincidenceConstraint(bool bCreateImp = true);
};

/// <summary>
/// This class represents a Concentric constraint node in the owning AcDbAssoc2dConstraintGroup.
/// It can be applied between two circles, arcs or ellipses.
/// </summary>
///
class  AcConcentricConstraint: public AcGeomConstraint 
{ 
public:
  ACRX_DECLARE_MEMBERS_EXPIMP(AcConcentricConstraint, ACDBCORE2D_PORT);

  /// <summary> default constructor. </summary>
  /// 
  ACDBCORE2D_PORT AcConcentricConstraint(bool bCreateImp = true);
};

/// <summary>
/// This class represents a Concentric constraint node in the owning AcDbAssoc2dConstraintGroup.
/// It can be applied between a point and a circle, arc or ellipse.
/// </summary>
///
class  AcCenterPointConstraint: public AcConcentricConstraint 
{ 
public:
  ACRX_DECLARE_MEMBERS_EXPIMP(AcCenterPointConstraint, ACDBCORE2D_PORT);
};

/// <summary>
/// This class represents a Tangent constraint node in the owning AcDbAssoc2dConstraintGroup.
/// It can be applied between two constrained curve.
/// </summary>
///
class  AcTangentConstraint : public AcGeomConstraint 
{ 
public:
  ACRX_DECLARE_MEMBERS_EXPIMP(AcTangentConstraint, ACDBCORE2D_PORT);

  /// <summary> default constructor. </summary>
  /// 
  ACDBCORE2D_PORT AcTangentConstraint(bool bCreateImp = true);
};

/// <summary>
/// This class represents a EqualRadius constraint node in the owning AcDbAssoc2dConstraintGroup.
/// It can be applied between two constrained circles (arcs).
/// </summary>
///
class  AcEqualRadiusConstraint: public AcGeomConstraint 
{ 
public:
  ACRX_DECLARE_MEMBERS_EXPIMP(AcEqualRadiusConstraint, ACDBCORE2D_PORT);

  /// <summary> default constructor. </summary>
  /// 
  ACDBCORE2D_PORT AcEqualRadiusConstraint(bool bCreateImp = true);
};

/// <summary>
/// This class represents a EqualDistance constraint node in the owning AcDbAssoc2dConstraintGroup.
/// It can be applied between two pairs of points.
/// </summary>
///
class  AcEqualDistanceConstraint: public AcGeomConstraint 
{ 
public:
  ACRX_DECLARE_MEMBERS_EXPIMP(AcEqualDistanceConstraint, ACDBCORE2D_PORT);

  /// <summary> default constructor. </summary>
  /// 
  ACDBCORE2D_PORT AcEqualDistanceConstraint(bool bCreateImp = true);
};

/// <summary>
/// This class represents a EqualLength constraint node in the owning AcDbAssoc2dConstraintGroup.
/// It can be applied between two constrained bounded lines (not rays).
/// </summary>
///
class  AcEqualLengthConstraint: public AcGeomConstraint
{ 
public:
  ACRX_DECLARE_MEMBERS_EXPIMP(AcEqualLengthConstraint, ACDBCORE2D_PORT);

  /// <summary> default constructor. </summary>
  /// 
  ACDBCORE2D_PORT AcEqualLengthConstraint(bool bCreateImp = true);
};

/// <summary>
/// This class represents a Parallel constraint node in the owning AcDbAssoc2dConstraintGroup.
/// It can be applied between two constrained lines.
/// </summary>
///
class  AcParallelConstraint: public AcGeomConstraint
{ 
public:
  ACRX_DECLARE_MEMBERS_EXPIMP(AcParallelConstraint, ACDBCORE2D_PORT);

  /// <summary> 
  /// Default constructor.
  /// </summary>
  /// <param name="bCreateImp">
  /// Input Boolean indicating whether the implementation object should be created.
  /// The default value is true.
  /// </param>
  ///
  ACDBCORE2D_PORT AcParallelConstraint(bool bCreateImp = true);
};

/// <summary>
/// This class represents a Horizontal constraint node in the owning AcDbAssoc2dConstraintGroup.
/// It can be applied on one constrained line.
/// </summary>
///
class  AcHorizontalConstraint: public AcParallelConstraint
{ 
public:
  ACRX_DECLARE_MEMBERS_EXPIMP(AcHorizontalConstraint, ACDBCORE2D_PORT);

  /// <summary> default constructor. </summary>
  /// 
  ACDBCORE2D_PORT AcHorizontalConstraint(bool bCreateImp = true);
};

/// <summary>
/// This class represents a Vertical constraint node in the owning AcDbAssoc2dConstraintGroup.
/// It can be applied on one constrained line.
/// </summary>
///
class  AcVerticalConstraint: public AcParallelConstraint
{ 
public:
  ACRX_DECLARE_MEMBERS_EXPIMP(AcVerticalConstraint, ACDBCORE2D_PORT);

  /// <summary> default constructor. </summary>
  /// 
  ACDBCORE2D_PORT AcVerticalConstraint(bool bCreateImp = true);
};

/// <summary>
/// This class represents a EqualCurvature constraint node in the owning AcDbAssoc2dConstraintGroup.
/// It can be applied between a bounded spline and a bounded curve.
/// </summary>
///
class  AcEqualCurvatureConstraint: public AcGeomConstraint
{ 
public:
  ACRX_DECLARE_MEMBERS_EXPIMP(AcEqualCurvatureConstraint, ACDBCORE2D_PORT);

  /// <summary> default constructor. </summary>
  /// 
  ACDBCORE2D_PORT AcEqualCurvatureConstraint(bool bCreateImp = true);
};


/// <summary>
/// This class represents a Symmetric constraint node in the owning AcDbAssoc2dConstraintGroup.
/// It can be applied between two same type of constrained geometries (except spline).
/// </summary>
///
class  AcSymmetricConstraint: public AcGeomConstraint
{ 
public:
  ACRX_DECLARE_MEMBERS_EXPIMP(AcSymmetricConstraint, ACDBCORE2D_PORT);

  /// <summary> default constructor. </summary>
  /// 
  ACDBCORE2D_PORT AcSymmetricConstraint(bool bCreateImp = true);
};

/// <summary>
/// This class represents a MidPoint constraint node in the owning AcDbAssoc2dConstraintGroup.
/// It can be applied between a point and a bounded line (not ray) or arc.
/// </summary>
///
class  AcMidPointConstraint: public AcGeomConstraint
{ 
public:
  ACRX_DECLARE_MEMBERS_EXPIMP(AcMidPointConstraint, ACDBCORE2D_PORT);

  /// <summary> default constructor. </summary>
  /// 
  ACDBCORE2D_PORT AcMidPointConstraint(bool bCreateImp = true);
};

/// <summary>
/// This class represents a Fixed constraint node in the owning AcDbAssoc2dConstraintGroup.
/// It can be applied on any constrained geometry.
/// </summary>
///
class  AcFixedConstraint : public AcGeomConstraint
{
public:
  ACRX_DECLARE_MEMBERS_EXPIMP(AcFixedConstraint, ACDBCORE2D_PORT);

  /// <summary> default constructor. </summary>
  /// 
  ACDBCORE2D_PORT AcFixedConstraint(bool bCreateImp = true);
};

/// <summary>
/// This class represents a EqualHelpParameter constraint node in the owning AcDbAssoc2dConstraintGroup.
/// It can be applied between two AcHelpParameter objects which belong to the same spline or ellipse.
/// </summary>
///
class  AcEqualHelpParameterConstraint : public AcGeomConstraint
{
public:
  ACRX_DECLARE_MEMBERS_EXPIMP(AcEqualHelpParameterConstraint, ACDBCORE2D_PORT);

  /// <summary> default constructor. </summary>
  /// 
  ACDBCORE2D_PORT AcEqualHelpParameterConstraint(bool bCreateImp = true);

  /// <summary> 
  /// Returns pointers to the two AcHelpParameter objects which this constraint is connected to.
  /// </summary>
  /// <param name="pHelpParameter1"> 
  /// The returned pointer to the first AcHelpParameter object.
  /// </param>
  /// <param name="pHelpParameter2"> 
  /// The returned pointer to the second AcHelpParameter object.
  /// </param>
  /// <returns> Returns Acad::eOk if successful. </returns>
  ///
  ACDBCORE2D_PORT Acad::ErrorStatus getEqualHelpParameters(AcHelpParameter*& pHelpParameter1, AcHelpParameter*& pHelpParameter2) const;
};

/// <summary>
/// This class represents a G2Smooth composite constraint node in the owning AcDbAssoc2dConstraintGroup.
/// It can be applied between a constrained bounded spline and other constrained bounded curve.
/// It is a combination of AcTangentConstraint and AcEqualCurvatureConstraint.
/// </summary>
///
class  AcG2SmoothConstraint : public AcCompositeConstraint
{
public:
  ACRX_DECLARE_MEMBERS_EXPIMP(AcG2SmoothConstraint, ACDBCORE2D_PORT);

  /// <summary> default constructor. </summary>
  /// 
  ACDBCORE2D_PORT AcG2SmoothConstraint(bool bCreateImp = true);
};

#pragma pack (pop)
