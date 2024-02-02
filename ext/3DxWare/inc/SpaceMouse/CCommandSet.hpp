#ifndef CCommandSet_HPP_INCLUDED
#define CCommandSet_HPP_INCLUDED
// <copyright file="CCommandSet.hpp" company="3Dconnexion">
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
// $Id: CCommandSet.hpp 16056 2019-04-10 13:42:31Z mbonk $
//
// </history>
#include <SpaceMouse/CCommandTreeNode.hpp>

namespace TDx {
namespace SpaceMouse {
/// <summary>
/// The helper class implements the <see cref="SiActionNodeType_t::SI_ACTIONSET_NODE"/> node type.
/// </summary>
class CCommandSet : public CCommandTreeNode {
  typedef CCommandTreeNode base_type;

public:
  CCommandSet() {
  }

  explicit CCommandSet(std::string id, std::string name)
      : base_type(std::move(id), std::move(name), SiActionNodeType_t::SI_ACTIONSET_NODE) {
  }
#if defined(_MSC_VER) && _MSC_VER < 1900
  CCommandSet(CCommandSet &&other) NOEXCEPT : base_type(std::forward<base_type>(other)) {
  }
  CCommandSet &operator=(CCommandSet &&other) NOEXCEPT {
    base_type::operator=(std::forward<base_type>(other));
    return *this;
  }
#else
  CCommandSet(CCommandSet &&) = default;
  CCommandSet &operator=(CCommandSet &&) = default;
#endif
};
} // namespace SpaceMouse
} // namespace TDx

#endif // CCommandSet_HPP_INCLUDED