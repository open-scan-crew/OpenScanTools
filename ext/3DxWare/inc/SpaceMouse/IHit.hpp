#ifndef IHit_HPP_INCLUDED
#define IHit_HPP_INCLUDED
// <copyright file="IHit.hpp" company="3Dconnexion">
// ------------------------------------------------------------------------------------------------
// Copyright (c) 2018-2019 3Dconnexion. All rights reserved.
//
// This file and source code are an integral part of the "3Dconnexion Software Developer Kit",
// including all accompanying documentation, and is protected by intellectual property laws. All
// use of the 3Dconnexion Software Developer Kit is subject to the License Agreement found in the
// "LicenseAgreementSDK.txt" file. All rights not expressly granted by 3Dconnexion are reserved.
// ------------------------------------------------------------------------------------------------
// </copyright>
// <history>
// ************************************************************************************************
// File History
//
// $Id: IHit.hpp 16047 2019-04-05 12:51:24Z mbonk $
//
// </history>
#include <navlib/navlib_types.h>

namespace TDx {
namespace SpaceMouse {
namespace Navigation3D {
/// <summary>
/// The hit-testing interface
/// </summary>
class IHit {
public:
  /// <summary>
  /// Is called when the navigation library queries the result of the hit-testing.
  /// </summary>
  /// <param name="position">The hit <see cref="navlib::point_t"/> in world coordinates.</param>
  /// <returns>0 =no error, otherwise &lt;0 <see cref="navlib::make_result_code"/>.</returns>
  virtual long GetHitLookAt(navlib::point_t &position) const = 0;

  /// <summary>
  /// Is called when the navigation library sets the aperture of the hit-testing ray/cone.
  /// </summary>
  /// <param name="aperture">The aperture of the ray/cone on the near plane.</param>
  /// <returns>0 =no error, otherwise &lt;0 <see cref="navlib::make_result_code"/>.</returns>
  virtual long SetHitAperture(double aperture) = 0;

  /// <summary>
  /// Is called when the navigation library sets the direction of the hit-testing ray/cone.
  /// </summary>
  /// <param name="direction">The <see cref="navlib::vector_t"/> direction of the ray/cone.</param>
  /// <returns>0 =no error, otherwise &lt;0 <see cref="navlib::make_result_code"/>.</returns>
  virtual long SetHitDirection(const navlib::vector_t& direction) = 0;

  /// <summary>
  /// Is called when the navigation library sets the source of the hit-testing ray/cone.
  /// </summary>
  /// <param name="eye">The source <see cref="navlib::point_t"/> of the hit cone.</param>
  /// <returns>0 =no error, otherwise &lt;0 <see cref="navlib::make_result_code"/>.</returns>
  virtual long SetHitLookFrom(const navlib::point_t& eye) = 0;

  /// <summary>
  /// Is called when the navigation library sets the selection filter for hit-testing.
  /// </summary>
  /// <param name="onlySelection">true = ignore non-selected items.</param>
  /// <returns>0 =no error, otherwise &lt;0 <see cref="navlib::make_result_code"/>.</returns>
  virtual long SetHitSelectionOnly(bool onlySelection) = 0;
};
} // namespace Navigation3D
} // namespace SpaceMouse
} // namespace TDx
#endif // IHit_HPP_INCLUDED