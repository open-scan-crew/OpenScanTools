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
//
#ifndef __AXPNT2D_H_
#define __AXPNT2D_H_

#include "gept2dar.h"
#include "gepnt2d.h"
#include "gevec2d.h"
#pragma pack (push, 8)

#ifndef AXAUTOEXP
#ifdef AXAUTO_DLL
	#define AXAUTOEXP __declspec(dllexport)
#else
	#define AXAUTOEXP __declspec(dllimport)
#endif
#endif

#pragma warning(disable : 4290) 

class AXAUTOEXP AcAxPoint2d : public AcGePoint2d
{
public:
    // constructors
    AcAxPoint2d();
    AcAxPoint2d(double x, double y);
    AcAxPoint2d(const AcGePoint2d& pt);
    AcAxPoint2d(const AcGeVector2d& pt);
   	AcAxPoint2d(const VARIANT* var) noexcept(false);
   	AcAxPoint2d(const VARIANT& var) noexcept(false);
   	AcAxPoint2d(const SAFEARRAY* safeArrayPt) noexcept(false);

    // equal operators
   	AcAxPoint2d& operator=(const AcGePoint2d& pt);
   	AcAxPoint2d& operator=(const AcGeVector2d& pt);
   	AcAxPoint2d& operator=(const VARIANT* var) noexcept(false);
   	AcAxPoint2d& operator=(const VARIANT& var) noexcept(false);
   	AcAxPoint2d& operator=(const SAFEARRAY* safeArrayPt) noexcept(false);

    // type requests
    VARIANT* asVariantPtr() const noexcept(false);
    SAFEARRAY* asSafeArrayPtr() const noexcept(false);

    VARIANT& setVariant(VARIANT& var) const noexcept(false);
    VARIANT* setVariant(VARIANT* var) const noexcept(false);

    // utilities
private:
    AcAxPoint2d& fromSafeArray(const SAFEARRAY* safeArrayPt) noexcept(false);
};

#pragma warning(disable : 4275) 

class AXAUTOEXP AcAxPoint2dArray : public AcGePoint2dArray
{
public:
    // equal operators
   	AcAxPoint2dArray& append(const AcGePoint2d& pt);
   	AcAxPoint2dArray& append(const VARIANT* var) noexcept(false);
   	AcAxPoint2dArray& append(const VARIANT& var) noexcept(false);
   	AcAxPoint2dArray& append(const SAFEARRAY* safeArrayPt) noexcept(false);
    
    
    // type requests
    SAFEARRAY* asSafeArrayPtr() const noexcept(false);

    VARIANT& setVariant(VARIANT& var) const noexcept(false);
    VARIANT* setVariant(VARIANT* var) const noexcept(false);

    // utilities
private:
    AcAxPoint2dArray& fromSafeArray(const SAFEARRAY* safeArrayPt) noexcept(false);
};

#pragma pack (pop)
#endif
