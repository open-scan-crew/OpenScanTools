#ifndef IAccessors_HPP_INCLUDED
#define IAccessors_HPP_INCLUDED
// <copyright file="IAccessors.hpp" company="3Dconnexion">
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
// $Id: IAccessors.hpp 16047 2019-04-05 12:51:24Z mbonk $
//
// </history>

#include <SpaceMouse/IEvents.hpp>
#include <SpaceMouse/IHit.hpp>
#include <SpaceMouse/IModel.hpp>
#include <SpaceMouse/IPivot.hpp>
#include <SpaceMouse/ISpace3D.hpp>
#include <SpaceMouse/IState.hpp>
#include <SpaceMouse/IView.hpp>

namespace TDx {
namespace SpaceMouse {
namespace Navigation3D {
/// <summary>
/// The accessor interface to the client 3D properties.
/// </summary>
class IAccessors : public ISpace3D,
                    public IView,
                    public IModel,
                    public IPivot,
                    public IHit,
                    public IEvents,
                    public IState {};
} // namespace Navigation3D
} // namespace SpaceMouse
} // namespace TDx
#endif // IAccessors_HPP_INCLUDED