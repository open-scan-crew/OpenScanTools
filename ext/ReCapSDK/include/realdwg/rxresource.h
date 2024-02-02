//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2020 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license
//  agreement provided at the time of installation or download, or which
//  otherwise accompanies this software in either electronic or hard copy form.
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _RXRESOURCE_H_
#define _RXRESOURCE_H_
#pragma once

#include "acbasedefs.h"
#include "PAL/api/def.h"
#include "adesk.h"
#include "pimplapi.h"
#include "AcHeapOpers.h"

class AcString;
class AcLocale;

#ifdef _ADESK_CROSS_PLATFORM_
#define ACBASE_NON_CROSSPLATFORM ADESK_DEPRECATED
#else
#define ACBASE_NON_CROSSPLATFORM 
#endif

#undef PAL
namespace Autodesk { namespace AutoCAD {namespace PAL { class AcRxResourceInstanceImp; } } }

class AcRxResourceInstance 
    : public Pimpl::ApiPart<AcHeapOperators, Autodesk::AutoCAD::PAL::AcRxResourceInstanceImp>
{
public:

    ACBASE_PORT explicit AcRxResourceInstance(const wchar_t* path);

    /// <summary> 
    /// Creates an AcRxResourceInstance using the locale and resource DLL name passed in. 
    /// </summary>
    /// <param name="locale">  
    /// Locale of resource to load
    /// </param>
    /// <param name="resDllName">  
    /// Name of resource DLL to load
    /// </param>
    ACBASE_PORT AcRxResourceInstance(const AcLocale& locale, const wchar_t* resDllName);

    //for compatibility with existing code, do not use in new code
    ACBASE_PORT ACBASE_NON_CROSSPLATFORM AcRxResourceInstance(void* hInst);

    ACBASE_PORT AcRxResourceInstance(const AcRxResourceInstance& other);

    ACBASE_PORT ~AcRxResourceInstance();

    /// <summary>
    /// Move constructor
    /// </summary>
    AcRxResourceInstance(AcRxResourceInstance&&) = delete;

    /// <summary>
    /// Assignment operator
    /// </summary>
    AcRxResourceInstance& operator= (AcRxResourceInstance&&) = delete;

    // disable copy assignment operator
    AcRxResourceInstance& operator= (const AcRxResourceInstance& other) = delete;

    ACBASE_PORT bool tryLoadString(Adesk::Int32 id, AcString& out) const noexcept;
    ACBASE_PORT bool isLoaded() const noexcept;
    
    ACBASE_PORT bool loadDataResource(Adesk::Int32 id, unsigned long& resourceSize, const void*& data) const noexcept;

    /// <summary>
    /// For Autodesk internal use only
    /// frees a resource loaded by loadDataResource.
    /// </summary>
    /// 
    /// <param name="data">a reference to a pointer set by loadDataResource</param>
    ACBASE_PORT void freeDataResource(const void*& data) const noexcept;

    /// <summary>
    /// Static accessor for a default empty AcRxResourceInstance
    /// </summary>
    /// 
    /// <returns>
    /// Returns a const reference to the the global empty default AcRxResourceInstance
    /// </returns>

    ACBASE_PORT const static AcRxResourceInstance& empty();
};
#endif 