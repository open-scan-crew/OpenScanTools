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
#include "acdbassocactionbody.h"
#pragma pack (push, 8)


/// <summary>
/// Deprecated in AutoCAD 2013. Please derive directly from the AcDbAssocActionBody class
/// that now contains all the methods of the deprecated AcDbAssocParamBasedActionBody
/// derived class.
/// </summary>
///
class  AcDbAssocParamBasedActionBody : public AcDbAssocActionBody
{
public: 
    ACRX_DECLARE_MEMBERS_EXPIMP(AcDbAssocParamBasedActionBody, ACDBCORE2D_PORT);

    ACDBCORE2D_PORT explicit AcDbAssocParamBasedActionBody(AcDbAssocCreateImpObject createImpObject = kAcDbAssocCreateImpObject);
};


#pragma pack (pop)

