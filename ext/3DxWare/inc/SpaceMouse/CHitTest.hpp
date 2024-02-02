#ifndef CHitTest_HPP_INCLUDED
#define CHitTest_HPP_INCLUDED
// <copyright file="CHitTest.hpp" company="3Dconnexion">
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
// $Id: CHitTest.hpp 16056 2019-04-10 13:42:31Z mbonk $
//
// 01/20/14 MSB Initial design
// </history>

// 3dxware
#include <navlib/navlib_types.h>

namespace TDx {
namespace SpaceMouse {
/// <summary>
/// Class can be used to hold the hit-test properties.
/// </summary>
template <class point_ = navlib::point_t, class vector_ = navlib::vector_t> class CHitTest {
public:
  typedef point_ point_type;
  typedef vector_ vector_type;

public:
  /// <summary>
  /// Creates a new instance of the <see cref="CHitTest"/> class.
  /// </summary>
  CHitTest() : m_aperture(1), m_dirty(false), m_selectionOnly(false) {
  }

  /// <summary>
  /// Gets or sets the ray direction.
  /// </summary>
  __declspec(property(get = GetDirection, put = PutDirection)) vector_type Direction;
  void PutDirection(vector_type value) {
    if (!m_dirty) {
      m_dirty = static_cast<bool>(m_direction != value);
    }
    m_direction = std::move(value);
  }
  vector_type GetDirection() const {
    return m_direction;
  }

  /// <summary>
  /// Gets or sets the ray origin.
  /// </summary>
  __declspec(property(get = GetLookFrom, put = PutLookFrom)) point_type LookFrom;
  void PutLookFrom(point_type value) {
    if (!m_dirty) {
      m_dirty = static_cast<bool>(m_lookFrom != value);
    }
    m_lookFrom = std::move(value);
  }
  const point_type GetLookFrom() const {
    return m_lookFrom;
  }

  /// <summary>
  /// Gets or sets the ray hit test result location.
  /// </summary>
  __declspec(property(get = GetLookingAt, put = PutLookingAt)) point_type LookingAt;
  void PutLookingAt(point_type value) {
    m_lookingAt = std::move(value);
    m_dirty = false;
  }
  const point_type GetLookingAt() const {
    return m_lookingAt;
  }

  /// <summary>
  /// Gets or sets a value indicating if a the parameters have changed since the last hit calculation.
  /// </summary>
  __declspec(property(get = GetIsDirty, put = PutIsDirty)) bool IsDirty;
  void PutIsDirty(bool value) {
    m_dirty = value;
  }
  bool GetIsDirty() const {
    return m_dirty;
  }

  /// <summary>
  /// Gets or sets the ray diameter / aperture on the near clipping plane.
  /// </summary>
  __declspec(property(get = GetAperture, put = PutAperture)) double Aperture;
  void PutAperture(double value) {
    m_dirty = (m_aperture != value);
    m_aperture = value;
  }
  double GetAperture() const {
    return m_aperture;
  }

  /// <summary>
  /// Gets or sets a value indicating whether the hit-testing is limited to the selection set.
  /// </summary>
  __declspec(property(get = GetSelectionOnly, put = PutSelectionOnly)) bool SelectionOnly;
  void PutSelectionOnly(bool value) {
    m_selectionOnly = value;
  }
  bool GetSelectionOnly() const {
    return m_selectionOnly;
  }

private:
  double m_aperture;
  mutable bool m_dirty;
  vector_type m_direction;
  point_type m_lookFrom;
  mutable point_type m_lookingAt;
  bool m_selectionOnly;
};
} // namespace SpaceMouse
} // namespace TDx
#endif // CHitTest_HPP_INCLUDED

