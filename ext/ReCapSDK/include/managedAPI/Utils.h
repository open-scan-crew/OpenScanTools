//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Copyright 2020 by Autodesk, Inc.
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

#include "managedAPI/Wrap_RCBox.h"
#include "managedAPI/Wrap_RCVector.h"

#include <foundation/RCBox.h>
#include <foundation/RCVector.h>

#include "globals.h"

NS_RCFoundation::RCVector3d vec3dFromWrapped(Autodesk::RealityComputing::Managed::RCVector3d vector);
NS_RCFoundation::RCBox boxFromWrapped(Autodesk::RealityComputing::Managed::RCBox box);
NS_RCFoundation::RCVector4ub colorFromWrapped(Autodesk::RealityComputing::Managed::RCColor color);

//string conversions

System::String^ convertFromRecapANSI(const NS_RCFoundation::RCString& str);
System::String^ convertFromRecapUnicode(const NS_RCFoundation::RCString& str);

NS_RCFoundation::RCString convertFromDotNetAnsi(System::String^ str);
NS_RCFoundation::RCString convertFromDotNetUnicode(System::String^ str);

bool areDoublesEqual(const double doubleValue1, const double doubleValue2);