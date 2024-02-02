#ifndef NAVLIB_DEFINES_H_INCLUDED_
#define NAVLIB_DEFINES_H_INCLUDED_
// <copyright file="navlib.h" company="3Dconnexion">
// -------------------------------------------------------------------------------------------------
// Copyright (c) 2014-2019 3Dconnexion. All rights reserved.
//
// This file and source code are an integral part of the "3Dconnexion Software Developer Kit",
// including all accompanying documentation, and is protected by intellectual property laws. All use
// of the 3Dconnexion Software Developer Kit is subject to the License Agreement found in the
// "LicenseAgreementSDK.txt" file. All rights not expressly granted by 3Dconnexion are reserved.
// -------------------------------------------------------------------------------------------------
// </copyright>
// <history>
// *************************************************************************************************
// File History
//
// $Id: navlib_defines.h 16000 2019-03-25 07:36:56Z mbonk $
//
// 01/23/14 MSB Initial design
// </history>
// <description>
// *************************************************************************************************
// File Description
//
// This header file defines the macros used in the 3dconnexion interface and header files.
//
// *************************************************************************************************
// </description>

// Invalid handle
#define INVALID_NAVLIB_HANDLE 0

// Navlib facility used to generate error codes
// Note this is identical to FACILITY_ITF on windows
#if _WIN32
#define FACILITY_NAVLIB 4
#else
#define FACILITY_NAVLIB 4
#endif

// resources
#define NAVLIB_IDB_ManualPivot 0x6004
#define NAVLIB_IDB_AutoPivot 0x6005

#if __cplusplus
#define _NAVLIB_BEGIN namespace navlib {
#define _NAVLIB_END }
#define _NAVLIB ::navlib::
#define _USING_NAVLIB using namespace navlib;
#else
#define _NAVLIB_BEGIN
#define _NAVLIB_END
#define _NAVLIB
#define _USING_NAVLIB
#endif

#if defined(_MSC_VER) && defined(NAVLIB_EXPORTS)
#define _NAVLIB_DLLAPI extern "C" __declspec(dllexport)
#elif __cplusplus
#define _NAVLIB_DLLAPI extern "C"
#else
#define _NAVLIB_DLLAPI
#endif
#endif // NAVLIB_DEFINES_H_INCLUDED_
