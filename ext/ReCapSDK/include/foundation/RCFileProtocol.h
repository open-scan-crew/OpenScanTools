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

#include <foundation/RCFileSystemUtilityDef.h>

namespace Autodesk { namespace RealityComputing { namespace Foundation {

    class ARCFileHandle;

    /// <description>
    /// Interface that abstracts away the details of reading files.
    /// </description>
    class RC_FS_UTILITY_API IRCFileProtocol
    {
    public:
        IRCFileProtocol();

        virtual bool FileExists(const wchar_t* file_uri)       = 0;
        virtual long long GetFileSize(const wchar_t* file_uri) = 0;

        virtual ARCFileHandle* OpenFile(const wchar_t* file_uri)                                   = 0;
        virtual void PrepareRead(ARCFileHandle* handle, long long offset, int num_bytes)           = 0;
        virtual bool ReadFile(ARCFileHandle* handle, long long offset, int num_bytes, char* bytes) = 0;
        virtual bool CloseFile(ARCFileHandle* handle)                                              = 0;

    protected:
        virtual ~IRCFileProtocol();
    };

}}}    // namespace Autodesk::RealityComputing::Foundation
