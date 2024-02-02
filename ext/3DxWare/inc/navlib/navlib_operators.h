#ifndef NAVLIB_OPERATORS_H_INCLUDED_
#define NAVLIB_OPERATORS_H_INCLUDED_
// <copyright file="navlib_operators.h" company="3Dconnexion">
// -------------------------------------------------------------------------------------------------
// Copyright (c) 2014-2019 3Dconnexion. All rights reserved.
//
// This file and source code are an integral part of the "3Dconnexion Software Developer Kit",
// including all accompanying documentation, and is protected by intellectual property laws. All use
// of the 3Dconnexion Software Developer Kit is subject to the License Agreement found in the
// "LicenseAgreementSDK.txt" file. All rights not expressly granted by 3Dconnexion are reserved.
// -------------------------------------------------------------------------------------------------
// </copyright>
// <history>
// *************************************************************************************************
// File History
//
// $Id: navlib_operators.h 17364 2020-04-22 09:15:55Z mbonk $
//
// 01/23/14 MSB Initial design
// </history>
// <description>
// *************************************************************************************************
// File Description
//
// This header file defines the operator overloads for variable types used in the 3dconnexion
// interface.
//
// *************************************************************************************************
// </description>
#include <navlib/navlib_types.h>

// stdlib
#include <float.h>
#include <math.h>

_NAVLIB_BEGIN

/// <summary>
/// Compare floating point numbers.
/// </summary>
/// <param name="a">First value to compare.</param>
/// <param name="b">Second value to compare.</param>
/// <param name="epsilon">Maximum relative error.</param>
/// <returns></returns>
/// <remarks>
/// From https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition. 
/// Copyright Bruce Dawson.
/// </remarks>
template <class T> bool equals(T a, T b, T epsilon = static_cast<T>(FLT_EPSILON)) {
  T diff = fabs(a - b);
  if (diff < epsilon) {
    return true;
  }
  a = fabs(a);
  b = fabs(b);
  T largest = (a > b) ? a : b;
  if (diff <= largest * epsilon) {
    return true;
  }
  return false;
}

inline bool operator==(const vector_t &lhs, const vector_t &rhs) {
  return (equals(lhs.x, rhs.x) && equals(lhs.y, rhs.y) && equals(lhs.z, rhs.z));
}

inline bool operator!=(const vector_t &lhs, const vector_t &rhs) {
  return !(lhs == rhs);
}

inline bool operator==(const point_t &lhs, const point_t &rhs) {
  return (equals(lhs.x, rhs.x) && equals(lhs.y, rhs.y) && equals(lhs.z, rhs.z));
}

inline bool operator!=(const point_t &lhs, const point_t &rhs) {
  return !(lhs == rhs);
}

inline bool operator==(const box_t &lhs, const box_t &rhs) {
  return lhs.min == rhs.min && lhs.max == rhs.max;
}

inline bool operator!=(const box_t &lhs, const box_t &rhs) {
  return !(lhs == rhs);
}

inline bool operator==(const matrix_t &lhs, const matrix_t &rhs) {
  for (size_t i = 0; i < sizeof(rhs.m) / sizeof(rhs.m[0]); ++i) {
    if (!equals(lhs.m[i], rhs.m[i])) {
      return false;
    }
  }
  return true;
}

inline bool operator!=(const matrix_t &lhs, const matrix_t &rhs) {
  return !(lhs == rhs);
}

inline nlOptions_t operator|(nlOptions_t lhs, nlOptions_t rhs) {
  return static_cast<nlOptions_t>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

_NAVLIB_END

#endif /* NAVLIB_OPERATORS_H_INCLUDED_ */