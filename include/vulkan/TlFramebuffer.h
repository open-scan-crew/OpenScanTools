#pragma once

#define TL_DEFINE_HANDLE(object) typedef struct object##_T* object;
#define TL_NULL_HANDLE 0

TL_DEFINE_HANDLE(TlFramebuffer)
