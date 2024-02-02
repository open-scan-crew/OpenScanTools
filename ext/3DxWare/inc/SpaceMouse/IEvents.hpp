#ifndef IEvents_HPP_INCLUDED
#define IEvents_HPP_INCLUDED
// <copyright file="IEvents.hpp" company="3Dconnexion">
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
// $Id: IEvents.hpp 16047 2019-04-05 12:51:24Z mbonk $
//
// </history>
#include <navlib/navlib_types.h>

//stdlib
#include <string>

namespace TDx {
namespace SpaceMouse {
namespace Navigation3D {
/// <summary>
/// The Events interface
/// </summary>
class IEvents {
public:
  /// <summary>
  /// Is called when the user invokes an application command from the SpaceMouse.
  /// </summary>
  /// <param name="commandId">The id of the command to invoke.</param>
  /// <returns>The result of the function: 0 = no error, otherwise &lt;0.</returns>
  virtual long SetActiveCommand(std::string commandId) = 0;

  /// <summary>
  /// Is called when the navigation settings change.
  /// </summary>
  /// <param name="count">The change count.</param>
  /// <returns>0 = no error, otherwise &lt;0.</returns>
  virtual long SetSettingsChanged(long count) = 0;

  /// <summary>
  /// Is invoked when the user releases a key on the 3D Mouse, which has been programmed to send a
  /// virtual key code.
  /// </summary>
  /// <param name="vkey">The virtual key code of the key pressed.</param>
  /// <returns>0 = no error, otherwise &lt;0.</returns>
  virtual long SetKeyPress(long vkey) = 0;

  /// <summary>
  /// Is invoked when the user releases a key on the 3D Mouse, which has been programmed to send a
  /// virtual key code.
  /// </summary>
  /// <param name="vkey">The virtual key code of the key released.</param>
  /// <returns>0 = no error, otherwise &lt;0.</returns>
  virtual long SetKeyRelease(long vkey) = 0;
};
} // namespace Navigation3D
} // namespace SpaceMouse
} // namespace TDx
#endif // IEvents_HPP_INCLUDED