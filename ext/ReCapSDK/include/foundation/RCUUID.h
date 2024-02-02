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

#include <foundation/RCCommonDef.h>

namespace Autodesk { namespace RealityComputing { namespace Foundation {
    class RCString;
    /// \brief Unique ID class
    ///
    /// Firewall around boost for 128-bit GUID generation and comparison.
    ///
    class RC_COMMON_API RCUUID
    {
    public:
        /// Default constructor creates a new GUID.
        RCUUID();
        RCUUID(const char* data, const size_t sz);
        virtual ~RCUUID();

        bool operator<(const RCUUID& that) const;
        bool operator==(const RCUUID& that) const;
        bool operator!=(const RCUUID& that) const;

        RCUUID(const RCUUID& that);
        void operator=(const RCUUID& that);

        RCUUID(const char* string);
        const char* getString() const;
        const wchar_t* getWString() const;

        const char* getData() const;
        static size_t size();

        static RCUUID nil();
        bool isNil() const;

        // Used for hash maps & lookups
        unsigned long long operator()(RCUUID const& val) const;
        unsigned long long getHash() const;

        // Legacy Conversion Functions
        RCString toLegacyString() const;
        static RCUUID fromLegacyString(const char* data, const size_t sz);
        static RCUUID fromLegacyString(const wchar_t* data, const size_t sz);

    private:
        class RCUUIDImpl;
        RCUUIDImpl* mImpl;
    };
}}}    // namespace Autodesk::RealityComputing::Foundation
