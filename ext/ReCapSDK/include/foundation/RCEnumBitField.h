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

#include <type_traits>

template <typename T>
struct EnableBitField
{
    static constexpr bool value = false;
};

#define RC_ENABLE_BITFIELD(T)               \
    template <>                             \
    struct EnableBitField<T>                \
    {                                       \
        static constexpr bool value = true; \
    };

template <typename T>
typename std::enable_if<std::is_enum<T>::value && EnableBitField<T>::value, T>::type operator&(T lhs, T rhs)
{
    typedef typename std::underlying_type<T>::type integer_type;
    return static_cast<T>(static_cast<integer_type>(lhs) & static_cast<integer_type>(rhs));
}

template <typename T>
typename std::enable_if<std::is_enum<T>::value && EnableBitField<T>::value, T>::type operator|(T lhs, T rhs)
{
    typedef typename std::underlying_type<T>::type integer_type;
    return static_cast<T>(static_cast<integer_type>(lhs) | static_cast<integer_type>(rhs));
}

template <typename T>
typename std::enable_if<std::is_enum<T>::value && EnableBitField<T>::value, T>::type operator^(T lhs, T rhs)
{
    typedef typename std::underlying_type<T>::type integer_type;
    return static_cast<T>(static_cast<integer_type>(lhs) ^ static_cast<integer_type>(rhs));
}

template <typename T>
typename std::enable_if<std::is_enum<T>::value && EnableBitField<T>::value, T>::type operator~(T t)
{
    typedef typename std::underlying_type<T>::type integer_type;
    return static_cast<T>(~static_cast<integer_type>(t));
}

template <typename T>
typename std::enable_if<std::is_enum<T>::value && EnableBitField<T>::value, T&>::type operator&=(T& lhs, T rhs)
{
    typedef typename std::underlying_type<T>::type integer_type;
    lhs = static_cast<T>(static_cast<integer_type>(lhs) & static_cast<integer_type>(rhs));
    return lhs;
}

template <typename T>
typename std::enable_if<std::is_enum<T>::value && EnableBitField<T>::value, T&>::type operator|=(T& lhs, T rhs)
{
    typedef typename std::underlying_type<T>::type integer_type;
    lhs = static_cast<T>(static_cast<integer_type>(lhs) | static_cast<integer_type>(rhs));
    return lhs;
}

template <typename T>
typename std::enable_if<std::is_enum<T>::value && EnableBitField<T>::value, T&>::type operator^=(T& lhs, T rhs)
{
    typedef typename std::underlying_type<T>::type integer_type;
    lhs = static_cast<T>(static_cast<integer_type>(lhs) ^ static_cast<integer_type>(rhs));
    return lhs;
}
