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
// CREATED BY: Jiri Kripac                                 March 2009
//
// DESCRIPTION:
//
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include "AcDbAssocActionParam.h"
#pragma pack (push, 8)


/// <summary>
/// Action parameter that owns other AcDbAssocActionParameters, allowing to represent 
/// hierarchical structures of action parameters.
/// </summary>
///
class  AcDbAssocCompoundActionParam : public AcDbAssocActionParam
{
public:
    ACRX_DECLARE_MEMBERS_EXPIMP(AcDbAssocCompoundActionParam, ACDBCORE2D_PORT);

    /// <summary> Default constructor. </summary>
    /// <param name="createImpObject"> See AcDbAssocCreateImpObject. </param>
    ///
    ACDBCORE2D_PORT explicit AcDbAssocCompoundActionParam(AcDbAssocCreateImpObject createImpObject = kAcDbAssocCreateImpObject);

    /// <summary> 
    /// Removes all owned AcDbAssocActionParams, optionally also erasing them. 
    /// </summary>
    /// <param name="alsoEraseThem">  
    /// If true, the owned AcDbAssocActionParams are also erased. Otherwise they
    /// are just detached so that this AcDbAssocCompoundActionParam is not their
    /// database owner.
    /// </param>
    /// <returns> Acad::ErrorStatus. </returns>
    ///
    ACDBCORE2D_PORT virtual Acad::ErrorStatus removeAllParams(bool alsoEraseThem);

    /// <summary> Returns then number of the owned AcDbAssocActionParams. </summary>
    /// <returns> Number of the owned AcDbAssocActionParams. </returns>
    ///
    ACDBCORE2D_PORT int paramCount() const;

    /// <summary> 
    /// Returns an array of the owned AcDbAssocActionParams. 
    /// </summary>
    /// <returns> 
    /// Array of the owned AcDbAssocActionParams. Notice that it returns a 
    /// const reference to an internal array. The client code must not hold on 
    /// to this array and should make a copy of the array if it is doing anything 
    /// more than just momentarily accessing its elements.
    /// </returns>
    ///
    ACDBCORE2D_PORT const AcDbObjectIdArray& ownedParams() const;

    /// <summary> 
    /// Adds a new AcDbAssocActionParam. The AcDbAssocCompoundActionParam
    /// becomes the database owner of this action parameter.
    /// </summary>
    /// <param name="paramId"> 
    /// AcDbObjectId of the action parameter being added. If the action parameter 
    /// is already owned by this AcDbAssocCompoundActionParam, it it just returns 
    /// its existing index in the array and does not add it again. 
    ///</param>
    /// <param name="paramIndex"> 
    /// Returned index of the newly added action parameter. If the parameter is 
    /// already owned by this AcDbAssocCompoundActionParam, its existing index
    /// is returned.
    /// </param>
    /// <returns> Acad::ErrorStatus. </returns>
    ///
    ACDBCORE2D_PORT Acad::ErrorStatus addParam(const AcDbObjectId& paramId, int& paramIndex);

    /// <summary> 
    /// Removes the AcDbAssocActionParam, optionally also erasing it. 
    /// </summary>
    /// <param name="paramId"> AcDbObjectId of the action parameter to remove. </param>
    /// <param name="alsoEraseIt">  
    /// If true, the owned action parameter is also erased. Otherwise it is just
    /// detached so that this AcDbAssocCompoundActionParam is not its database owner.
    /// </param>
    /// <returns> Acad::ErrorStatus. </returns>
    ///
    ACDBCORE2D_PORT Acad::ErrorStatus removeParam(const AcDbObjectId& paramId, bool alsoEraseIt); 

    /// <summary> 
    /// Returns all owned AcDbAssocActionParams with the given name. Notice that
    /// more than one action parameter may have the same name. The parameters are 
    /// always returned in the same order which is the order the parameters were 
    /// added to the action body. Thus, an array of action parameters can easily 
    /// be represented by multiple action parameters with the same name.
    /// </summary>
    /// <param name="paramName">  The name of the action parameter. </param>
    /// <returns> All owned action parameters with the given name. </returns>
    ///
    ACDBCORE2D_PORT const AcDbObjectIdArray& paramsAtName(const AcString& paramName) const;

    /// <summary> 
    /// Returns an owned AcDbAssocActionParam specified by its name and optional 
    /// index among all parameters with the same name. The default index 0 means
    /// the first parameter with the given name is returned.
    /// </summary>
    /// <param name="paramName"> The name of the action parameter. </param>
    /// <param name="index"> Index among all parameters with the same name. </param>
    /// <returns> The owned action parameter or a null AcDbObjectId if not found. </returns>
    ///
    ACDBCORE2D_PORT AcDbObjectId paramAtName(const AcString& paramName, int index = 0) const;

    /// <summary> 
    /// Returns an owned AcDbAssocActionParam in the array of all action parameters 
    /// owned by this AcDbAssocCompoundActionParam.
    /// </summary>
    /// <param name="paramIndex"> Index in the array of all action parameters. </param>
    /// <returns> The owned action parameter or a null AcDbObjectId if not found. </returns>
    ///
    ACDBCORE2D_PORT AcDbObjectId paramAtIndex(int paramIndex) const;
};

#pragma pack (pop)
