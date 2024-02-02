#ifndef CCategory_HPP_INCLUDED
#define CCategory_HPP_INCLUDED
// <copyright file="CCategory.hpp" company="3Dconnexion">
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
// $Id: CCategory.hpp 16056 2019-04-10 13:42:31Z mbonk $
//
// </history>
#include <SpaceMouse/CCommandTreeNode.hpp>

namespace TDx {
/// <summary>
/// Contains types used for programming the SpaceMouse.
/// </summary>
namespace SpaceMouse {
/// <summary>
/// The helper class implements the <see cref="SiActionNodeType_t::SI_CATEGORY_NODE"/> node type.
/// </summary>
class CCategory : public CCommandTreeNode {
  typedef CCommandTreeNode base_type;

public:
  CCategory() {
  }

  explicit CCategory(std::string id, std::string name)
      : base_type(std::move(id), std::move(name), SiActionNodeType_t::SI_CATEGORY_NODE) {
  }

#if defined(_MSC_VER) && _MSC_VER < 1900
  CCategory(CCategory &&other) NOEXCEPT : base_type(std::forward<base_type>(other)) {
  }
  CCategory &operator=(CCategory &&other) NOEXCEPT {
    base_type::operator=(std::forward<base_type>(other));
    return *this;
  }
#else
  CCategory(CCategory &&) = default;
  CCategory &operator=(CCategory &&) = default;
#endif
};
} // namespace SpaceMouse
} // namespace TDx

#endif // CCategory_HPP_INCLUDED