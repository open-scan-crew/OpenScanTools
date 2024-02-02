#ifndef CCommand_HPP_INCLUDED
#define CCommand_HPP_INCLUDED
// <copyright file="CCommand.hpp" company="3Dconnexion">
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
// $Id: CCommand.hpp 16056 2019-04-10 13:42:31Z mbonk $
//
// </history>
#include <SpaceMouse/CCommandTreeNode.hpp>

namespace TDx {
namespace SpaceMouse {
/// <summary>
/// The <see cref="CCommand"/> class implements the application command node.
/// </summary>
class CCommand : public CCommandTreeNode {
  typedef CCommandTreeNode base_type;

public:
  CCommand() {
  }

  explicit CCommand(std::string id, std::string name, std::string description)
      : base_type(std::move(id), std::move(name), std::move(description),
                  SiActionNodeType_t::SI_ACTION_NODE) {
  }
  explicit CCommand(std::string id, std::string name)
      : base_type(std::move(id), std::move(name), SiActionNodeType_t::SI_ACTION_NODE) {
  }
#if defined(_MSC_VER) && _MSC_VER < 1900
  CCommand(CCommand &&other) NOEXCEPT : base_type(std::forward<base_type>(other)) {
  }
  CCommand &operator=(CCommand &&other) NOEXCEPT {
    base_type::operator=(std::forward<base_type>(other));
    return *this;
  }
#else
  CCommand(CCommand &&) = default;
  CCommand &operator=(CCommand &&) = default;
#endif
};
} // namespace SpaceMouse
} // namespace TDx

#endif // CCommand_HPP_INCLUDED