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
#include "acdbport.h"
#include "dbmain.h"
#include "rxboiler.h"
#include "gepnt3d.h"
#include "gevec3d.h"
#include "AcString.h"

#pragma pack (push, 8)

class AcDbMText;

/// <summary>
/// the AcDbGeoPositionMarker class represents a Geographical location aware marker object with label
/// </summary>
///
class  AcDbGeoPositionMarker : public AcDbEntity
{
public:

    enum TextAlignmentType {
        /// <summary>
        /// Left-justifies text in the MText.
        /// </summary>
        ///
        /// <value>
        /// 0
        /// </value>
        kLeftAlignment = 0,
        /// <summary>
        /// Centers text in MText.
        /// </summary>
        ///
        /// <value>
        /// 1
        /// </value>
        kCenterAlignment = 1,
        /// <summary>
        /// Right-justifies text in MText.
        /// </summary>
        ///
        /// <value>
        /// 2
        /// </value>
        kRightAlignment = 2
    };

    ACRX_DECLARE_MEMBERS_EXPIMP(AcDbGeoPositionMarker, ACDBCORE2D_PORT);

    ACDBCORE2D_PORT AcDbGeoPositionMarker();
    ACDBCORE2D_PORT AcDbGeoPositionMarker(const AcGePoint3d &position, double radius, double landingGap);


    /// <summary> Get/Set Insertion point of the position marker. </summary>
    ///
    ACDBCORE2D_PORT AcGePoint3d          position() const;
    ACDBCORE2D_PORT Acad::ErrorStatus    setPosition(const AcGePoint3d &position);

    /// <summary> Get/Set radius of the position marker. </summary>
    ///
    ACDBCORE2D_PORT double               radius() const;
    ACDBCORE2D_PORT Acad::ErrorStatus    setRadius(double radius);

    /// <summary> Get/Set position marker text label. </summary>
    ///
    ACDBCORE2D_PORT AcString             text() const;
    ACDBCORE2D_PORT Acad::ErrorStatus    setText(const AcString& text);

    /// <summary> Get/Set position marker MText label. </summary>
    ///
    ACDBCORE2D_PORT AcDbMText*           mtext() const;
    ACDBCORE2D_PORT Acad::ErrorStatus    setMText(const AcDbMText* pMText);

    /// <summary> Get/Set the visibility of the MText label. </summary>
    ///
    ACDBCORE2D_PORT bool                 mtextVisible() const;
    ACDBCORE2D_PORT Acad::ErrorStatus    setMTextVisible(bool visible);

    /// <summary> Get/Set landing gap of the position marker. </summary>
    ///
    ACDBCORE2D_PORT double               landingGap() const;
    ACDBCORE2D_PORT Acad::ErrorStatus    setLandingGap(double landingGap);

    /// <summary> Get/Set label text frame enabling. </summary>
    ///
    ACDBCORE2D_PORT bool                 enableFrameText() const;
    ACDBCORE2D_PORT Acad::ErrorStatus    setEnableFrameText(bool enableFrameText);

    /// <summary> Get/Set label text alignment type. </summary>
    ///
    ACDBCORE2D_PORT TextAlignmentType    textAlignmentType() const;
    ACDBCORE2D_PORT Acad::ErrorStatus    setTextAlignmentType(TextAlignmentType textAlignmentType);

    /// <summary> Get/Set the notes. </summary>
    ///
    ACDBCORE2D_PORT AcString             notes() const;
    ACDBCORE2D_PORT Acad::ErrorStatus    setNotes(const AcString& notes);

    /// <summary> Get/Set the Geographical location of the position marker. </summary>
    ///
    ACDBCORE2D_PORT AcGePoint3d          geoPosition() const;
    ACDBCORE2D_PORT Acad::ErrorStatus    setGeoPosition(const AcGePoint3d &position);
    ACDBCORE2D_PORT Acad::ErrorStatus    latLonAlt(double &lat, double &lon, double &alt) const;
    ACDBCORE2D_PORT Acad::ErrorStatus    setLatLonAlt(double lat, double lon, double alt);

    /// <summary> The normal of the position marker. </summary>
    ///
    ACDBCORE2D_PORT AcGeVector3d        normal() const;

    /// <summary> The text style Object ID of the position marker text. </summary>
    ///
    ACDBCORE2D_PORT AcDbObjectId        textStyle() const;

protected:
    ACDBCORE2D_PORT Acad::ErrorStatus   subGetClassID(CLSID* pClsid) const override;
};

#pragma pack (pop)
