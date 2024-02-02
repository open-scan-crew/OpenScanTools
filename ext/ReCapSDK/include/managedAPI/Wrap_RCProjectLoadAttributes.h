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

#include "globals.h"

namespace Autodesk { namespace RealityComputing { namespace Managed {
    /// \brief An enum class for project Access
    public enum class RCFileAccess
    {
        ReadOnly = (int)NS_RCData::RCFileAccess::ReadOnly,
        ReadWrite = (int)NS_RCData::RCFileAccess::ReadWrite,
        Overwrite = (int)NS_RCData::RCFileAccess::Overwrite
    };

    /// \brief An enum class for what kind of edits to load.
    public enum class RCProjectUserEdits
    {
        None = (int)NS_RCData::RCProjectUserEdits::None,
        All = (int)NS_RCData::RCProjectUserEdits::All
    };
}}}    // namespace Autodesk::RealityComputing::Managed