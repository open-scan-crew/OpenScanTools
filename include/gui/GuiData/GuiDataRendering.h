#ifndef GUI_DATA_RENDERING_H
#define GUI_DATA_RENDERING_H

#include "gui/GuiData/IGuiData.h"
#include "gui/UnitUsage.h"
#include "pointCloudEngine/RenderingTypes.h"
#include "pointCloudEngine/GuiRenderingTypes.h"
#include "pointCloudEngine/ShowTypes.h"
#include "io/ImageTypes.h"
#include "models/3d/BoundingBox.h"
#include "utils/Color32.hpp"
#include "utils/safe_ptr.h"
#include "models/3d/NavigationTypes.h"

#include <filesystem>

class IPanel;

class CameraNode;
class ViewPointNode;
class AGraphNode;

class GuiDataActiveCamera : public IGuiData
{
public:
	GuiDataActiveCamera(SafePtr<CameraNode> camera);
	~GuiDataActiveCamera() {};
	virtual guiDType getType() = 0;

public:
	SafePtr<CameraNode> m_camera;
};

class GuiDataCameraInfo : public GuiDataActiveCamera
{
public:
	GuiDataCameraInfo(SafePtr<CameraNode> camera);
	~GuiDataCameraInfo() {};
	virtual guiDType getType();
};

class GuiDataDisplayGuizmo : public GuiDataActiveCamera
{
public:
	GuiDataDisplayGuizmo(SafePtr<CameraNode> camera, const bool& display);
	~GuiDataDisplayGuizmo() {};
	virtual guiDType getType();

public:
	const bool m_isDisplayed;
};

class GuiDataGizmoParameters : public IGuiData
{
public:
	GuiDataGizmoParameters(const glm::vec3& parameters);
	~GuiDataGizmoParameters() {};
	virtual guiDType getType();

public:
	const glm::vec3 m_paramters;
};

class GuiDataRenderBackgroundColor : public GuiDataActiveCamera
{
public:
	GuiDataRenderBackgroundColor(SafePtr<CameraNode> camera, const Color32& color = Color32(0,0,0));
	GuiDataRenderBackgroundColor(SafePtr<CameraNode> camera, const uint8_t& r, const uint8_t& g, const uint8_t& b);
	~GuiDataRenderBackgroundColor();
	virtual guiDType getType() override;

public:
	const Color32 m_color;
};

class GuiDataRenderAdjustZoom : public GuiDataActiveCamera
{
public:
	GuiDataRenderAdjustZoom(const BoundingBoxD& sceneBBox, SafePtr<CameraNode> camera);
	virtual guiDType getType() override;

public:
	const BoundingBoxD scene_bbox_;
};

class GuiDataRenderCameraMoveTo : public GuiDataActiveCamera
{
public:
    GuiDataRenderCameraMoveTo(glm::dvec3 position, SafePtr<CameraNode> camera);
    ~GuiDataRenderCameraMoveTo() {};
    virtual guiDType getType() override;

public:
    const glm::dvec3 m_newPosition;
};

class GuiDataRenderRotateCamera : public GuiDataActiveCamera
{
public:
	GuiDataRenderRotateCamera(const double& theta, const double& phi, const bool& isAdditive, SafePtr<CameraNode> camera);
	~GuiDataRenderRotateCamera() {};
	virtual guiDType getType() override;

public:
	const double m_theta;
	const double m_phi;
	const bool m_additive;
};

class GuiDataRenderColorMode : public GuiDataActiveCamera
{
public:
    GuiDataRenderColorMode(UiRenderMode mode, SafePtr<CameraNode> camera);
    ~GuiDataRenderColorMode() {};
    virtual guiDType getType() override;

public:
	UiRenderMode m_mode;
};

class GuiDataRenderExamine : public GuiDataActiveCamera
{
public:
    GuiDataRenderExamine(SafePtr<CameraNode> camera, const bool& activate);
	GuiDataRenderExamine(SafePtr<CameraNode> camera, const glm::dvec3& pos);
    ~GuiDataRenderExamine() {};
    virtual guiDType getType() override;

public:
    bool m_activate;
	const glm::dvec3 m_position;
};

class GuiDataRenderExamineTarget : public IGuiData
{
public:
	GuiDataRenderExamineTarget(bool show);
	~GuiDataRenderExamineTarget() {};
	virtual guiDType getType() override;

public:
	bool m_show;
};

class GuiDataRenderTargetClick : public IGuiData
{
public:
	GuiDataRenderTargetClick();
	GuiDataRenderTargetClick(const glm::dvec3& pos, const Color32& color = glm::vec3(0.875f, 0.0f, 0.875f));
	~GuiDataRenderTargetClick();
	virtual guiDType getType() override;

public:
	bool m_reset;
	const glm::dvec3 m_position;
	const Color32 m_color;
};

class GuiDataRenderPointSize : public GuiDataActiveCamera
{
public:
    GuiDataRenderPointSize(float ptSize, SafePtr<CameraNode> camera);
    ~GuiDataRenderPointSize() {};
    virtual guiDType getType() override;

public:
    float m_pointSize;
};

class GuiDataRenderTexelThreshold : public GuiDataActiveCamera
{
public:
    GuiDataRenderTexelThreshold(int texelThreshold, SafePtr<CameraNode> camera);
    ~GuiDataRenderTexelThreshold() {};
    virtual guiDType getType() override;

public:
    int m_texelThreshold;
};

class GuiDataRenderBrightness : public GuiDataActiveCamera
{
public:
    GuiDataRenderBrightness(int brightness, SafePtr<CameraNode> camera);
    ~GuiDataRenderBrightness() {};
    virtual guiDType getType() override;

public:
    int m_brightness;
};

class GuiDataRenderContrast : public GuiDataActiveCamera
{
public:
    GuiDataRenderContrast(int contrast, SafePtr<CameraNode> camera);
    ~GuiDataRenderContrast() {};
    virtual guiDType getType() override;

public:
    int m_contrast;
};

class GuiDataRenderLuminance : public GuiDataActiveCamera
{
public:
	GuiDataRenderLuminance(int luminance, SafePtr<CameraNode> camera);
	~GuiDataRenderLuminance() {};
	virtual guiDType getType() override;

public:
	int m_luminance;
};

class GuiDataRenderBlending : public GuiDataActiveCamera
{
public:
	GuiDataRenderBlending(int blending, SafePtr<CameraNode> camera);
	~GuiDataRenderBlending() {};
	virtual guiDType getType() override;

public:
	int m_hue;
};


class GuiDataRenderSaturation : public GuiDataActiveCamera
{
public:
	GuiDataRenderSaturation(int saturation, SafePtr<CameraNode> camera);
	~GuiDataRenderSaturation() {};
	virtual guiDType getType() override;

public:
	int m_saturation;
};

class GuiDataRenderTransparency : public GuiDataActiveCamera
{
public:
    GuiDataRenderTransparency(BlendMode mode, float transparencyValue, SafePtr<CameraNode> camera);
    ~GuiDataRenderTransparency() {};
    virtual guiDType getType() override;

public:
	BlendMode m_mode;
    float m_transparencyValue; // transparencyValue
};

class GuiDataRenderTransparencyOptions : public GuiDataActiveCamera
{
public:
	GuiDataRenderTransparencyOptions(bool negativeColors, bool reduceFlash, bool flashAdvanced, float flashControl, float hlt, SafePtr<CameraNode> camera);
	~GuiDataRenderTransparencyOptions() {};
	virtual guiDType getType() override;

public:
	bool m_negativeEffect;
	bool m_reduceFlash;
    bool m_flashAdvanced;
    float m_flashControl;
	float m_highLuminosityThreshold;
};

class GuiDataRenderPointCount : public IGuiData
{
public:
	GuiDataRenderPointCount(uint64_t pointCount);
	~GuiDataRenderPointCount() {};
	virtual guiDType getType() override;

public:
	uint64_t m_pointCount;
};

class GuiDataRenderViewPoint : public IGuiData
{
public:
	GuiDataRenderViewPoint(SafePtr<ViewPointNode> camera);
	~GuiDataRenderViewPoint() {}
	virtual guiDType getType() override;
public:
	SafePtr<ViewPointNode> m_keypoint;
};

class GuiDataRenderAnimationViewPoint : public IGuiData
{
public:
	GuiDataRenderAnimationViewPoint(SafePtr<ViewPointNode> keypoint);
	~GuiDataRenderAnimationViewPoint() {}
	virtual guiDType getType() override;
public:
	SafePtr<ViewPointNode> m_keypoint;
};

class GuiDataRenderFlatColor : public GuiDataActiveCamera
{
public:
	GuiDataRenderFlatColor(const glm::vec3& color, SafePtr<CameraNode> camera);
	GuiDataRenderFlatColor(const float& r, const float& g, const float& b, SafePtr<CameraNode> camera);
	~GuiDataRenderFlatColor() {}
	virtual guiDType getType() override;
public:
	const glm::vec3 m_color;
};

class GuiDataPostRenderingNormals : public GuiDataActiveCamera
{
public:
	GuiDataPostRenderingNormals(const PostRenderingNormals& lightingParams, bool onlySimpleNormalsInfo, SafePtr<CameraNode> camera);
	~GuiDataPostRenderingNormals() {};
	virtual guiDType getType() override;
public:
	PostRenderingNormals m_normals;
	bool m_onlySimpleNormalsInfo;
};

class GuiDataRenderAmbientOcclusion : public GuiDataActiveCamera
{
public:
	GuiDataRenderAmbientOcclusion(const PostRenderingAmbientOcclusion& aoParams, SafePtr<CameraNode> camera);
	~GuiDataRenderAmbientOcclusion() {};
	virtual guiDType getType() override;

public:
	PostRenderingAmbientOcclusion m_ao;
};

class GuiDataEdgeAwareBlur : public GuiDataActiveCamera
{
public:
        GuiDataEdgeAwareBlur(const EdgeAwareBlur& blurSettings, SafePtr<CameraNode> camera);
        ~GuiDataEdgeAwareBlur() {};
        virtual guiDType getType() override;

public:
        EdgeAwareBlur m_blur;
};

class GuiDataDepthLining : public GuiDataActiveCamera
{
public:
        GuiDataDepthLining(const DepthLining& liningSettings, SafePtr<CameraNode> camera);
        ~GuiDataDepthLining() {};
        virtual guiDType getType() override;

public:
        DepthLining m_lining;
};

class GuiDataRenderDistanceRampValues : public GuiDataActiveCamera
{
public:
	GuiDataRenderDistanceRampValues(float min, float max, int steps, SafePtr<CameraNode> camera);
	~GuiDataRenderDistanceRampValues() {};
	virtual guiDType getType() override;
public:
	float m_min;
	float m_max;
	int m_steps;
};

class GuiDataRenderDisplayObjectTexts : public GuiDataActiveCamera
{
public:
	GuiDataRenderDisplayObjectTexts(const bool& display, SafePtr<CameraNode> camera);
	~GuiDataRenderDisplayObjectTexts() {}
	virtual guiDType getType() override;

public:
	bool m_display;
};

class GuiDataRenderStartAnimation : public IGuiData
{
public:
	GuiDataRenderStartAnimation() {}
	~GuiDataRenderStartAnimation() {}
	virtual guiDType getType() override;
};

class GuiDataRenderStopAnimation : public IGuiData
{
public:
	GuiDataRenderStopAnimation() {}
	~GuiDataRenderStopAnimation() {}
	virtual guiDType getType() override;
};

class GuiDataRenderCleanAnimationList : public IGuiData
{
public:
	GuiDataRenderCleanAnimationList() {}
	~GuiDataRenderCleanAnimationList() {}
	virtual guiDType getType() override;
};

class GuiDataRenderAnimationLoop : public IGuiData
{
public:
	GuiDataRenderAnimationLoop(const bool& loop);
	~GuiDataRenderAnimationLoop() {}
	virtual guiDType getType() override;
	const bool _loop;
};

class GuiDataRenderAnimationSpeed : public IGuiData
{
public:
	GuiDataRenderAnimationSpeed(const uint16_t& speed);
	~GuiDataRenderAnimationSpeed() {}
	virtual guiDType getType() override;

	const uint16_t m_speed;
};

class GuiDataRenderRecordPerformances : public IGuiData
{
public:
	GuiDataRenderRecordPerformances(const std::filesystem::path& file);
	~GuiDataRenderRecordPerformances();
	virtual guiDType getType() override;

public:
	const std::filesystem::path m_path;
};

class GuiDataRenderImagesFormat : public IGuiData
{
public:
	GuiDataRenderImagesFormat(const ImageFormat& format, bool includeAlpha);
	~GuiDataRenderImagesFormat() {};
	virtual guiDType getType() override;

public:
	const ImageFormat m_format;
	const bool m_includeAlpha;
};

class GuiDataRenderUnitUsage : public GuiDataActiveCamera
{
public:
	GuiDataRenderUnitUsage(const UnitUsage& valueParameters, SafePtr<CameraNode> camera);
	~GuiDataRenderUnitUsage() {}
	guiDType getType() override;
public:
	const UnitUsage m_valueParameters;
};

class GuiDataRenderDecimationOptions : public IGuiData
{
public:
    GuiDataRenderDecimationOptions(DecimationOptions options);
    ~GuiDataRenderDecimationOptions() {}
    guiDType getType() override;
public:
    const DecimationOptions m_options;
};

class GuiDataRenderOctreePrecision : public IGuiData
{
public:
	GuiDataRenderOctreePrecision(OctreePrecision precision);
	~GuiDataRenderOctreePrecision() {}
	guiDType getType() override;
public:
	const OctreePrecision m_precision;
};

class GuiDataRenderNavigationConstraint : public GuiDataActiveCamera
{
public:
	GuiDataRenderNavigationConstraint(NaviConstraint constraint, SafePtr<CameraNode> camera);
	~GuiDataRenderNavigationConstraint() {}
	virtual guiDType getType() override;

public:
	NaviConstraint m_constraint;
};

class GuiDataRenderApplyNavigationConstraint : public GuiDataActiveCamera
{
public:
	GuiDataRenderApplyNavigationConstraint(bool checked, SafePtr<CameraNode> camera);
	~GuiDataRenderApplyNavigationConstraint() {}
	virtual guiDType getType() override;

public:
	bool m_checked;
};

class GuiDataRenderFov : public GuiDataActiveCamera
{
public:
	GuiDataRenderFov(float fov, SafePtr<CameraNode> camera);
	~GuiDataRenderFov() {}
	virtual guiDType getType() override;

public:
	float m_fov;
};

class GuiDataRenderIsPanoramic : public IGuiData
{
public:
	GuiDataRenderIsPanoramic(bool pano);
	~GuiDataRenderIsPanoramic() {}
	virtual guiDType getType() override;

public:
	bool m_pano;
};

class GuiDataRenderTextFilter : public GuiDataActiveCamera
{
public:
	GuiDataRenderTextFilter(TextFilter textFilter, SafePtr<CameraNode> camera);
	~GuiDataRenderTextFilter() {}
	guiDType getType() override;

	TextFilter m_textFilter;
};


class GuiDataRenderTextTheme : public GuiDataActiveCamera
{
public:
	GuiDataRenderTextTheme(int textTheme, SafePtr<CameraNode> camera);
	guiDType getType() override;

public:
	//0 -> dark theme (normal) | 1 -> light theme (black text, light grey background)
	int m_textTheme;
};

class GuiDataRenderTextFontSize : public GuiDataActiveCamera
{
public:
	GuiDataRenderTextFontSize(float fontSize, SafePtr<CameraNode> camera);
	guiDType getType() override;

	float m_textFontSize;
};

class GuiDataMarkerDisplayOptions : public GuiDataActiveCamera
{
public:
    GuiDataMarkerDisplayOptions(const MarkerDisplayOptions& params, SafePtr<CameraNode> camera);
    guiDType getType() override;

    MarkerDisplayOptions m_parameters;
};

class GuiDataAlphaObjectsRendering : public GuiDataActiveCamera
{
public:
	GuiDataAlphaObjectsRendering(const float& alpha, SafePtr<CameraNode> camera);
	guiDType getType() override;

	const float m_alpha;
};

class GuiDataRenderMeasureOptions : public GuiDataActiveCamera
{
public:
	GuiDataRenderMeasureOptions(MeasureShowMask showMask, SafePtr<CameraNode> camera);
	~GuiDataRenderMeasureOptions() {};
	virtual guiDType getType() override;

public:
	MeasureShowMask m_showMask;
};

class GuiDataRenderNavigationParameters : public IGuiData
{
public:
	GuiDataRenderNavigationParameters(const NavigationParameters& navParams);
	~GuiDataRenderNavigationParameters() {};
	guiDType getType() override;

public:
	NavigationParameters m_navParam;
};

class GuiDataRenderPerspectiveZBounds : public GuiDataActiveCamera
{
public:
	GuiDataRenderPerspectiveZBounds(const PerspectiveZBounds& rOpt, SafePtr<CameraNode> camera);
	~GuiDataRenderPerspectiveZBounds() {};
	guiDType getType() override;

public:
	PerspectiveZBounds m_zBounds;
};

class GuiDataRenderOrthographicZBounds : public GuiDataActiveCamera
{
public:
	GuiDataRenderOrthographicZBounds(const OrthographicZBounds& rOpt, SafePtr<CameraNode> camera);
	~GuiDataRenderOrthographicZBounds() {};
	guiDType getType() override;

public:
	OrthographicZBounds m_zBounds;
};

class GuiDataRampScale : public GuiDataActiveCamera
{
public:
	GuiDataRampScale(const RampScale& rampScale, const SafePtr<CameraNode>& camera);
	~GuiDataRampScale() {};
	virtual guiDType getType() override;
public:
	RampScale m_rampScale;
};

#endif
