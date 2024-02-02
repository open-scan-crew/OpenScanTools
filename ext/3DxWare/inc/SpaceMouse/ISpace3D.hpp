#ifndef ISpace3D_HPP_INCLUDED
#define ISpace3D_HPP_INCLUDED
// <copyright file="ISpace3D.hpp" company="3Dconnexion">
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
// $Id: ISpace3D.hpp 16047 2019-04-05 12:51:24Z mbonk $
//
// </history>
#include <navlib/navlib_types.h>

namespace TDx {
namespace SpaceMouse {
namespace Navigation3D {
/// <summary>
/// The interface to access the client coordinate system
/// </summary>
class ISpace3D {
public:
  /// <summary>
  /// Gets the coordinate system used by the client.
  /// </summary>
  /// <param name="matrix">The coordinate system <see cref="navlib::matrix_t"/>.</param>
  /// <returns>0 = no error, otherwise &lt;0.</returns>
  /// <remarks>The matrix describes the applications coordinate frame in the navlib coordinate
  /// system. i.e. the application to navlib transform.</remarks>
  virtual long GetCoordinateSystem(navlib::matrix_t &matrix) const = 0;

  /// <summary>
  /// Gets the orientation of the front view.
  /// </summary>
  /// <param name="matrix">The front view transform <see cref="navlib::matrix_t"/>.</param>
  /// <returns>0 = no error, otherwise &lt;0.</returns>
  virtual long GetFrontView(navlib::matrix_t &matrix) const = 0;
};
} // namespace Navigation3D
} // namespace SpaceMouse
} // namespace TDx
#endif // ISpace3D_HPP_INCLUDED