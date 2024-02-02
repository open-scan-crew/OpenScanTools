#ifndef CNavlibInterface_HPP_INCLUDED
#define CNavlibInterface_HPP_INCLUDED
// <copyright file="CNavilibInterface.hpp" company="3Dconnexion">
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
// $Id: CNavlibInterface.hpp 16051 2019-04-09 11:29:53Z mbonk $
//
// 07/23/19 MSB Do not set the cookie to zero when the open fails.
// </history>
#include <SpaceMouse/CCookieCollection.hpp>
#include <SpaceMouse/IAccessors.hpp>
#include <SpaceMouse/INavlib.hpp>

// stdlib
#include <map>
#include <memory>
#include <string>
#include <vector>

// navlib
#include <navlib/navlib.h>
#include <navlib/navlib_error.h>
#if defined(_DEBUG) && defined(TRACE_NAVLIB)
#include <iostream>
#include <navlib/navlib_ostream.h>
#endif

namespace TDx {
namespace SpaceMouse {
/// <summary>
/// Template to allow defining the static members in the header file
/// </summary>
template <typename Ty_, typename I_> struct StaticSinkCollection {
protected:
  static CCookieCollection<I_> s_sinkCollection;
#if defined(_DEBUG) && defined(TRACE_NAVLIB)
  static std::mutex s_mutex;
#endif
};

template <class Ty_, class I_>
CCookieCollection<I_> StaticSinkCollection<Ty_, I_>::s_sinkCollection;

#if defined(_DEBUG) && defined(TRACE_NAVLIB)
/// <summary>
/// Mutex used to synchronize the trace output.
/// </summary>
template <class Ty_, class I_> std::mutex StaticSinkCollection<Ty_, I_>::s_mutex;
#endif

namespace Navigation3D {
/// <summary>
/// Class implements the interface to the navlib.
/// </summary>
class CNavlibInterface : public INavlib,
                         private StaticSinkCollection<CNavlibInterface, IAccessors> {
public:
  /// <summary>
  /// Initializes a new instance of the CNavlibInterface class.
  /// </summary>
  /// <param name="sink">Shared pointer to the instance implementing the IAccessors interface
  /// accessors and mutators.</param>
  /// <param name="multiThreaded">true to use multi-threading, false for single-threaded.</param>
  /// <param name="rowMajor">true for row-major ordered matrices, false for column-major.</param>
  explicit CNavlibInterface(std::shared_ptr<IAccessors> sink, bool multiThreaded = false,
                            bool rowMajor = false)
      : m_hdl(INVALID_NAVLIB_HANDLE), m_cookie(s_sinkCollection.insert(sink))
#if defined(_MSC_VER) && (_MSC_VER < 1800)
  {
    navlib::nlCreateOptions_t options = {sizeof(navlib::nlCreateOptions_t), multiThreaded,
                                         rowMajor ? navlib::row_major_order : navlib::none};

    m_createOptions = options;
  }
#else
        ,
        m_createOptions{sizeof(navlib::nlCreateOptions_t), multiThreaded,
                        rowMajor ? navlib::row_major_order : navlib::none} {
  }
#endif

  /// <summary>
  /// Initializes a new instance of the CNavlibInterface class.
  /// </summary>
  /// <param name="sink">Shared pointer to the instance implementing the IAccessors interface
  /// accessors and mutators.</param>
  /// <param name="multiThreaded">true to use multi-threading, false for single-threaded.</param>
  /// <param name="options">A combination of the <see cref="navlib::nlOptions_t"/> values.</param>
  explicit CNavlibInterface(std::shared_ptr<IAccessors> sink, bool multiThreaded,
                            navlib::nlOptions_t options)
      : m_hdl(INVALID_NAVLIB_HANDLE), m_cookie(s_sinkCollection.insert(sink))
#if defined(_MSC_VER) && (_MSC_VER < 1800)
  {
    navlib::nlCreateOptions_t createOptions = {sizeof(navlib::nlCreateOptions_t), multiThreaded,
                                               options};

    m_createOptions = createOptions;
  }
#else
        ,
        m_createOptions{sizeof(navlib::nlCreateOptions_t), multiThreaded, options} {
  }
#endif

  /// <summary>
  /// Clean up the resources
  /// </summary>
  virtual ~CNavlibInterface() {
    if (m_cookie) {
      s_sinkCollection.erase(m_cookie);
    }
    if (m_hdl != INVALID_NAVLIB_HANDLE) {
      navlib::NlClose(m_hdl);
    }
  }

public:
  /// <summary>
  /// Close the connection to the 3D navigation library.
  /// </summary>
  void Close() override {
    if (m_hdl != INVALID_NAVLIB_HANDLE) {
      std::unique_lock<std::mutex> lock(m_mutex);
      if (m_hdl != INVALID_NAVLIB_HANDLE) {
        navlib::NlClose(m_hdl);
        m_hdl = INVALID_NAVLIB_HANDLE;
      }
    }
  }

  /// <summary>
  /// Opens a connection to the 3D navigation library.
  /// </summary>
  void Open() override {
    Open(m_name);
  }

  /// <summary>
  /// Opens a connection to the 3D navigation library
  /// </summary>
  /// <param name="profileText">The text to display in the 3Dconnexion profile.</param>
  /// <exception cref="std::system_error">The connection to the library is already open.</exception>
  /// <exception cref="std::system_error">Cannot create a connection to the library.</exception>
  /// <exception cref="std::invalid_argument">The text for the profile is empty.</exception>
  void Open(std::string profileText) override {
    using namespace ::navlib;

    if (profileText.empty()) {
      throw std::invalid_argument("The text for the profile is empty.");
    }

    std::unique_lock<std::mutex> lock(m_mutex);

    if (m_hdl != INVALID_NAVLIB_HANDLE) {
      throw std::system_error(navlib::make_error_code(navlib_errc::already_connected),
                              "Connection to the library is already open.");
    }

    accessor_t accessors[] = {
        {motion_k, nullptr, &CNavlibInterface::SetMotionFlag, m_cookie},
        {transaction_k, nullptr, &CNavlibInterface::SetTransaction, m_cookie},
        {coordinate_system_k, &CNavlibInterface::GetCoordinateSystem, nullptr, m_cookie},
        {views_front_k, &CNavlibInterface::GetFrontView, nullptr, m_cookie},

        // view access
        {view_affine_k, &CNavlibInterface::GetCameraMatrix, &CNavlibInterface::SetCameraMatrix,
         m_cookie},
        {view_constructionPlane_k, &CNavlibInterface::GetViewConstructionPlane, nullptr, m_cookie},
        {view_extents_k, &CNavlibInterface::GetViewExtents, &CNavlibInterface::SetViewExtents,
         m_cookie},
        {view_fov_k, &CNavlibInterface::GetViewFOV, &CNavlibInterface::SetViewFOV, m_cookie},
        {view_frustum_k, &CNavlibInterface::GetViewFrustum, &CNavlibInterface::SetViewFrustum,
         m_cookie},
        {view_perspective_k, &CNavlibInterface::GetIsViewPerspective, nullptr, m_cookie},
        {view_target_k, &CNavlibInterface::GetCameraTarget, &CNavlibInterface::SetCameraTarget,
         m_cookie},
        {view_rotatable_k, &CNavlibInterface::GetIsViewRotatable, nullptr, m_cookie},
        {pointer_position_k, &CNavlibInterface::GetPointerPosition,
         &CNavlibInterface::SetPointerPosition, m_cookie},

        // pivot accessors
        {pivot_position_k, &CNavlibInterface::GetPivotPosition, &CNavlibInterface::SetPivotPosition,
         m_cookie},
        {pivot_user_k, &CNavlibInterface::IsUserPivot, nullptr, m_cookie},
        {pivot_visible_k, &CNavlibInterface::GetPivotVisible, &CNavlibInterface::SetPivotVisible,
         m_cookie},

        // hit testing for auto pivot algorithm etc.
        {hit_lookfrom_k, nullptr, &CNavlibInterface::SetHitLookFrom, m_cookie},
        {hit_direction_k, nullptr, &CNavlibInterface::SetHitDirection, m_cookie},
        {hit_aperture_k, nullptr, &CNavlibInterface::SetHitAperture, m_cookie},
        {hit_lookat_k, &CNavlibInterface::GetHitLookAt, nullptr, m_cookie},
        {hit_selectionOnly_k, nullptr, &CNavlibInterface::SetHitSelectionOnly, m_cookie},

        // model access
        {model_extents_k, &CNavlibInterface::GetModelExtents, nullptr, m_cookie},
        {selection_empty_k, &CNavlibInterface::GetIsSelectionEmpty, nullptr, m_cookie},
        {selection_extents_k, &CNavlibInterface::GetSelectionExtents, nullptr, m_cookie},
        {selection_affine_k, &CNavlibInterface::GetSelectionTransform,
         &CNavlibInterface::SetSelectionTransform, m_cookie},

        // events
        {commands_activeCommand_k, nullptr, &CNavlibInterface::SetActiveCommand, m_cookie},
        {events_keyPress_k, nullptr, &CNavlibInterface::SetKeyPress, m_cookie},
        {events_keyRelease_k, nullptr, &CNavlibInterface::SetKeyRelease, m_cookie},
        {settings_changed_k, nullptr, &CNavlibInterface::SetSettingsChanged, m_cookie}};

    // Create the navlib instance
    long error = NlCreate(&m_hdl, profileText.c_str(), static_cast<const accessor_t *>(accessors),
                          sizeof(accessors) / sizeof(accessors[0]), &m_createOptions);

    if (error != 0) {
      throw std::system_error(
          navlib::make_error_code(static_cast<navlib_errc::navlib_errc_t>(error & 0xffff)),
          "Cannot create a connection to the 3DMouse.");
    }

    m_name = std::move(profileText);
  }

  /// <summary>
  /// Writes the value of a property to the navlib.
  /// </summary>
  /// <param name="propertyName">The <see cref="navlib::property_t"/> name of the navlib property to
  /// write.</param>
  /// <param name="value">The <see cref="navlib::value"/> to write.</param>
  /// <returns>0 =no error, otherwise a value from <see cref="navlib::make_result_code"/>.</returns>
  /// <exception cref="std::system_error">No connection to the navlib.</exception>
  long Write(const std::string &propertyName, const navlib::value &value) override {
    if (m_hdl == INVALID_NAVLIB_HANDLE) {
      throw std::system_error(navlib::make_error_code(navlib::navlib_errc::invalid_operation),
                              "No connection to the navlib");
    }

    long resultCode = WriteValue(m_hdl, propertyName.c_str(), &value);

    return resultCode;
  }

  /// <summary>
  /// Reads the value of a navlib property.
  /// </summary>
  /// <param name="propertyName">The <see cref="navlib::property_t"/> name of the navlib property to
  /// read.</param>
  /// <param name="value">The <see cref="navlib::value"/> to read.</param>
  /// <returns>0 =no error, otherwise a value from <see cref="navlib::make_result_code"/>.</returns>
  /// <exception cref="std::system_error">No connection to the navlib.</exception>
  long Read(const std::string &propertyName, navlib::value &value) const override {
    if (m_hdl == INVALID_NAVLIB_HANDLE) {
      throw std::system_error(navlib::make_error_code(navlib::navlib_errc::invalid_operation),
                              "No connection to the navlib.");
    }

    long resultCode = ReadValue(m_hdl, propertyName.c_str(), &value);

    return resultCode;
  }

  /// <summary>
  /// Reads the value of a navlib string property.
  /// </summary>
  /// <param name="propertyName">The <see cref="navlib::property_t"/> name of the navlib property to
  /// read.</param>
  /// <param name="string">The <see cref="std::string"/> value of the property.</param>
  /// <returns>0 =no error, otherwise a value from <see cref="navlib::make_result_code"/>.</returns>
  /// <exception cref="std::system_error">No connection to the navlib.</exception>
  long Read(const std::string &propertyName, std::string &string) const override {
    if (m_hdl == INVALID_NAVLIB_HANDLE) {
      throw std::system_error(navlib::make_error_code(navlib::navlib_errc::invalid_operation),
                              "No connection to the navlib.");
    }

    navlib::value value(&string[0], string.length());
    long resultCode = ReadValue(m_hdl, propertyName.c_str(), &value);
    if ((resultCode & 0xffff) == static_cast<int>(navlib::navlib_errc::insufficient_buffer)) {
      string.resize(value.string.length);
      value = navlib::value(&string[0], string.length());
      resultCode = ReadValue(m_hdl, propertyName.c_str(), &value);
    }

    return resultCode;
  }

private:
  typedef std::shared_ptr<IAccessors> isink_t;

  template <typename F>
  static long GetValue(navlib::param_t cookie, navlib::property_t property, navlib::value_t *value,
                       F fn) {
    isink_t isink;
    try {
      isink = s_sinkCollection.at(cookie);
      long result = fn(isink);
#if defined(_DEBUG) && defined(TRACE_NAVLIB)
      std::unique_lock<std::mutex> lock(s_mutex);
      std::cout << "GetValue(0x" << std::hex << cookie << std::dec << ", " << property << ", "
                << *value << ") result =0x" << std::hex << result << std::endl;
#endif
      return result;
    }
#if defined(_DEBUG) && defined(TRACE_NAVLIB)
    catch (const std::out_of_range &e) {
      std::unique_lock<std::mutex> lock(s_mutex);
      std::cout << "std::out_of_range exception thrown in GetValue(0x" << std::hex << cookie
                << std::dec << ", " << property << ", value)\n"
                << *value << e.what() << std::endl;
      return navlib::make_result_code(navlib::navlib_errc::invalid_argument);
    } catch (const std::exception &e) {
      std::cout << "Uncaught exception thrown in GetValue(0x" << std::hex << cookie << std::dec
                << ", " << property << ", value)\n"
                << *value << e.what() << std::endl;
    }
#else
    catch (const std::out_of_range &) {
      return navlib::make_result_code(navlib::navlib_errc::invalid_argument);
    } catch (const std::exception &) {
    }
#endif
    return navlib::make_result_code(navlib::navlib_errc::error);
  }

  template <typename F>
  static long SetValue(navlib::param_t cookie, navlib::property_t property,
                       const navlib::value_t *value, F fn) {
    isink_t isink;
    try {
      isink = s_sinkCollection.at(cookie);
      long result = fn(isink);
#if defined(_DEBUG) && defined(TRACE_NAVLIB)
      std::cout << "SetValue(0x" << std::hex << cookie << std::dec << ", " << property << ", "
                << *value << ") result =0x" << std::hex << result << std::endl;
#endif
      return result;
    }
#if defined(_DEBUG) && defined(TRACE_NAVLIB)
    catch (const std::out_of_range &e) {
      std::unique_lock<std::mutex> lock(s_mutex);
      std::cout << "std::out_of_range exception thrown in SetValue(0x" << std::hex << cookie
                << std::dec << ", " << property << ", value)\n"
                << *value << e.what() << std::endl;
      return navlib::make_result_code(navlib::navlib_errc::invalid_argument);
    } catch (const std::exception &e) {
      std::unique_lock<std::mutex> lock(s_mutex);
      std::cout << "Uncaught exception thrown in SetValue(0x" << std::hex << cookie << std::dec
                << ", " << property << "," << *value << ")\n"
                << e.what() << std::endl;
    }
#else
    catch (const std::out_of_range &) {
      return navlib::make_result_code(navlib::navlib_errc::invalid_argument);
    } catch (const std::exception &) {
    }
#endif
    return navlib::make_result_code(navlib::navlib_errc::error);
  }

  /// <summary>
  /// IEvents accessors and mutators
  /// </summary>
  static long SetActiveCommand(navlib::param_t cookie, navlib::property_t property,
                               const navlib::value_t *value) {
    return SetValue(cookie, property, value, [&](isink_t isink) {
      return isink->SetActiveCommand(static_cast<const char *>(*value));
    });
  }

  static long SetSettingsChanged(navlib::param_t cookie, navlib::property_t property,
                                 const navlib::value_t *value) {
    return SetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->SetSettingsChanged(*value); });
  }

  static long SetKeyPress(navlib::param_t cookie, navlib::property_t property,
                          const navlib::value_t *value) {
    return SetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->SetKeyPress(*value); });
  }
  static long SetKeyRelease(navlib::param_t cookie, navlib::property_t property,
                            const navlib::value_t *value) {
    return SetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->SetKeyRelease(*value); });
  }

  /// <summary>
  /// IHit accessors and mutators
  /// </summary>
  static long GetHitLookAt(navlib::param_t cookie, navlib::property_t property,
                           navlib::value_t *value) {
    return GetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->GetHitLookAt(*value); });
  }
  static long SetHitAperture(navlib::param_t cookie, navlib::property_t property,
                             const navlib::value_t *value) {
    return SetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->SetHitAperture(*value); });
  }
  static long SetHitDirection(navlib::param_t cookie, navlib::property_t property,
                              const navlib::value_t *value) {
    return SetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->SetHitDirection(*value); });
  }
  static long SetHitLookFrom(navlib::param_t cookie, navlib::property_t property,
                             const navlib::value_t *value) {
    return SetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->SetHitLookFrom(*value); });
  }
  static long SetHitSelectionOnly(navlib::param_t cookie, navlib::property_t property,
                                  const navlib::value_t *value) {
    return SetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->SetHitSelectionOnly(*value); });
  }

  /// <summary>
  /// IModel accessors and mutators
  /// </summary>
  static long GetModelExtents(navlib::param_t cookie, navlib::property_t property,
                              navlib::value_t *value) {
    return GetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->GetModelExtents(*value); });
  }
  static long GetSelectionExtents(navlib::param_t cookie, navlib::property_t property,
                                  navlib::value_t *value) {
    return GetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->GetSelectionExtents(*value); });
  }
  static long GetSelectionTransform(navlib::param_t cookie, navlib::property_t property,
                                    navlib::value_t *value) {
    return GetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->GetSelectionTransform(*value); });
  }
  static long GetIsSelectionEmpty(navlib::param_t cookie, navlib::property_t property,
                                  navlib::value_t *value) {
    return GetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->GetIsSelectionEmpty(*value); });
  }
  static long SetSelectionTransform(navlib::param_t cookie, navlib::property_t property,
                                    const navlib::value_t *value) {
    return SetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->SetSelectionTransform(*value); });
  }

  /// <summary>
  /// IPivot accessors and mutators
  /// </summary>
  static long GetPivotPosition(navlib::param_t cookie, navlib::property_t property,
                               navlib::value_t *value) {
    return GetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->GetPivotPosition(*value); });
  }
  static long IsUserPivot(navlib::param_t cookie, navlib::property_t property,
                          navlib::value_t *value) {
    return GetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->IsUserPivot(*value); });
  }
  static long SetPivotPosition(navlib::param_t cookie, navlib::property_t property,
                               const navlib::value_t *value) {
    return SetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->SetPivotPosition(*value); });
  }
  static long GetPivotVisible(navlib::param_t cookie, navlib::property_t property,
                              navlib::value_t *value) {
    return GetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->GetPivotVisible(*value); });
  }
  static long SetPivotVisible(navlib::param_t cookie, navlib::property_t property,
                              const navlib::value_t *value) {
    return SetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->SetPivotVisible(*value); });
  }

  /// <summary>
  /// ISpace3D accessors and mutators
  /// </summary>
  static long GetCoordinateSystem(navlib::param_t cookie, navlib::property_t property,
                                  navlib::value_t *value) {
    return GetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->GetCoordinateSystem(*value); });
  }
  static long GetFrontView(navlib::param_t cookie, navlib::property_t property,
                           navlib::value_t *value) {
    return GetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->GetFrontView(*value); });
  }

  /// <summary>
  /// IState accessors and mutators
  /// </summary>
  static long SetTransaction(navlib::param_t cookie, navlib::property_t property,
                             const navlib::value_t *value) {
    return SetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->SetTransaction(*value); });
  }
  static long SetMotionFlag(navlib::param_t cookie, navlib::property_t property,
                            const navlib::value_t *value) {
    return SetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->SetMotionFlag(*value); });
  }

  /// <summary>
  /// IView accessors and mutators
  /// </summary>
  static long GetCameraMatrix(navlib::param_t cookie, navlib::property_t property,
                              navlib::value_t *value) {
    return GetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->GetCameraMatrix(*value); });
  }
  static long GetCameraTarget(navlib::param_t cookie, navlib::property_t property,
                              navlib::value_t *value) {
    return GetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->GetCameraTarget(*value); });
  }
  static long GetPointerPosition(navlib::param_t cookie, navlib::property_t property,
                                 navlib::value_t *value) {
    return GetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->GetPointerPosition(*value); });
  }
  static long GetViewConstructionPlane(navlib::param_t cookie, navlib::property_t property,
                                       navlib::value_t *value) {
    return GetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->GetViewConstructionPlane(*value); });
  }
  static long GetViewExtents(navlib::param_t cookie, navlib::property_t property,
                             navlib::value_t *value) {
    return GetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->GetViewExtents(*value); });
  }
  static long GetViewFOV(navlib::param_t cookie, navlib::property_t property,
                         navlib::value_t *value) {
    return GetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->GetViewFOV(*value); });
  }
  static long GetViewFrustum(navlib::param_t cookie, navlib::property_t property,
                             navlib::value_t *value) {
    return GetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->GetViewFrustum(*value); });
  }
  static long GetIsViewPerspective(navlib::param_t cookie, navlib::property_t property,
                                   navlib::value_t *value) {
    return GetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->GetIsViewPerspective(*value); });
  }
  static long GetIsViewRotatable(navlib::param_t cookie, navlib::property_t property,
                                 navlib::value_t *value) {
    return GetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->GetIsViewRotatable(*value); });
  }
  static long SetCameraMatrix(navlib::param_t cookie, navlib::property_t property,
                              const navlib::value_t *value) {
    return SetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->SetCameraMatrix(*value); });
  }
  static long SetCameraTarget(navlib::param_t cookie, navlib::property_t property,
                              const navlib::value_t *value) {
    return SetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->SetCameraTarget(*value); });
  }
  static long SetPointerPosition(navlib::param_t cookie, navlib::property_t property,
                                 const navlib::value_t *value) {
    return SetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->SetPointerPosition(*value); });
  }
  static long SetViewExtents(navlib::param_t cookie, navlib::property_t property,
                             const navlib::value_t *value) {
    return SetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->SetViewExtents(*value); });
  }
  static long SetViewFOV(navlib::param_t cookie, navlib::property_t property,
                         const navlib::value_t *value) {
    return SetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->SetViewFOV(*value); });
  }
  static long SetViewFrustum(navlib::param_t cookie, navlib::property_t property,
                             const navlib::value_t *value) {
    return SetValue(cookie, property, value,
                    [&](isink_t isink) { return isink->SetViewFrustum(*value); });
  }

private:
  /// <summary>
  /// Read a <see cref="navlib::property_t"/> value from the navlib.
  /// </summary>
  /// <param name="nh">The <see cref="navlib::nlHandle_t"/> to the navigation library returned by
  /// a previous call to <see cref="navlib::NlCreate"/>.</param>
  /// <param name="name">The name of the navlib property to read.</param>
  /// <param name="value">Pointer to a <see cref="navlib::value_t"/> to receive the value.</param>
  /// <returns>0 =no error, otherwise a value from <see cref="navlib::make_result_code"/>.</returns>
  /// <exception cref="std::system_error">No connection to the navlib / 3D Mouse.</exception>
  static long ReadValue(navlib::nlHandle_t nh, navlib::property_t name, navlib::value_t *value) {
    try {
      if (nh == INVALID_NAVLIB_HANDLE) {
        throw std::system_error(navlib::make_error_code(navlib::navlib_errc::invalid_operation),
                                "No connection to the navlib");
      }
      long resultCode = navlib::NlReadValue(nh, name, value);
#if defined(_DEBUG) && defined(TRACE_NAVLIB)
      std::unique_lock<std::mutex> lock(s_mutex);
      std::cout << "NlReadValue(0x" << std::hex << nh << std::dec << ", " << name << ", " << *value
                << ") result =0x" << std::hex << resultCode << std::endl;
#endif
      return resultCode;
    }
#if defined(_DEBUG) && defined(TRACE_NAVLIB)
    catch (const std::system_error &e) {
      std::unique_lock<std::mutex> lock(s_mutex);
      std::cout << "system_error exception thrown in ReadValue(0x" << std::hex << nh << std::dec
                << ", " << name << ", value)\n"
                << *value << e.what() << std::endl;
      throw;
    } catch (const std::exception &e) {
      std::unique_lock<std::mutex> lock(s_mutex);
      std::cout << "Uncaught exception thrown in NlReadValue(0x" << std::hex << nh << std::dec
                << ", " << name << ", value)\n"
                << *value << e.what() << std::endl;
    }
#else
    catch (const std::system_error &) {
      throw;
    } catch (const std::exception &) {
    }
#endif
    return navlib::make_result_code(navlib::navlib_errc::error);
  }

  /// <summary>
  /// Write a <see cref="navlib::property_t"/> value to the navlib.
  /// </summary>
  /// <param name="nh">The <see cref="navlib::nlHandle_t"/> to the navigation library returned by
  /// a previous call to <see cref="navlib::NlCreate"/>.</param>
  /// <param name="name">The name of the navlib property to read.</param>
  /// <param name="value">Pointer to a <see cref="navlib::value_t"/> to receive the value.</param>
  /// <returns>0 =no error, otherwise a value from <see cref="navlib::make_result_code"/>.</returns>
  /// <exception cref="std::system_error">No connection to the navlib / 3D Mouse.</exception>
  static long WriteValue(navlib::nlHandle_t nh, navlib::property_t name,
                         const navlib::value_t *value) {
    try {
      if (nh == INVALID_NAVLIB_HANDLE) {
        throw std::system_error(navlib::make_error_code(navlib::navlib_errc::invalid_operation),
                                "No connection to the navlib");
      }
      long resultCode = navlib::NlWriteValue(nh, name, value);
#if defined(_DEBUG) && defined(TRACE_NAVLIB)
      std::unique_lock<std::mutex> lock(s_mutex);
      std::cout << "NlWriteValue(0x" << std::hex << nh << std::dec << ", " << name << ", " << *value
                << ") result =0x" << std::hex << resultCode << std::endl;
#endif
      return resultCode;
    }
#if defined(_DEBUG) && defined(TRACE_NAVLIB)
    catch (const std::system_error &e) {
      std::unique_lock<std::mutex> lock(s_mutex);
      std::cout << "system_error exception thrown in WriteValue(0x" << std::hex << nh << std::dec
                << ", " << name << ", value)\n"
                << *value << e.what() << std::endl;
      throw;
    } catch (const std::exception &e) {
      std::unique_lock<std::mutex> lock(s_mutex);
      std::cout << "Uncaught exception thrown in NlWriteValue(0x" << std::hex << nh << std::dec
                << ", " << name << ", value)\n"
                << *value << e.what() << std::endl;
    }
#else
    catch (const std::system_error &) {
      throw;
    } catch (const std::exception &) {
    }
#endif
    return navlib::make_result_code(navlib::navlib_errc::error);
  }

private:
  navlib::nlHandle_t m_hdl;
  std::mutex m_mutex;
  navlib::param_t m_cookie;
  std::string m_name;
  navlib::nlCreateOptions_t m_createOptions;
};
} // namespace Navigation3D
} // namespace SpaceMouse
} // namespace TDx
#endif // CNavigationModelImpl_HPP_INCLUDED