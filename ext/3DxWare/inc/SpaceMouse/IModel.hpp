#ifndef IModel_HPP_INCLUDED
#define IModel_HPP_INCLUDED
// <copyright file="IModel.hpp" company="3Dconnexion">
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
// $Id: IModel.hpp 16047 2019-04-05 12:51:24Z mbonk $
//
// </history>
#include <navlib/navlib_types.h>

namespace TDx {
namespace SpaceMouse {
namespace Navigation3D {
/// <summary>
/// The Model interface
/// </summary>
class IModel {
public:
  /// <summary>
  /// Is called when the navigation library needs to get the extents of the model.
  /// </summary>
  /// <param name="extents">A <see cref="navlib::box_t"/> representing the extents of the
  /// model.</param>
  /// <returns>0 = no error, otherwise &lt;0.</returns>
  virtual long GetModelExtents(navlib::box_t &extents) const = 0;

  /// <summary>
  /// Is called when the navigation library needs to get the extents of the selection.
  /// </summary>
  /// <param name="extents">A <see cref="navlib::box_t"/> representing the extents of the
  /// selection.</param>
  /// <returns>0 = no error, otherwise &lt;0.</returns>
  virtual long GetSelectionExtents(navlib::box_t &extents) const = 0;

  /// <summary>
  /// Is called to get the selections's transform <see cref="navlib::matrix_t"/>.
  /// </summary>
  /// <param name="transform">The world affine <see cref="navlib::matrix_t"/> of the
  /// selection.</param>
  /// <returns>0 = no error, otherwise &lt;0.</returns>
  virtual long GetSelectionTransform(navlib::matrix_t &transform) const = 0;

  /// <summary>
  /// Is called to query if the selection is empty.
  /// </summary>
  /// <param name="empty">true if nothing is selected.</param>
  /// <returns>0 = no error, otherwise &lt;0.</returns>
  virtual long GetIsSelectionEmpty(navlib::bool_t &empty) const = 0;

  /// <summary>
  /// Is called to set the selections's transform <see cref="navlib::matrix_t"/>.
  /// </summary>
  /// <param name="matrix">The world affine <see cref="navlib::matrix_t"/> of the selection.</param>
  /// <returns>0 = no error, otherwise &lt;0.</returns>
  virtual long SetSelectionTransform(const navlib::matrix_t& matrix) = 0;
};
} // namespace Navigation3D
} // namespace SpaceMouse
} // namespace TDx
#endif // IModel_HPP_INCLUDED