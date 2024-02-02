//  Copyright 2020 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form

#pragma once

#include <stddef.h>
#include "adesk.h"
#include "PAL/api/def.h"


typedef void* AcHeapHandle;
ACPAL_PORT AcHeapHandle acHeapCreate(Adesk::UInt32 flags);
ACPAL_PORT void acHeapDestroy(AcHeapHandle heap);
ACPAL_PORT void* acHeapAlloc(AcHeapHandle heap, size_t size);
ACPAL_PORT void* acTryHeapAlloc(AcHeapHandle heap, size_t size);
ACPAL_PORT void acHeapFree(AcHeapHandle heap, void* p);
ACPAL_PORT void* acHeapReAlloc(AcHeapHandle heap, void* p, size_t size);
ACPAL_PORT size_t acHeapSize(AcHeapHandle heap, const void* p);
ACPAL_PORT bool acHeapValidate(AcHeapHandle heap, const void* p);

ACPAL_PORT void* acAllocAligned(size_t alignment, size_t size);
ACPAL_PORT void acFreeAligned(void* p);
ACPAL_PORT size_t acMsizeAligned(void* p, size_t alignment);
