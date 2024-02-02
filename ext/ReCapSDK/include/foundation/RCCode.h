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

namespace Autodesk { namespace RealityComputing { namespace Foundation {
    enum class RCCode
    {
        Unknown = -1,

        // bool
        False = 0,
        True  = 1,

        // operation
        OK                      = 100,
        Failed                  = 101,
        Cancelled               = 102,
        BadArgument             = 103,
        NullPointer             = 104,
        OutOfMemory             = 105,
        NotEnoughDiskSpace      = 106,
        NotImplemented          = 107,
        FailToCreateCacheFolder = 108,
        NotInitialized          = 109,

        // file and folder
        FileNotExist       = 1001,
        FileOpenFailed     = 1002,
        FileCorrupt        = 1003,
        EndOfFile          = 1004,    // end of file
        NoValidFiles       = 1005,
        CreateFolderFailed = 1006,
        FileAlreadyExist   = 1007,
        ReadOnly           = 1008,
        FolderNotExist     = 1009,
        InvalidFilePath    = 1010,

        // algorithms
        TooFewPoints = 10000,    ///< too few points

        // import
        ThreadRunning          = 100000,    // must wait until the current import finishes
        UnknownFileFormat      = 100001,
        PluginInterfaceNotImpl = 100002,
        LegacyFileFormat       = 100003,    // the file format is legacy but it can be loaded while some features are not supported
        DeprecatedFileFormat   = 100004,    // the file format is too old, so it can't be loaded
        FutureFileFormat       = 100005,    // can't support future format

        Indexing               = 100011,    // the file is being indexed
        IndexCancelled         = 100012,    // the file is cancelled
        IndexFailed            = 100013,    // index failed for this file
        EmptyFile              = 100014,    // empty file
        IndexInvalidFileFormat = 100015,    // invalid file format
        NotIndexed             = 100016,    // the file hasn't been indexed
        DuplicateFiles         = 100017,    // the file has already exist
        NameNotMatch           = 100018,

        // project
        LegacyProject  = 100101,    // legacy project
        FutureProject  = 100102,    // later project version
        InvalidProject = 100103,    // invalid project

        PhotoProject = 100110,    // photo project

        // coordinate system
        UnknownCoordSystem     = 100201,
        UnsupportedCoordSystem = 100202,
        LoadCoordSystemFailed  = 100203,
        NotLidarData           = 100204,
    };

}}}    // namespace Autodesk::RealityComputing::Foundation
