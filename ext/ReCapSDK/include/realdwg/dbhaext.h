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
// DBHAEXT.H
//
// DESCRIPTION: This class provides the abstract base class for
//              protocol extension on AcDbHostApplicationServices
//              whereby a host application can instruct to an Object
//              Enabler what services it doesn't want to expose to
//              its end users. The Object Enabler, upon querying this,
//              selectively denys certain service for that particular
//              host app.
//
//              Absence of this protocol extension on the host app
//              is interpreted as all service is allowed.
//

#ifndef AcDbHostApplicationServiceRestrictions_INC
#define AcDbHostApplicationServiceRestrictions_INC

#include "acdb.h"
#include "dbmain.h"

#pragma pack (push, 8)

class AcDbHostApplicationServiceRestrictions : public AcRxObject
{
public:

    enum RestrictionType {
        kAllAllowed              = 0x00,
        kTransformNotAllowed     = 0x01,
        kEditingNotAllowed       = 0x02,
        kCloneNotAllowed         = 0x04
        };

	ACRX_DECLARE_MEMBERS(AcDbHostApplicationServiceRestrictions);

	virtual AcDbHostApplicationServiceRestrictions::RestrictionType
                whatsNotAllowed() const  = 0;
};

#pragma pack (pop)

#endif

