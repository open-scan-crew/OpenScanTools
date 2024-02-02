#ifndef MOUSE_3D_CONNEXION_H_
#define MOUSE_3D_CONNEXION_H_

#include <stdexcept>
typedef __int64 LONG_PTR, * PLONG_PTR;
typedef unsigned __int64 ULONG_PTR, * PULONG_PTR;
#define IS_INTRESOURCE(_r) ((((ULONG_PTR)(_r)) >> 16) == 0)
#include <3DxWare/inc/SpaceMouse/CNavigation3D.hpp>
#include <glm/glm.hpp>

class ViewportOrganizer;

class Mouse_3DConnexion : public TDx::SpaceMouse::Navigation3D::CNavigation3D
{
	typedef TDx::SpaceMouse::Navigation3D::CNavigation3D mouseHandler;

public:
	Mouse_3DConnexion();
	~Mouse_3DConnexion() {};

	bool setViewportOrganizer(ViewportOrganizer* camera);

private:
	//initialization
	void Disable3DNavigation();
	bool ExportCommands();
	bool ExportImages();
	bool Enable3DNavigation();

	//accessor
	long GetCoordinateSystem(navlib::matrix_t& affine) const override;
	long GetFrontView(navlib::matrix_t& affine) const override;
	long GetViewConstructionPlane(navlib::plane_t& plane) const override;
	long GetCameraMatrix(navlib::matrix_t& affine) const override;
	long GetViewExtents(navlib::box_t& affine) const override;
	long GetViewFrustum(navlib::frustum_t& frustum) const override;
	long GetViewFOV(double& fov) const override;
	long GetIsViewRotatable(navlib::bool_t& rotatble) const override;
	long IsUserPivot(navlib::bool_t& userPivot) const override;
	long GetPivotVisible(navlib::bool_t& visible) const override;
	long GetPivotPosition(navlib::point_t& position) const override;
	long GetPointerPosition(navlib::point_t& position) const override;
	long GetIsViewPerspective(navlib::bool_t& persp) const override;
	long GetModelExtents(navlib::box_t& extents) const override;
	long GetIsSelectionEmpty(navlib::bool_t& empty) const override;
	long GetSelectionExtents(navlib::box_t& extents) const override;
	long GetSelectionTransform(navlib::matrix_t&) const override;
	long GetHitLookAt(navlib::point_t& position) const override;

	//setter
	//long SetSettingsChanged(long change) override;
	long SetMotionFlag(bool value) override;
	long SetTransaction(long value) override;
	long SetCameraMatrix(const navlib::matrix_t& affine) override;
	long SetSelectionTransform(const navlib::matrix_t& affine) override;
	long SetViewExtents(const navlib::box_t& extents) override;
	long SetViewFOV(double fov) override;
	long SetViewFrustum(const navlib::frustum_t& frustum) override;
	long SetPivotPosition(const navlib::point_t& position) override;
	long SetPivotVisible(bool visible) override;

	long SetHitLookFrom(const navlib::point_t& position) override;
	long SetHitDirection(const navlib::vector_t& direction) override;
	long SetHitAperture(double diameter) override;
	long SetHitSelectionOnly(bool value) override;
	long SetActiveCommand(std::string commandId) override;

private:
	ViewportOrganizer* m_viewportOrganiser;
};


#endif // !MOUSE_3D_CONNEXION_H_ 
