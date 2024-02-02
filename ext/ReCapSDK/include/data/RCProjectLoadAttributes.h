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

namespace Autodesk { namespace RealityComputing { namespace Data {

    /// \brief An enum class for project Access
    enum class RCFileAccess
    {
        ReadOnly,
        ReadWrite,
        Overwrite
    };

    /// \brief An enum class for what kind of edits to load.
    enum class RCProjectUserEdits
    {
        None,
        All
    };
}}}    // namespace Autodesk::RealityComputing::Data
