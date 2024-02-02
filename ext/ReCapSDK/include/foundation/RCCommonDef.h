//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Copyright 2018 by Autodesk, Inc.
//
// The information contained herein is confidential, proprietary to Autodesk,
// Inc., and considered a trade secret as defined in section 499C of the
// penal code of the State of California.  Use of this information by anyone
// other than authorized employees of Autodesk, Inc. is granted only under a
// written non-disclosure agreement, expressly prescribing the scope and
// manner of such use.
//
//        AUTODESK PROVIDES THIS PROGRAM "AS IS" AND WITH ALL FAULTS.
//        AUTODESK SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF
//        MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  AUTODESK, INC.
//        DOES NOT WARRANT THAT THE OPERATION OF THE PROGRAM WILL BE
//        UNINTERRUPTED OR ERROR FREE.
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#ifdef _MSC_VER
#ifdef RC_COMMON_EXPORTS
#define RC_COMMON_API __declspec(dllexport)
#define RC_COMMON_API_T
#else
#define RC_COMMON_API __declspec(dllimport)
#define RC_COMMON_API_T extern
#endif
#define RC_COMMON_API_INLINE __forceinline
#elif defined(__clang__)
#define RC_COMMON_API
#define RC_COMMON_API_T
#define RC_COMMON_API_INLINE inline __attribute__((always_inline))
#else
#define RC_COMMON_API
#define RC_COMMON_API_T
#define RC_COMMON_API_INLINE inline
#endif
