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

//ReCap data includes
#include <data/RCProject.h>
#include <data/RCScan.h>
#include <data/RCStructuredScan.h>
#include <data/IRCPointIterator.h>
#include "data/RCSpatialFilter.h"

//ReCap foundation includes
#include <foundation/RCCode.h>
#include <foundation/RCUnitService.h>
#include <foundation/RCSharedPtr.h>
#include <foundation/RCSphericalModel.h>

namespace NS_RCData = Autodesk::RealityComputing::Data;
namespace NS_RCFoundation = Autodesk::RealityComputing::Foundation;

namespace NS_RevitDB = Autodesk::Revit::DB;


using namespace System;

using namespace System::Runtime::InteropServices;

using namespace System::Collections::Generic;

#define RECAPWRAPPER_DOUBLE_TOLERANCE 0.0000001
