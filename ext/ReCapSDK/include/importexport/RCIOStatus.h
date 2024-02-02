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

#include <cstdint>

#include <importexport/RCImportDef.h>
#include <foundation/RCString.h>
#include <foundation/RCBuffer.h>

namespace Autodesk { namespace RealityComputing { namespace Data {

    using Autodesk::RealityComputing::Foundation::RCBuffer;
    using Autodesk::RealityComputing::Foundation::RCString;

    /// \brief An enum class for the I/O states
    enum class RCIOState
    {
        Ready,
        Running,
        Finished,
        Paused,
        Error
    };

    struct RCIOScanStatus
    {
        RCString first;
        uint8_t second;
    };

    ///
    /// \brief A struct for I/O status
    ///
    struct RC_IMPORT_API RCIOStatus
    {
        RCIOStatus();
        ~RCIOStatus();

        void addItem(const RCString& key, uint8_t progress);
        bool getItem(const RCString& key, uint8_t& progress) const;

        void setTotalProgress(uint8_t totalProgress);
        uint8_t getTotalProgress() const;
        RCBuffer<RCIOScanStatus> getAllProgress() const;
        void setAllProgress(const RCBuffer<RCIOScanStatus>&);

        RCIOState getIOState() const;
        void setIOState(const RCIOState& ioState);

        uint8_t operator[](const RCString& key) const;
        size_t size() const;

    private:
        class Impl;
        Impl* mImpl;
    };
}}}    // namespace Autodesk::RealityComputing::Data
