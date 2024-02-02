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

#define VECTOR_CAST(TYPE, OBJECT) (*reinterpret_cast<TYPE*>(&(OBJECT)))

#define VECTOR_CCAST(TYPE, OBJECT) (*reinterpret_cast<const TYPE*>(&(OBJECT)))

#define RC_PIMPL_DECLARATION(CLASS)       \
                                          \
public:                                   \
    CLASS();                              \
    virtual ~CLASS();                     \
    CLASS(const CLASS& other);            \
    CLASS& operator=(const CLASS& other); \
                                          \
private:                                  \
    class Impl;                           \
    Impl* mImpl = nullptr;

#define RC_PIMPL(CLASS)                                \
    CLASS::CLASS() : mImpl(new Impl)                   \
    {                                                  \
    }                                                  \
    CLASS::~CLASS()                                    \
    {                                                  \
        if (nullptr != mImpl)                          \
            delete mImpl;                              \
    }                                                  \
    CLASS::CLASS(const CLASS& other) : mImpl(new Impl) \
    {                                                  \
        *this = other;                                 \
    }                                                  \
    CLASS& CLASS::operator=(const CLASS& other)        \
    {                                                  \
        *mImpl = *(other.mImpl);                       \
        return *this;                                  \
    }

#define RC_PIMPL_WITH_BASE(CLASS, BASE)                             \
    CLASS::CLASS() : mImpl(new Impl)                                \
    {                                                               \
    }                                                               \
    CLASS::~CLASS()                                                 \
    {                                                               \
        if (nullptr != mImpl)                                       \
            delete mImpl;                                           \
    }                                                               \
    CLASS::CLASS(const CLASS& other) : BASE(other), mImpl(new Impl) \
    {                                                               \
        *this = other;                                              \
    }                                                               \
    CLASS& CLASS::operator=(const CLASS& other)                     \
    {                                                               \
        BASE::operator=(other);                                     \
        *mImpl        = *(other.mImpl);                             \
        return *this;                                               \
    }
