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

#ifndef ACCOREDEFS_H
#define ACCOREDEFS_H

#if defined(_MSC_VER) || defined(_ADESK_MAC_)
#ifndef ACCORE_PORT
#ifdef ACCORE_API
    #include "adesk.h"
    #define ACCORE_PORT ADESK_EXPORT
    #define ACCORE_DATA_PORT _declspec(dllexport)
    #define ACCORE_STATIC_DATA_PORT _declspec(dllexport) static
#else
    #define ACCORE_PORT
    #define ACCORE_DATA_PORT _declspec(dllimport)
    #define ACCORE_STATIC_DATA_PORT _declspec(dllimport) static
#endif
#endif
#else
    #define ACCORE_PORT
    #define ACCORE_DATA_PORT
    #define ACCORE_STATIC_DATA_PORT static
#endif

#endif //ACCOREDEFS_H
