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

#include "rxcopyonwriteobject.h"

#pragma pack(push, 8)

class AcGiParameterImp;

///////////////////////////////////////////////////////////////////////////////
// class AcGiParameter
//
class  AcGiParameter : public AcRxCopyOnWriteObject
{
    friend class AcGiParameterImp;

public:
    ACRX_DECLARE_MEMBERS_READWRITE_EXPIMP(AcGiParameter, AcGiParameterImp, ACDBCORE2D_PORT);

    ACDBCORE2D_PORT AcGiParameter(AcGiParameterImp* pImp);
    ACDBCORE2D_PORT AcGiParameter(const AcGiParameter& other);
    ACDBCORE2D_PORT const AcGiParameter& operator=(const AcGiParameter& other);
};

#pragma pack(pop)
