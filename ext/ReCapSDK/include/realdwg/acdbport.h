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
#pragma once

#ifdef  ACDB_API
    #include "adesk.h"
    #define   ACDB_PORT ADESK_EXPORT
#else
    #define   ACDB_PORT
#endif

