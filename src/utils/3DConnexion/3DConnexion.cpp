#include "utils/3DConnexion/3DConnexion.h"
#include "utils/Logger.h"

#include "gui/viewport/ViewportOrganizer.h"

Mouse_3DConnexion::Mouse_3DConnexion()
	: TDx::SpaceMouse::Navigation3D::CNavigation3D(false, true)
    , m_viewportOrganiser(nullptr)
{}

bool Mouse_3DConnexion::setViewportOrganizer(ViewportOrganizer* viewportOrganizer)
{
    if (m_viewportOrganiser)
        return true;
    m_viewportOrganiser = viewportOrganizer;
    return Enable3DNavigation();
}

void Mouse_3DConnexion::Disable3DNavigation()
{
	mouseHandler::Enable = false;
}

bool Mouse_3DConnexion::Enable3DNavigation() 
{
	try {
	    // Set the hint/title for the '3Dconnexion Properties' Utility.
		mouseHandler::Profile = "OpenScanTools";
	
	    // Enable input from / output to the Navigation3D controller.
		//mouseHandler::Enable = true;
        mouseHandler::EnableNavigation(true);
	
#if APPLICATION_HAS_ANIMATION_LOOP
	    // Use the application render loop as the timing source for the frames
	    nav3d::FrameTiming = TimingSource::Application;
#else
	    // Use the SpaceMouse as the timing source for the frames.
		mouseHandler::FrameTiming = TimingSource::SpaceMouse;
#endif
	
	    // Export the command images
	    ExportImages();
	
	    // Export the commands
	    ExportCommands();
	
	}
	catch (const std::exception& e) 
	{
		Logger::log(LoggerMode::LogConfig, std::string("3DConnexion error: ") + e.what());
	    return false;
	}
	
	return true;
}

/// <summary>
/// Expose the images of the application commands to the 3Dconnexion UI
/// </summary>
/// Note images embedded in a resource dll (e_resource_file type) use the
/// "#DecimalNumber" Microsoft string notation i.e. RT_BITMAP = "#2", RT_ICON =
/// "#3", resource id with a value of MAKEINTRESOURCE(216) = "#216". The
/// SiImage_t::id is used as the key to associate the image with the
/// corresponding command
bool Mouse_3DConnexion::ExportImages()
{
  //  std::vector<SiImage_t> images;
  //  // Use some images from a resource file
  //  images.push_back({ sizeof(SiImage_t),
  //                      e_resource_file,
  //                      "MenuItem 57601" /*ID_FILE_OPEN */,
  //                      {"c:/windows/system32/ieframe.dll", "#216", "#2", 12} });
  //  images.push_back({ sizeof(SiImage_t),
  //                      e_resource_file,
  //                      "MenuItem 57665" /*ID_APP_EXIT */,
  //                      {"c:/windows/system32/ieframe.dll", "#216", "#2", 10} });
  //  images.push_back({ sizeof(SiImage_t),
  //                      e_resource_file,
  //                      "MenuItem 57664" /*ID_APP_ABOUT*/,
  //                      {"c:/windows/system32/ieframe.dll", "#216", "#2", 2} });
  //  if (images.size()) 
		//mouseHandler::AddImages(images);
    return true;
}

/// <summary>
/// Expose the application commands to the 3Dconnexion UI
/// </summary>
bool  Mouse_3DConnexion::ExportCommands() 
{
    using TDx::SpaceMouse::CCommandSet;

    // The root action set node
    CCommandSet commandSet("Default", "Modeling");

    // Activate the command set
	mouseHandler::ActiveCommands = commandSet.Id;

    // Add the menu(s) to the action set
    //CacheMenu(commandSet, GetMenu());

    // Add the command set to the commands available for assigning to 3DMouse buttons
	mouseHandler::AddCommandSet(commandSet);

    return true;
}

long Mouse_3DConnexion::GetModelExtents(navlib::box_t& bbox) const
{
    double min(-1), max(1);
    bbox.min = { {min, min, min} };
    bbox.max = { {max, max, max} };
    return 0;
}

long Mouse_3DConnexion::GetIsSelectionEmpty(navlib::bool_t& empty) const 
{
    empty = true;
    return 0;
}

long Mouse_3DConnexion::GetIsViewRotatable(navlib::bool_t& rotatable) const 
{
    rotatable = true;
    return 0;
}

/// <inheritdoc/>
long Mouse_3DConnexion::IsUserPivot(navlib::bool_t& userPivot) const 
{
    userPivot = false;
    return 0;
}

long Mouse_3DConnexion::GetCoordinateSystem(navlib::matrix_t& affine) const 
{
#if _Z_UP
    // For Z-up
    matrix_t cs = { 1., 0., 0., 0., 0., 0., -1., 0., 0., 1., 0., 0., 0., 0., 0., 1. };
#else
    // Y-Up rhs
    navlib::matrix_t cs = { 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1. };
#endif
    affine = cs;
    return 0;
}

long Mouse_3DConnexion::GetFrontView(navlib::matrix_t& affine) const
{
    glm::dmat4 front(1.0);
    for (uint16_t iterator(0); iterator < 16; iterator++)
        affine.m[iterator] = front[iterator/4][iterator % 4];
    return 0;
}

long Mouse_3DConnexion::GetCameraMatrix(navlib::matrix_t& affine) const 
{
    if (m_viewportOrganiser)
    {
        glm::dmat4 view(m_viewportOrganiser->getCurrentViewMatrix());
        for (uint16_t iterator(0); iterator < 16; iterator++)
            affine.m[iterator] = view[iterator / 4][iterator % 4];
    }
    else
    {
        glm::dmat4 front(1.0);
        for (uint16_t iterator(0); iterator < 16; iterator++)
            affine.m[iterator] = front[iterator / 4][iterator % 4];
    }
    return 0;
}

/// <inheritdoc/>
long Mouse_3DConnexion::GetPivotVisible(navlib::bool_t& visible) const 
{
    visible = false;
    return 0;
}


long Mouse_3DConnexion::GetIsViewPerspective(navlib::bool_t& persp) const 
{
    persp = true;
    return 0;
}

/// <summary>
/// Sets the transaction property value
/// </summary>
/// <param name="value">!0 at the beginning of a frame;0 at the end of a
/// frame</param> <returns>0 on success, otherwise an error.</returns>
long Mouse_3DConnexion::SetMotionFlag(bool value) 
{
    return 0;
}

/// <summary>
/// Sets the transaction property value
/// </summary>
/// <param name="value">!0 at the beginning of a frame;0 at the end of a
/// frame</param> <returns>0 on success, otherwise an error.</returns>
long Mouse_3DConnexion::SetTransaction(long value) 
{
    return 0;
}

/// <summary>
/// Sets the affine of the view
/// </summary>
/// <param name="affine">The camera to world transformation</param>
/// <returns>0 on success, otherwise an error.</returns>
long Mouse_3DConnexion::SetCameraMatrix(const navlib::matrix_t& affine) 
{
    if (m_viewportOrganiser)
    {
        glm::dmat4 newView;
        for (uint16_t iterator(0); iterator < 16; iterator++)
            newView[iterator / 4][iterator % 4] = affine[iterator];
        newView = glm::inverse(newView);
        
        m_viewportOrganiser->mouse3DUpdate(newView);
    }
    return 0;
}

long Mouse_3DConnexion::SetActiveCommand(std::string commandId)
{
    /*if (!commandId.empty()) {
        if (!IsWindowEnabled()) {
            return navlib::make_result_code(navlib::navlib_errc::invalid_operation);
        }

        CApplicationCommand action;
        std::istringstream stream(std::move(commandId));
        stream >> action;
#ifdef _DEBUG
        action(this);
#else
        try {
            action(this);
        }
        catch (...) {
            return navlib::make_result_code(navlib::navlib_errc::invalid_function);
        }
#endif
    }*/
    return 0;
}

/// Useless methods (for now)

long Mouse_3DConnexion::GetPointerPosition(navlib::point_t& position) const
{
    return navlib::make_result_code(navlib::navlib_errc::function_not_supported);
}

long Mouse_3DConnexion::GetSelectionExtents(navlib::box_t& bbox) const
{
    return navlib::make_result_code(navlib::navlib_errc::function_not_supported);
}

long Mouse_3DConnexion::GetViewConstructionPlane(navlib::plane_t& plane) const
{
    return navlib::make_result_code(navlib::navlib_errc::function_not_supported);
}

long Mouse_3DConnexion::GetViewExtents(navlib::box_t& extents) const
{
    return navlib::make_result_code(navlib::navlib_errc::function_not_supported);
}

long Mouse_3DConnexion::GetViewFOV(double& fov) const
{
    return navlib::make_result_code(navlib::navlib_errc::function_not_supported);
}

long Mouse_3DConnexion::GetViewFrustum(navlib::frustum_t& frustum) const
{
    return navlib::make_result_code(navlib::navlib_errc::function_not_supported);
}

long Mouse_3DConnexion::GetHitLookAt(navlib::point_t& position) const
{
    return navlib::make_result_code(navlib::navlib_errc::function_not_supported);
}

long Mouse_3DConnexion::GetPivotPosition(navlib::point_t& position) const
{

    return navlib::make_result_code(navlib::navlib_errc::function_not_supported);
}

long Mouse_3DConnexion::GetSelectionTransform(navlib::matrix_t& affine) const 
{
    return navlib::make_result_code(navlib::navlib_errc::function_not_supported);
}

long Mouse_3DConnexion::SetSelectionTransform(const navlib::matrix_t& affine)
{
    return navlib::make_result_code(navlib::navlib_errc::function_not_supported);
}

long Mouse_3DConnexion::SetViewExtents(const navlib::box_t& extents) 
{
    return navlib::make_result_code(navlib::navlib_errc::function_not_supported);
}

long Mouse_3DConnexion::SetPivotVisible(bool show) 
{
    return navlib::make_result_code(navlib::navlib_errc::function_not_supported);
}

long Mouse_3DConnexion::SetPivotPosition(const navlib::point_t& position) 
{
    return navlib::make_result_code(navlib::navlib_errc::function_not_supported);
}

long Mouse_3DConnexion::SetViewFOV(double fov) 
{
    return navlib::make_result_code(navlib::navlib_errc::function_not_supported);
}

long Mouse_3DConnexion::SetViewFrustum(const navlib::frustum_t& frustum) 
{
    return navlib::make_result_code(navlib::navlib_errc::function_not_supported);
}

long Mouse_3DConnexion::SetHitAperture(double diameter)
{
    return navlib::make_result_code(navlib::navlib_errc::function_not_supported);
}

long Mouse_3DConnexion::SetHitDirection(const navlib::vector_t& direction) 
{
    return navlib::make_result_code(navlib::navlib_errc::function_not_supported);
}

long Mouse_3DConnexion::SetHitLookFrom(const navlib::point_t& position)
{
    return navlib::make_result_code(navlib::navlib_errc::function_not_supported);
}

long Mouse_3DConnexion::SetHitSelectionOnly(bool value)
{
    return navlib::make_result_code(navlib::navlib_errc::function_not_supported);
}