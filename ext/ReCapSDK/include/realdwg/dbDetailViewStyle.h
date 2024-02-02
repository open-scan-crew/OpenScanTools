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


#ifndef __ACDBDETAILVIEWSTYLE_H__
#define __ACDBDETAILVIEWSTYLE_H__

#pragma once
#include "dbModelDocViewStyle.h"

class  AcDbDetailViewStyle : public AcDbModelDocViewStyle
{
public:
	/// <summary>
	/// Enumeration defining label and identifier placement
	/// </summary>
	enum IdentifierPlacement {
		kOutsideBoundary=0,
        kOutsideBoundaryWithLeader,
        kOnBoundary,
        kOnBoundaryWithLeader,
	};

    enum ModelEdge {
        kSmooth = 0,
        kSmoothWithBorder,
        kSmoothWithConnectionLine,
        kJagged,
    };


    ACDBCORE2D_PORT AcDbDetailViewStyle();
    ACDBCORE2D_PORT ~AcDbDetailViewStyle();
    ACDB_DECLARE_MEMBERS_EXPIMP(AcDbDetailViewStyle, ACDBCORE2D_PORT);

    /// <summary>
    /// Returns the text style objectId of the identifier.
    /// </summary>
    ///
    /// <returns>
    /// Returns the text style objectId of the identifier.
    /// </returns>
    ACDBCORE2D_PORT AcDbObjectId      identifierStyleId () const;

    /// <summary>
    /// Sets the identifier to use specified text style.
    /// </summary>
    ///
    /// <param name="objId">
    /// ObjectId of text style to use for the identifier.
    /// </param>
    ///
    /// <returns>
    /// Return Acad::eOk if Successful
    /// </returns>
    ACDBCORE2D_PORT Acad::ErrorStatus setIdentifierStyleId (const AcDbObjectId &objId);

    /// <summary>
    /// Returns the color of the identifier.
    /// </summary>
    ///
    /// <returns>
    /// Returns the color of the identifier.
    /// </returns>
    ACDBCORE2D_PORT AcCmColor         identifierColor () const;

    /// <summary>
    /// Sets the identifier to use specified color.
    /// </summary>
    ///
    /// <param name="color">
    /// Color to use for the identifier.
    /// </param>
    ///
    /// <returns>
    /// Return Acad::eOk if Successful
    /// </returns>
    ACDBCORE2D_PORT Acad::ErrorStatus setIdentifierColor (const AcCmColor& color);

    /// <summary>
    /// Returns the height of the identifier.
    /// </summary>
    ///
    /// <returns>
    /// Returns the height of the identifier.
    /// </returns>
    ACDBCORE2D_PORT double            identifierHeight () const;

    /// <summary>
    /// Sets the text height the identifier.
    /// </summary>
    ///
    /// <param name="height">
    /// Text height to use for the identifier.
    /// </param>
    ///
    /// <returns>
    /// Return Acad::eOk if Successful
    /// </returns>
    ACDBCORE2D_PORT Acad::ErrorStatus setIdentifierHeight (double height);

    /// <summary>
    /// Returns the offset between arrow line and extension line.
    /// The value is relative in range &lt0, 1&gt, where 0 means that
    /// arrow touches extension line at beginning and 1.0 means
    /// means that arrow touches the extension line at the end.
    /// </summary>
    ///
    /// <returns>
    /// Returns the offset between arrow line and extension line in
    /// relative form.
    /// </returns>
    ACDBCORE2D_PORT double identifierOffset () const;

    /// <summary>
    /// Sets the offset between arrow line and extension line.
    /// The value is relative in range &lt0, 1&gt, where 0 means that
    /// arrow touches extension line at beginning and 1.0 means
    /// means that arrow touches the extension line at the end.
    /// </summary>
    ///
    /// <returns>
    /// Sets the offset between arrow line and extension line in relative
    /// form.
    /// </returns>
    ACDBCORE2D_PORT Acad::ErrorStatus setIdentifierOffset (double offset);

    /// <summary>
    /// Returns the label and identifier placement flags.
    /// </summary>
    ///
    /// <returns>
    /// Returns the bit coded flags defining label and identifier placement.
    /// </returns>
    ACDBCORE2D_PORT AcDbDetailViewStyle::IdentifierPlacement identifierPlacement () const;

    /// <summary>
    /// Sets the label and identifier placement flags.
    /// Use bit coded flags defined by identifierPlacement enumeration.
    /// </summary>
    ///
    /// <param name="placement">
    /// Bit coded flags defining label and identifier placement.
    /// </param>
    ///
    /// <returns>
    /// Return Acad::eOk if Successful
    /// </returns>
    ACDBCORE2D_PORT Acad::ErrorStatus setIdentifierPlacement (AcDbDetailViewStyle::IdentifierPlacement placement);

    /// <summary>
    /// Returns the objectId of arrow start symbol.
    /// </summary>
    ///
    /// <returns>
    /// Returns the objectId of arrow start symbol.
    /// </returns>
    ACDBCORE2D_PORT AcDbObjectId      arrowSymbolId () const;

    /// <summary>
    /// Sets the arrow start symbol to use specified block table record.
    /// </summary>
    ///
    /// <param name="objId">
    /// ObjectId of block table record to use for arrow start symbol
    /// </param>
    ///
    /// <returns>
    /// Return Acad::eOk if Successful
    /// </returns>
    ACDBCORE2D_PORT Acad::ErrorStatus setArrowSymbolId (const AcDbObjectId &arrowSymbolId);

    /// <summary>
    /// Returns the color of arrow symbol.
    /// </summary>
    ///
    /// <returns>
    /// Returns the color of arrow symbol.
    /// </returns>
    ACDBCORE2D_PORT AcCmColor         arrowSymbolColor () const;

    /// <summary>
    /// Sets the arrow symbol to use specified color.
    /// </summary>
    ///
    /// <param name="color">
    /// Color to use for arrow symbol
    /// </param>
    ///
    /// <returns>
    /// Return Acad::eOk if Successful
    /// </returns>
    ACDBCORE2D_PORT Acad::ErrorStatus setArrowSymbolColor (const AcCmColor& color);

    /// <summary>
    /// Returns the size of arrow symbol.
    /// </summary>
    ///
    /// <returns>
    /// Returns the size of arrow symbol.
    /// </returns>
    ACDBCORE2D_PORT double            arrowSymbolSize () const;

    /// <summary>
    /// Sets the size of arrow symbol.
    /// </summary>
    ///
    /// <param name="size">
    /// Size of arrow symbol
    /// </param>
    ///
    /// <returns>
    /// Return Acad::eOk if Successful
    /// </returns>
    ACDBCORE2D_PORT Acad::ErrorStatus setArrowSymbolSize (double size);


    /// <summary>
    /// Returns boolean value indicating if to show arrowheads.
    /// </summary>
    ///
    /// <returns>
    /// true if to show arrowheads is on, false otherwise.
    /// </returns>
    ACDBCORE2D_PORT bool              showArrows () const;
    ACDBCORE2D_PORT Acad::ErrorStatus setShowArrows (bool bValue);

    /// <summary>
    /// Returns the line weight of Boundary line.
    /// </summary>
    ///
    /// <returns>
    /// Returns the line weight of Boundary line.
    /// </returns>
    ACDBCORE2D_PORT AcDb::LineWeight  boundaryLineWeight () const;

    /// <summary>
    /// Sets the Boundary line to use specified line weight.
    /// </summary>
    ///
    /// <param name="color">
    /// Line weight to use.for Boundary.line
    /// </param>
    ///
    /// <returns>
    /// Return Acad::eOk if Successful
    /// </returns>
    ACDBCORE2D_PORT Acad::ErrorStatus setBoundaryLineWeight (AcDb::LineWeight lineweight);

    /// <summary>
    /// Returns the color of Boundary line.
    /// </summary>
    ///
    /// <returns>
    /// Returns the color of Boundary line.
    /// </returns>
    ACDBCORE2D_PORT AcCmColor         boundaryLineColor () const;

    /// <summary>
    /// Sets the Boundary line to use specified color.
    /// </summary>
    ///
    /// <param name="color">
    /// Color to use for Boundary.line
    /// </param>
    ///
    /// <returns>
    /// Return Acad::eOk if Successful
    /// </returns>
    ACDBCORE2D_PORT Acad::ErrorStatus setBoundaryLineColor (const AcCmColor& color);

    /// <summary>
    /// Returns the linetype objectId of Boundary line.
    /// </summary>
    ///
    /// <returns>
    /// Returns the linetype objectId of Boundary line.
    /// </returns>
    ACDBCORE2D_PORT AcDbObjectId      boundaryLineTypeId () const;

    /// <summary>
    /// Sets the Boundary line to use specified linetype.
    /// </summary>
    ///
    /// <param name="objId">
    /// ObjectId of the linetype to use.for Boundary.line
    /// </param>
    ///
    /// <returns>
    /// Return Acad::eOk if Successful
    /// </returns>
    ACDBCORE2D_PORT Acad::ErrorStatus setBoundaryLineTypeId (const AcDbObjectId &objId);

    /// <summary>
    /// Returns the line weight of Connection line.
    /// </summary>
    ///
    /// <returns>
    /// Returns the line weight of Connection line.
    /// </returns>
    ACDBCORE2D_PORT AcDb::LineWeight  connectionLineWeight () const;

    /// <summary>
    /// Sets the Connection line to use specified line weight.
    /// </summary>
    ///
    /// <param name="color">
    /// Line weight to use.for Connection.line
    /// </param>
    ///
    /// <returns>
    /// Return Acad::eOk if Successful
    /// </returns>
    ACDBCORE2D_PORT Acad::ErrorStatus setConnectionLineWeight (AcDb::LineWeight lineweight);

    /// <summary>
    /// Returns the color of Connection line.
    /// </summary>
    ///
    /// <returns>
    /// Returns the color of Connection line.
    /// </returns>
    ACDBCORE2D_PORT AcCmColor         connectionLineColor () const;

    /// <summary>
    /// Sets the Connection line to use specified color.
    /// </summary>
    ///
    /// <param name="color">
    /// Color to use for Connection.line
    /// </param>
    ///
    /// <returns>
    /// Return Acad::eOk if Successful
    /// </returns>
    ACDBCORE2D_PORT Acad::ErrorStatus setConnectionLineColor (const AcCmColor& color);

    /// <summary>
    /// Returns the linetype objectId of Connection line.
    /// </summary>
    ///
    /// <returns>
    /// Returns the linetype objectId of Connection line.
    /// </returns>
    ACDBCORE2D_PORT AcDbObjectId      connectionLineTypeId () const;

    /// <summary>
    /// Sets the Connection line to use specified linetype.
    /// </summary>
    ///
    /// <param name="objId">
    /// ObjectId of the linetype to use.for Connection.line
    /// </param>
    ///
    /// <returns>
    /// Return Acad::eOk if Successful
    /// </returns>
    ACDBCORE2D_PORT Acad::ErrorStatus setConnectionLineTypeId (const AcDbObjectId &objId);

    /// <summary>
    /// Returns the text style objectId of view label.
    /// </summary>
    ///
    /// <returns>
    /// Returns the text style objectId of view label.
    /// </returns>
    ACDBCORE2D_PORT AcDbObjectId      viewLabelTextStyleId () const;

    /// <summary>
    /// Sets the text style of the view label text.
    /// </summary>
    ///
    /// <param name="objId">
    /// length of end and bend line.
    /// </param>
    ///
    /// <returns>
    /// Return Acad::eOk if Successful
    /// </returns>
    ACDBCORE2D_PORT Acad::ErrorStatus setViewLabelTextStyleId (const AcDbObjectId &objId);

    /// <summary>
    /// Returns the color of view label.
    /// </summary>
    ///
    /// <returns>
    /// Returns the color of view label.
    /// </returns>
    ACDBCORE2D_PORT AcCmColor         viewLabelTextColor () const;

    /// <summary>
    /// Sets the view label to use specified color.
    /// </summary>
    ///
    /// <param name="color">
    /// Color to use for view label
    /// </param>
    ///
    /// <returns>
    /// Return Acad::eOk if Successful
    /// </returns>
    ACDBCORE2D_PORT Acad::ErrorStatus setViewLabelTextColor (const AcCmColor& color);

    /// <summary>
    /// Returns the text height of view label.
    /// </summary>
    ///
    /// <returns>
    /// Returns the text height of view label.
    /// </returns>
    ACDBCORE2D_PORT double            viewLabelTextHeight () const;
    ACDBCORE2D_PORT Acad::ErrorStatus setViewLabelTextHeight (double height);

    /// <summary>
    /// Returns the offset of view label.
    /// </summary>
    ///
    /// <returns>
    /// Returns the offset of view label.
    /// </returns>
    ACDBCORE2D_PORT double            viewLabelOffset () const;

    /// <summary>
    /// Sets the offset for view label.
    /// </summary>
    ///
    /// <param name="offset">
    /// offset for view label
    /// </param>
    ///
    /// <returns>
    /// Return Acad::eOk if Successful
    /// </returns>
    ACDBCORE2D_PORT Acad::ErrorStatus setViewLabelOffset (double offset);

    /// <summary>
    /// Returns the attachment of view label.
    /// </summary>
    ///
    /// <returns>
    /// Returns the attachment of view label.
    /// </returns>
    ACDBCORE2D_PORT AcDbModelDocViewStyle::AttachmentPoint viewLabelAttachment () const;

    /// <summary>
    /// Sets the attachment for view label.
    /// </summary>
    ///
    /// <param name="attachment">
    /// attachment of view label
    /// </param>
    ///
    /// <returns>
    /// Return Acad::eOk if Successful
    /// </returns>
    ACDBCORE2D_PORT Acad::ErrorStatus setViewLabelAttachment (AcDbModelDocViewStyle::AttachmentPoint attachment);

    /// <summary>
    /// Returns the text alignment of view label.
    /// </summary>
    ///
    /// <returns>
    /// Returns the text alignment of view label.
    /// </returns>
    ACDBCORE2D_PORT AcDbModelDocViewStyle::TextAlignment viewLabelAlignment () const;

    /// <summary>
    /// Sets the text alignment for view label.
    /// </summary>
    ///
    /// <param name="attachment">
    /// text alignment for view label
    /// </param>
    ///
    /// <returns>
    /// Return Acad::eOk if Successful
    /// </returns>
    ACDBCORE2D_PORT Acad::ErrorStatus setViewLabelAlignment (AcDbModelDocViewStyle::TextAlignment alignment);

    /// <summary>
    /// Returns the pattern of view label.
    /// </summary>
    ///
    /// <returns>
    /// Returns the pattern of view label.
    /// </returns>
    ACDBCORE2D_PORT const ACHAR *     viewLabelPattern () const;

    /// <summary>
    /// Gets the pattern for view label. If 'pField' parameter is specified and label
    /// pattern is currently using fields, the master field will be copied to 'pField'
    /// including child fields.
    /// </summary>
    ///
    /// <param name="pattern">
    /// pattern for view label
    /// </param>
    /// <param name="pField">
    /// If this optional parameter is specified and the pattern label is currently
    /// using fields, the master field will be copied to 'pField'. Child fields
    /// are also copied.
    /// </param>
    ///
    /// <returns>
    /// Return Acad::eOk if Successful
    /// </returns>
    ACDBCORE2D_PORT Acad::ErrorStatus getViewLabelPattern (AcString &pattern,
                                           AcDbField *pField = NULL) const;

    /// <summary>
    /// Sets the pattern for view label. If 'pField' parameter is specified and
    /// contains child fields, then label pattern will be acquired from the field.
    /// Otherwise 'pattern' is used.
    /// </summary>
    ///
    /// <param name="pattern"> pattern for view label
    /// </param>
    /// <param name="pField">
    /// If 'pField' parameter is specified and contains child fields, then label
    /// pattern will be acquired from the field, rather than from 'pattern'.
    /// The master field including child fields are also copied to view style's field.
    /// </param>
    ///
    /// <returns>
    /// Return Acad::eOk if Successful
    /// </returns>
    ACDBCORE2D_PORT Acad::ErrorStatus setViewLabelPattern (const ACHAR* pattern,
                                           const AcDbField *pField = NULL);

    /// <summary>
    /// Returns boolean value indicating if to show view label.
    /// </summary>
    ///
    /// <returns>
    /// true if to show view label is on, false otherwise.
    /// </returns>
    ACDBCORE2D_PORT bool              showViewLabel () const;
    ACDBCORE2D_PORT Acad::ErrorStatus setShowViewLabel (bool bValue);

    /// <summary>
    /// Returns the label and model edge flags.
    /// </summary>
    ///
    /// <returns>
    /// Returns the bit coded flags defining label and identifier placement.
    /// </returns>
    ACDBCORE2D_PORT AcDbDetailViewStyle::ModelEdge modelEdge () const;

    /// <summary>
    /// Sets the label and identifier placement flags.
    /// Use bit coded flags defined by modelEdge enumeration.
    /// </summary>
    ///
    /// <param name="placement">
    /// Bit coded flags defining label and identifier placement.
    /// </param>
    ///
    /// <returns>
    /// Return Acad::eOk if Successful
    /// </returns>
    ACDBCORE2D_PORT Acad::ErrorStatus setModelEdge (AcDbDetailViewStyle::ModelEdge placement);


    /// <summary>
    /// Returns the line weight of Border line.
    /// </summary>
    ///
    /// <returns>
    /// Returns the line weight of Border line.
    /// </returns>
    ACDBCORE2D_PORT AcDb::LineWeight  borderLineWeight () const;

    /// <summary>
    /// Sets the Border line to use specified line weight.
    /// </summary>
    ///
    /// <param name="color">
    /// Line weight to use.for Border.line
    /// </param>
    ///
    /// <returns>
    /// Return Acad::eOk if Successful
    /// </returns>
    ACDBCORE2D_PORT Acad::ErrorStatus setBorderLineWeight (AcDb::LineWeight lineweight);

    /// <summary>
    /// Returns the color of Border line.
    /// </summary>
    ///
    /// <returns>
    /// Returns the color of Border line.
    /// </returns>
    ACDBCORE2D_PORT AcCmColor         borderLineColor () const;

    /// <summary>
    /// Sets the Border line to use specified color.
    /// </summary>
    ///
    /// <param name="color">
    /// Color to use for Border.line
    /// </param>
    ///
    /// <returns>
    /// Return Acad::eOk if Successful
    /// </returns>
    ACDBCORE2D_PORT Acad::ErrorStatus setBorderLineColor (const AcCmColor& color);

    /// <summary>
    /// Returns the linetype objectId of Border line.
    /// </summary>
    ///
    /// <returns>
    /// Returns the linetype objectId of Border line.
    /// </returns>
    ACDBCORE2D_PORT AcDbObjectId      borderLineTypeId () const;

    /// <summary>
    /// Sets the Border line to use specified linetype.
    /// </summary>
    ///
    /// <param name="objId">
    /// ObjectId of the linetype to use.for Border.line
    /// </param>
    ///
    /// <returns>
    /// Return Acad::eOk if Successful
    /// </returns>
    ACDBCORE2D_PORT Acad::ErrorStatus setBorderLineTypeId (const AcDbObjectId &objId);

protected:
    // For internal use only
    ACDBCORE2D_PORT Acad::ErrorStatus subDeepClone(AcDbObject* pOwner, AcDbObject*& pClonedObject,
        AcDbIdMapping& idMap, Adesk::Boolean isPrimary = true) const override;
    ACDBCORE2D_PORT Acad::ErrorStatus subWblockClone(AcRxObject* pOwner, AcDbObject*& pClonedObject,
        AcDbIdMapping& idMap, Adesk::Boolean isPrimary = true) const override;
};

#endif //__ACDBDETAILVIEWSTYLE_H__
