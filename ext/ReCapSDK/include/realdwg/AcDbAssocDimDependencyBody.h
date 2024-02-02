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
// AcDbAssocDimDependencyBody derived class.
//
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include "AcDbAssocDimDependencyBodyBase.h"


/// <summary>
/// AcDbAssocDimDependencyBody manages an AcDbDimension entity that serves 
/// as the graphical representation of a dimensional constaint. It inherits
/// common functionality from the AcDbAssocDimDependencyBodyBase class and 
/// overrides the base-class pure virtual methods in which it implements the
/// AcDbDimension-specific functionality.
/// </summary>
///
class  AcDbAssocDimDependencyBody : public AcDbAssocDimDependencyBodyBase
{
public:
    ACRX_DECLARE_MEMBERS_EXPIMP(AcDbAssocDimDependencyBody, ACDBCORE2D_PORT);

    /// <summary> Default constructor.</summary>
    ACDBCORE2D_PORT AcDbAssocDimDependencyBody();

    /// <summary> Destructor. </summary>
    ACDBCORE2D_PORT ~AcDbAssocDimDependencyBody();

    // Implementation of pure virtual methods in the AcDbAssocDimDependencyBodyBase 
    // class. They deal with the controlled entity that is assumed to be an 
    /// AcDbDimension

    /// <summary> Returns the text of the controlled AcDimension. </summary>
    /// <returns> AcDbDimension text. </returns>
    ///
    ACDBCORE2D_PORT AcString getEntityTextOverride() const override;

    /// <summary> Sets the text property in the controlled AcDbDimension. </summary>
    /// <param  name="newText"> New text to set in the AcDbDimension.</param>
    /// <returns> Acad::eOk if successful. </returns>
    ///
    ACDBCORE2D_PORT Acad::ErrorStatus setEntityTextOverride(const AcString& newText) override;

    /// <summary> Gets the measurement from the controlled AcDbDimension. </summary>
    /// <returns> AcDbDimension measurement. </returns>
    ///
    ACDBCORE2D_PORT double getEntityMeasurementOverride() const override;

    /// <summary> Returns true iff the attachment of the controlled AcDbDimension
    /// changed, such as the AcDbDimension has been repositioned. </summary>
    /// <returns> Returns true if attachment changed. </returns>
    ///
    ACDBCORE2D_PORT bool isEntityAttachmentChangedOverride() const override;

    // Overridden method from the AcDbAssocDependencyBody base class

    /// <summary> Updates position, size and orientation of the controlled AcDbDimension.
    /// </summary>
    /// <returns> Acad::eOk if successful. </returns>
    ///
    ACDBCORE2D_PORT Acad::ErrorStatus updateDependentOnObjectOverride() override;

    /// <summary>
    /// "Pseudo constructor". Creates a new AcDbAssocDependency object owning 
    /// a new AcDbAssocDimDependencyBody object and makes the dependent-on object
    /// of the new dependency to be the provided AcDbDimension. Posts both objects 
    /// to the database of the AcDbDimension.
    /// </summary>
    /// <param name="dimId"> AcDbObjectId of the AcDbDimension. </param>
    /// <param name="dimDepId"> AcDbObjectId of the created AcDbAssocDependency. </param>
    /// <param name="dimDepBodyId"> AcDbObjectId of the created AcDbAssocDimDependencyBody. </param>
    /// <returns> Acad::eOk if successful. </returns>
    ///
    ACDBCORE2D_PORT static Acad::ErrorStatus
                createAndPostToDatabase(const AcDbObjectId& dimId,
                                        AcDbObjectId&       dimDepId,
                                        AcDbObjectId&       dimDepBodyId);
};

