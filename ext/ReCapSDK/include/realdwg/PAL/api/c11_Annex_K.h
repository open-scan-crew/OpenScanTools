//  Copyright 2020 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form
//
//Description:
//Shim header to provide trivial emulation of C 11 Annex K functions on platforms
//that do not support it.
//http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf
//
#pragma once

#define __STDC_WANT_LIB_EXT1__ 1
#include <wchar.h>
#include <string.h>
#include <cassert>

#if defined(__STDC_LIB_EXT1__) || defined(_MSC_VER) && !defined(_ADESK_MAC_)
//Visual C++ 2015 provides these functions
//
#define snwprintf_s _snwprintf_s    // K.3.9.1.3 The snwprintf_s function in spec
#elif defined (_ADESK_MAC_)
// mac already emulates these
#include <assert.h>
#include "ms_crtdef.h"
#include "ms_new.h" 
#include "ms_string.h"
#include "ms_tchar.h"
#include "ms_stdio.h"
#include "ms_wchar.h"
#include "ms_math.h"
#include "ms_time.h"
#include "mini_adpcore.h"
#include <ctime>
//use MSVC signature instead of standard. The standard looks like this:
//struct tm *gmtime_s(const time_t * restrict timer,struct tm * restrict result);
inline int gmtime_s(std::tm* st, const std::time_t*timer)
{
    return gmtime_r(timer, st) != nullptr ? 0 : 1;
}
#define snwprintf_s _snwprintf_s    // K.3.9.1.3 The snwprintf_s function in spec
#else
#include <stdarg.h>
#include <ctime>
#include <cstdlib>
#include <stdio.h>
#include <type_traits>
#include <errno.h>
#include <algorithm>

typedef int errno_t;
typedef size_t rsize_t;
#define _TRUNCATE ((rsize_t)-1)

inline errno_t memcpy_s(void * s1, rsize_t s1max, const void * s2, rsize_t n)
{
    memcpy(s1, s2, n);
    return 0;
}

inline errno_t memmove_s(void *s1, rsize_t s1max, const void *s2, rsize_t n)
{
    memmove(s1, s2, n);
    return 0;
}

inline errno_t strcpy_s(char * s1, rsize_t s1max, const char * s2)
{
    strcpy(s1, s2);
    return 0;
}

template<rsize_t N> errno_t strcpy_s(char(&s1)[N], const char * s2)
{
    return strcpy_s(s1, N, s2);
}

inline errno_t wcscpy_s(wchar_t * s1, rsize_t s1max, const wchar_t * s2)
{
    wcscpy(s1, s2);
    return 0;
}

template<size_t N> errno_t wcscpy_s(wchar_t(&s1)[N], const wchar_t * s2)
{
    return wcscpy_s(s1, N, s2);
}

//Required for wcsncpy_s
inline rsize_t wcsnlen_s(wchar_t const * str, rsize_t strsz) {
    if ( str == nullptr )
        return 0;

    rsize_t count = 0;

    while ( ( (*str++) != L'\0' ) && ( (++count) <= strsz ) );
    
    return count > strsz ? strsz
                          : count;
}

//Following the implementation in WinStubs\src\string.cpp
inline errno_t wcsncpy_s(wchar_t* dest, rsize_t destSize, const wchar_t* src, rsize_t srcNum)
{
    if (src == nullptr || dest == nullptr)
        return 22; //EINVAL

    if (src == dest)
        return 0;

    if (destSize == 0)
        return 34; //ERANGE

    rsize_t capacity = destSize - 1;

    rsize_t cpyCount = wcsnlen_s(src, srcNum);

    if (srcNum != static_cast<rsize_t>(-1) /*_TRUNCATE*/ && static_cast<rsize_t>(capacity) < cpyCount)
        return 22; //EINVAL;

    cpyCount = std::min(capacity, cpyCount);

    memset(dest, 0, sizeof(wchar_t) * destSize);
    wcsncpy(dest, src, cpyCount);

    return 0;
}

template<size_t N> errno_t wcsncpy_s(wchar_t(&s1)[N], const wchar_t * s2, rsize_t s2Num)
{
    wcsncpy(s1, s2, s2Num);
    return 0;
}

template<size_t N> errno_t strncpy_s(char(&s1)[N], const char * s2, rsize_t s2Num)
{
    strncpy(s1, s2, s2Num);
    return 0;
}

template<size_t N> errno_t wcscat_s(wchar_t(&s1)[N], const wchar_t * s2)
{
    wcscat(s1, s2);
    return 0;
}


inline errno_t wcscat_s(wchar_t * s1, rsize_t s1max, const wchar_t * s2)
{
    wcscat(s1, s2);
    return 0;
}

inline errno_t wcsncat_s(wchar_t* s1, rsize_t s1Size, const wchar_t* s2, rsize_t s2Num)
{
    wcsncat(s1, s2, s2Num);
    return 0;
}

template<rsize_t N> errno_t wcsncat_s(wchar_t(&s1)[N], const wchar_t * s2, rsize_t s2Num)
{
    return wcsncat_s(s1, N, s2, s2Num);
}

inline int sprintf_s(char* s, rsize_t n, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int retVal = vsnprintf(s, n, format, args);
    va_end(args);
    return retVal;
}

template<size_t N> int sprintf_s(char(&s)[N], const char * format, ...)
{
    va_list args;
    va_start(args, format);
    int retVal = vsnprintf(s, N, format, args);
    va_end(args);
    return retVal;
}

inline int swprintf_s(wchar_t * s, rsize_t n, const wchar_t * format, ...)
{
    va_list args;
    va_start(args, format);
    int retVal =  vswprintf(s, n, format, args);
    va_end(args);
    return retVal;
}

template<size_t N> int swprintf_s(wchar_t(&s)[N], const wchar_t * format, ...)
{
    va_list args;
    va_start(args, format);
    int retVal =  vswprintf(s, N, format, args);
    va_end(args);
    return retVal;
}

inline int snwprintf_s(wchar_t * buffer, rsize_t sizeOfBuffer, rsize_t count, const wchar_t * format, ...)
{
    if (format == NULL || buffer == NULL || count < 1) {
        errno = EINVAL;
        return -1;
    }

    memset(buffer, 0, sizeOfBuffer * sizeof(wchar_t));
    va_list va;
    va_start(va, format);

    int	nWritten = vswprintf(buffer, sizeOfBuffer, format, va);
    va_end(va);

    if (count != _TRUNCATE) {
        if (nWritten == -1 && errno == EOVERFLOW) {
            // the required storage exceeds sizeOfBuffer, set buffer to an empty string.
            buffer[0] = 0;
            errno = ERANGE;
        }
        else if (nWritten > 0 && count < (size_t)(nWritten)) {
            buffer[count] = 0;
            nWritten = static_cast<int>(count);
        }
    }

    return nWritten;
}

template<size_t N> int vswprintf_s(wchar_t(&s)[N], const wchar_t* format, va_list args)
{
    return vswprintf(s, N, format, args);
}

inline int vswprintf_s(wchar_t* s, size_t N, const wchar_t* format, va_list args)
{
    return vswprintf(s, N, format, args);
}

inline char* strtok_s(char * strToken, const char * strDelimit, char ** context)
{
    return strtok_r(strToken, strDelimit, context);
}

namespace details {

    class guard {};

    template<bool condition>
    struct condition_switch;

    template <typename Type, typename First, typename... Others>
    int scanf_helper(const Type* buf, const Type* format, First first, Others... others)
    {
        return condition_switch<std::is_integral<First>::value>::work(buf, format, first, others...);
    }

    template <typename... Args>
    int scanf_helper(const wchar_t* buf, const wchar_t* format, guard, Args... args)
    {
        return swscanf(buf, format, args...);
    }

    template <typename... Args>
    int scanf_helper(const char* buf, const char* format, guard, Args... args)
    {
        return sscanf(buf, format, args...);
    }


    template<>
    struct condition_switch<true>
    {
        template <typename Type, typename First, typename... Others>
        static int work(const Type* buf, const Type* format, First first, Others... others)
        {
            return scanf_helper(buf, format, others...);
        }
    };

    template<>
    struct condition_switch<false>
    {
        template <typename Type, typename First, typename... Others>
        static int work(const Type* buf, const Type* format, First first, Others... others)
        {
            return scanf_helper(buf, format, others..., first);
        }
    };

}

template <typename... Args>
int swscanf_s(const wchar_t* buf, const wchar_t* format, Args... args)
{
    // swprintf doesn't work on some android devices/simulators, especially api-level 19
    // put an assert here so that we can avoid wasting time to debug weird behavior caused by it
    // if you run into this assert, try another simulator.
#if defined(_ADESK_ANDROID_)
    int a = 0;
    assert(1 == swscanf(L"1", L"%d", &a));
#endif

    // for swscanf_s, %c, %s, and %[ conversion specifiers each expect two arguments(buffer and buffer size)
    // on windows, if give swscanf_s a parameter of incorrect type, compiler will report warning
    // so it should be safe to filter all parameters of integral type, then pass the rest to swscanf
    return details::scanf_helper(buf, format, args..., details::guard());
}

template <typename... Args>
int sscanf_s(const char* buf, const char* format, Args... args)
{
    // swprintf doesn't work on some android devices/simulators, especially api-level 19
    // put an assert here so that we can avoid wasting time to debug weird behavior caused by it
    // if you run into this assert, try another simulator.
#if defined(_ADESK_ANDROID_)
    int a = 0;
    assert(1 == swscanf(L"1", L"%d", &a));
#endif
    // for swscanf_s, %c, %s, and %[ conversion specifiers each expect two arguments(buffer and buffer size)
    // on windows, if give swscanf_s a parameter of incorrect type, compiler will report warning
    // so it should be safe to filter all parameters of integral type, then pass the rest to swscanf
    return details::scanf_helper(buf, format, args..., details::guard());
}


//we omit the s1max parameter to match the non-standard MSVC signature. -szilvaa 7/1/2015
inline wchar_t *wcstok_s(wchar_t * s1, /*rsize_t * s1max,*/ const wchar_t * s2, wchar_t ** ptr)
{
    return wcstok(s1, s2, ptr);
}
//use MSVC signature instead of standard. The standard looks like this:
//struct tm *gmtime_s(const time_t * restrict timer,struct tm * restrict result);
inline errno_t gmtime_s(std::tm* st, const std::time_t* timer)
{
    return gmtime_r(timer, st) != nullptr ? 0 : 1;
}
inline errno_t localtime_s(std::tm* st, const std::time_t* timer)
{
    // Emscripten does not support localtime_s
    return localtime_r(timer, st) != nullptr ? 0 : 1;
}

inline errno_t fopen_s(FILE**      _Stream,
    char const* _FileName,
    char const* _Mode)
{
    if (_Stream) {
        *_Stream = fopen(_FileName, _Mode);
    }
    return 0;
}

inline int fscanf_s(FILE* const _Stream, char const* const _Format, ...)
{
    va_list args;
    va_start(args, _Format);
    auto retVal = vfscanf(_Stream, _Format, args);
    va_end(args);
    
    return retVal;
}

inline int sscanf_s(char const* const _Buffer, char const* const _Format, ...)
{
    return sscanf(_Buffer, _Format);
}

inline char* _strdup(char const* _Source)
{
    return strdup(_Source);
}

inline errno_t strcat_s(char *strDestination, size_t numberOfElements, const char *strSource)
{
    strcat(strDestination, strSource);
    return 0;
}

inline errno_t strcat_s(char *strDestination, const char *strSource)
{
    strcat(strDestination, strSource);
    return 0;
}
#endif
