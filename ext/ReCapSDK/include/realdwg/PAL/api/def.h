//  Copyright 2020 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form

#pragma once

#ifndef ACPAL_DEF_H
#define ACPAL_DEF_H

#if defined(_MSC_VER) 
    #ifdef  ACPAL_API
        #define   ACPAL_PORT _declspec(dllexport)
    #else
//don't use __declspec(dllimport) so that we can use the .objs with both static and dynamc linking
        #define   ACPAL_PORT
    #endif
#elif defined(__clang__)
    #ifdef  ACPAL_API
        #define   ACPAL_PORT __attribute__ ((visibility ("default")))
    #else
        #define   ACPAL_PORT
    #endif
#else
    #error Visual C++ or Clang compiler is required.
#endif


// AcPal [defined(ACPAL_API)] will implement some non cross platform
//      API for vertical [!defined(_ADESK_CROSS_PLATFORM_)] convenience.
// Cross platform code or Fabric [defined(_ADESK_CROSS_PLATFORM_) in
//      .cmake by default] should not use these APIs.
// AcPal unit test [defined(ACPAL_TEST)] need test these APIs.
#if defined(ACPAL_API) || !defined(_ADESK_CROSS_PLATFORM_) || defined(ACPAL_TEST)
#define AC_NON_CROSS_PLATFORM_API
#endif



#endif
