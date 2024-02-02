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
    public enum  class RCCode
    {
        rcUnknown = static_cast<int>(NS_RCFoundation::RCCode::Unknown),

        // bool
        rcFalse = static_cast<int>(NS_RCFoundation::RCCode::False),
        rcTrue = static_cast<int>(NS_RCFoundation::RCCode::True),

        // operation
        rcOK = static_cast<int>(NS_RCFoundation::RCCode::OK),
        rcFailed = static_cast<int>(NS_RCFoundation::RCCode::Failed),
        rcCancelled = static_cast<int>(NS_RCFoundation::RCCode::Cancelled),
        rcBadArgument = static_cast<int>(NS_RCFoundation::RCCode::BadArgument),
        rcNullPtr = static_cast<int>(NS_RCFoundation::RCCode::NullPointer),
        rcOutOfMemory = static_cast<int>(NS_RCFoundation::RCCode::OutOfMemory),
        rcNotEnoughDiskSpace = static_cast<int>(NS_RCFoundation::RCCode::NotEnoughDiskSpace),
        rcNotImplemented = static_cast<int>(NS_RCFoundation::RCCode::NotImplemented),
        rcFailToCreateCacheFolder = static_cast<int>(NS_RCFoundation::RCCode::FailToCreateCacheFolder),
        rcNotInitialized = static_cast<int>(NS_RCFoundation::RCCode::NotInitialized),

        // file and folder
        rcFileNotExist = static_cast<int>(NS_RCFoundation::RCCode::FileNotExist),
        rcFileOpenFailed = static_cast<int>(NS_RCFoundation::RCCode::FileOpenFailed),
        rcFileCorrupt = static_cast<int>(NS_RCFoundation::RCCode::FileCorrupt),
        rcEOF = static_cast<int>(NS_RCFoundation::RCCode::EndOfFile),    // end of file
        rcNoValidFiles = static_cast<int>(NS_RCFoundation::RCCode::NoValidFiles),
        rcCreateFolderFailed = static_cast<int>(NS_RCFoundation::RCCode::CreateFolderFailed),
        rcFileAlreadyExist = static_cast<int>(NS_RCFoundation::RCCode::FileAlreadyExist),
        rcReadOnly = static_cast<int>(NS_RCFoundation::RCCode::ReadOnly),
        rcFolderNotExist = static_cast<int>(NS_RCFoundation::RCCode::FolderNotExist),
        rcInvalidFilePath = static_cast<int>(NS_RCFoundation::RCCode::InvalidFilePath),

        // algorithms
        rcTooFewPoints = static_cast<int>(NS_RCFoundation::RCCode::TooFewPoints),    ///< too few points

        // import
        rcThreadRunning = static_cast<int>(NS_RCFoundation::RCCode::ThreadRunning),    // must wait until the current import finishes
        rcUnknownFileFormat = static_cast<int>(NS_RCFoundation::RCCode::UnknownFileFormat),
        rcPluginInterfaceNotImpl = static_cast<int>(NS_RCFoundation::RCCode::PluginInterfaceNotImpl),
        rcLegacyFileFormat = static_cast<int>(NS_RCFoundation::RCCode::LegacyFileFormat),    // the file format is legacy but it can be loaded while some features are not supported
        rcDeprecatedFileFormat = static_cast<int>(NS_RCFoundation::RCCode::DeprecatedFileFormat),    // the file format is too old, so it can't be loaded
        rcFutureFileFormat = static_cast<int>(NS_RCFoundation::RCCode::FutureFileFormat),    // can't support future format

        rcIndexing = static_cast<int>(NS_RCFoundation::RCCode::Indexing),    // the file is being indexed
        rcIndexCancelled = static_cast<int>(NS_RCFoundation::RCCode::IndexCancelled),    // the file is cancelled
        rcIndexFailed = static_cast<int>(NS_RCFoundation::RCCode::IndexFailed),    // index failed for this file
        rcEmptyFile = static_cast<int>(NS_RCFoundation::RCCode::EmptyFile),    // empty file
        rcIndexInvalidFileFormat = static_cast<int>(NS_RCFoundation::RCCode::IndexInvalidFileFormat),    // invalid file format
        rcNotIndexed = static_cast<int>(NS_RCFoundation::RCCode::NotIndexed),    // the file hasn't been indexed
        rcDuplicateFiles = static_cast<int>(NS_RCFoundation::RCCode::DuplicateFiles),    // the file has already exist

        // project
        rcLegacyProject = static_cast<int>(NS_RCFoundation::RCCode::LegacyProject),    // legacy project
        rcFutureProject = static_cast<int>(NS_RCFoundation::RCCode::FutureProject),    // later project version
        rcInvalidProject = static_cast<int>(NS_RCFoundation::RCCode::InvalidProject),    // invalid project

        rcPhotoProject = static_cast<int>(NS_RCFoundation::RCCode::PhotoProject),    // photo project

        // coordinate system
        rcUnknownCoordSystem = static_cast<int>(NS_RCFoundation::RCCode::UnknownCoordSystem),
        rcUnsupportedCoordSystem = static_cast<int>(NS_RCFoundation::RCCode::UnsupportedCoordSystem),
        rcLoadCoordSystemFailed = static_cast<int>(NS_RCFoundation::RCCode::LoadCoordSystemFailed),
        rcNotLidarData = static_cast<int>(NS_RCFoundation::RCCode::NotLidarData),
    };
}}}    // namespace Autodesk::RealityComputing::Managed