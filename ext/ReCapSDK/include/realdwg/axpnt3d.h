//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2020 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __AXPNT3D_H_
#define __AXPNT3D_H_

#include "gept3dar.h"
#include "gepnt3d.h"
#include "gevec3d.h"
#pragma pack (push, 8)

#ifndef AXAUTOEXP
#ifdef AXAUTO_DLL
	#define AXAUTOEXP __declspec(dllexport)
#else
	#define AXAUTOEXP __declspec(dllimport)
#endif
#endif

#pragma warning(disable : 4290) 

class AXAUTOEXP AcAxPoint3d : public AcGePoint3d
{
public:
    // constructors
    AcAxPoint3d();
    AcAxPoint3d(double x, double y, double z);
    AcAxPoint3d(const AcGePoint3d& pt);
    AcAxPoint3d(const AcGeVector3d& pt);
   	AcAxPoint3d(const VARIANT* var) noexcept(false);
   	AcAxPoint3d(const VARIANT& var) noexcept(false);
   	AcAxPoint3d(const SAFEARRAY* safeArrayPt) noexcept(false);

    // equal operators
   	AcAxPoint3d& operator=(const AcGePoint3d& pt);
   	AcAxPoint3d& operator=(const AcGeVector3d& pt);
   	AcAxPoint3d& operator=(const VARIANT* var) noexcept(false);
   	AcAxPoint3d& operator=(const VARIANT& var) noexcept(false);
   	AcAxPoint3d& operator=(const SAFEARRAY* safeArrayPt) noexcept(false);

    // type requests
    VARIANT* asVariantPtr() const noexcept(false);
    SAFEARRAY* asSafeArrayPtr() const noexcept(false);

    VARIANT& setVariant(VARIANT& var) const noexcept(false);
    VARIANT* setVariant(VARIANT* var) const noexcept(false);

    // utilities
private:
    AcAxPoint3d& fromSafeArray(const SAFEARRAY* safeArrayPt) noexcept(false);
};

#pragma warning(disable : 4275) 

class AXAUTOEXP AcAxPoint3dArray : public AcGePoint3dArray
{
public:
    // equal operators
   	AcAxPoint3dArray& append(const AcGePoint3d& pt);
   	AcAxPoint3dArray& append(const VARIANT* var) noexcept(false);
   	AcAxPoint3dArray& append(const VARIANT& var) noexcept(false);
   	AcAxPoint3dArray& append(const SAFEARRAY* safeArrayPt) noexcept(false);
    
    
    // type requests
    SAFEARRAY* asSafeArrayPtr() const noexcept(false);

    VARIANT& setVariant(VARIANT& var) const noexcept(false);
    VARIANT* setVariant(VARIANT* var) const noexcept(false);

    // utilities
private:
    AcAxPoint3dArray& fromSafeArray(const SAFEARRAY* safeArrayPt) noexcept(false);
};

#pragma pack (pop)
#endif
