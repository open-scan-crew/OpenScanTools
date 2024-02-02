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

//Scope RCSharePtr to use it in .Net applications

#include "globals.h"

namespace Autodesk { namespace RealityComputing { namespace Managed {
    template <class T>
    public ref class RCScopedPointer sealed
    {
        NS_RCFoundation::RCSharedPtr<T>* pPtr;

    public:
        RCScopedPointer() : pPtr(new NS_RCFoundation::RCSharedPtr<T>())
        {
        }

        RCScopedPointer(T* t)
        {
            pPtr = new NS_RCFoundation::RCSharedPtr<T>(t);
        }

        RCScopedPointer(NS_RCFoundation::RCSharedPtr<T> t)
        {
            pPtr = new NS_RCFoundation::RCSharedPtr<T>(t);
        }

        RCScopedPointer(const RCScopedPointer<T> % t)
        {
            pPtr = new NS_RCFoundation::RCSharedPtr<T>(*t.pPtr);
        }

        !RCScopedPointer()
        {
            if (pPtr != nullptr)
            {
                delete pPtr;
                pPtr = nullptr;
            }
        }

        ~RCScopedPointer()
        {
            if (pPtr != nullptr)
            {
                delete pPtr;
                pPtr = nullptr;
            }
        }

        operator NS_RCFoundation::RCSharedPtr<T>()
        {
            return *pPtr;
        }

        RCScopedPointer<T> % operator=(T* ptr)
        {
            delete pPtr;
            pPtr = new NS_RCFoundation::RCSharedPtr<T>(ptr);
            return *this;
        }

        T* operator->()
        {
            return (*pPtr).get();
        }

        void Reset()
        {
            pPtr->reset();
        }

        const NS_RCFoundation::RCSharedPtr<T>& Get()
        {
            return *pPtr;
        }

        NS_RCFoundation::RCSharedPtr<T>* Share()
        {
            return new NS_RCFoundation::RCSharedPtr<T>(*pPtr);
        }

        RCScopedPointer<T> % operator=(const NS_RCFoundation::RCSharedPtr<T>& t)
        {
            delete pPtr;
            pPtr = new NS_RCFoundation::RCSharedPtr<T>(t);
            return *this;
        }
    };
}}}    // namespace Autodesk::RealityComputing::Managed