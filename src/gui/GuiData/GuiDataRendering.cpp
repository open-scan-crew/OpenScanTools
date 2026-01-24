#include "gui/GuiData/GuiDataRendering.h"

//*** Active Camera ***//
GuiDataActiveCamera::GuiDataActiveCamera(SafePtr<CameraNode> camera)
	: m_camera(camera)
{}

//*** Active Camera & Info ***//
GuiDataCameraInfo::GuiDataCameraInfo(SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
{}

guiDType GuiDataCameraInfo::getType()
{
	return guiDType::renderActiveCamera;
}

//*** Guizmo displaying ***//
GuiDataDisplayGuizmo::GuiDataDisplayGuizmo(SafePtr<CameraNode> camera, const bool& display)
	: GuiDataActiveCamera(camera)
	, m_isDisplayed(display)
{}

guiDType GuiDataDisplayGuizmo::getType()
{
	return guiDType::renderGuizmo;
}

//*** Guizmo displaying ***//

GuiDataGizmoParameters::GuiDataGizmoParameters(const glm::vec3& parameters)
	:m_paramters(parameters)
{}
	
guiDType GuiDataGizmoParameters::getType()
{
	return guiDType::renderGizmoParameters;
}


//*** Background Color ***//
GuiDataRenderBackgroundColor::GuiDataRenderBackgroundColor(SafePtr<CameraNode> camera, const Color32& color)
	: GuiDataActiveCamera(camera)
	, m_color(color)
{}

GuiDataRenderBackgroundColor::GuiDataRenderBackgroundColor(SafePtr<CameraNode> camera, const uint8_t& r, const uint8_t& g, const uint8_t& b)
	: GuiDataActiveCamera(camera)
	, m_color(r,g,b)
{}

GuiDataRenderBackgroundColor::~GuiDataRenderBackgroundColor() 
{}

guiDType GuiDataRenderBackgroundColor::getType()
{
	return guiDType::renderBackgroundColor;
}

//*** AdjustZoom ***//
GuiDataRenderAdjustZoom::GuiDataRenderAdjustZoom(const BoundingBoxD& scene_bbox, SafePtr<CameraNode> camera)
    : GuiDataActiveCamera(camera)
    , scene_bbox_(scene_bbox)
{}

guiDType GuiDataRenderAdjustZoom::getType()
{
    return guiDType::renderAdjustZoom;
}

//*** Move Camera to Position ***//
GuiDataRenderCameraMoveTo::GuiDataRenderCameraMoveTo(glm::dvec3 position, SafePtr<CameraNode> destViewport)
	: GuiDataActiveCamera(destViewport)
	, m_newPosition(position)
{}

guiDType GuiDataRenderCameraMoveTo::getType()
{
    return guiDType::renderCameraMoveTo;
}

//*** Rotate Theta View ***//
GuiDataRenderRotateCamera::GuiDataRenderRotateCamera(const double& theta, const double& phi, const bool& isAdditive, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_theta(theta)
	, m_phi(phi)
	, m_additive(isAdditive)
{}

guiDType GuiDataRenderRotateCamera::getType()
{
	return guiDType::renderRotateThetaPhiView;
}

//*** Color Mode ***//
GuiDataRenderColorMode::GuiDataRenderColorMode(UiRenderMode mode, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_mode(mode)
{}

guiDType GuiDataRenderColorMode::getType()
{
    return guiDType::renderColorMode;
}

//*** Examine activation ***//
GuiDataRenderExamine::GuiDataRenderExamine(SafePtr<CameraNode> camera, const bool& _activate)
	: GuiDataActiveCamera(camera)
	, m_activate(_activate)
	, m_position(NAN, NAN, NAN)
{}

GuiDataRenderExamine::GuiDataRenderExamine(SafePtr<CameraNode> camera, const glm::dvec3& position)
	: GuiDataActiveCamera(camera)
	, m_activate(true)
	, m_position(position)
{}

guiDType GuiDataRenderExamine::getType()
{
    return guiDType::renderExamine;
}

//*** Examine Target activation ***//
GuiDataRenderExamineTarget::GuiDataRenderExamineTarget(bool showTarget)
	: m_show(showTarget)
{}

guiDType GuiDataRenderExamineTarget::getType()
{
	return guiDType::renderTargetExamine;
}

//*** Target Marked ***//

GuiDataRenderTargetClick::GuiDataRenderTargetClick()
	: m_reset(true)
	, m_position(NAN, NAN, NAN)
{}

GuiDataRenderTargetClick::GuiDataRenderTargetClick(const glm::dvec3& position, const Color32& color)
	: m_reset(false)
    , m_position(position)
	, m_color(color)
{}

GuiDataRenderTargetClick::~GuiDataRenderTargetClick() 
{}

guiDType GuiDataRenderTargetClick::getType()
{
	return guiDType::renderTargetClick;
}

//*** Point Size ***//
GuiDataRenderPointSize::GuiDataRenderPointSize(float _pointSize, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_pointSize(_pointSize)
{}

guiDType GuiDataRenderPointSize::getType()
{
    return guiDType::renderPointSize;
}

//*** Texel Threshold ***//
GuiDataRenderTexelThreshold::GuiDataRenderTexelThreshold(int texelThreshold, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_texelThreshold(texelThreshold)
{}

guiDType GuiDataRenderTexelThreshold::getType()
{
	return guiDType::renderTexelThreshold;
}

//*** Gap Filling Density Auto ***//
GuiDataRenderGapFillingDensityAuto::GuiDataRenderGapFillingDensityAuto(bool enabled, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_enabled(enabled)
{}

guiDType GuiDataRenderGapFillingDensityAuto::getType()
{
	return guiDType::renderGapFillingDensityAuto;
}

//*** Brightness ***//
GuiDataRenderBrightness::GuiDataRenderBrightness(int _brightness, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_brightness(_brightness)
{}

guiDType GuiDataRenderBrightness::getType()
{
    return guiDType::renderBrightness;
}

//*** Contrast ***//
GuiDataRenderContrast::GuiDataRenderContrast(int _contrast, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_contrast(_contrast)
{}

guiDType GuiDataRenderContrast::getType()
{
    return guiDType::renderContrast;
}

//*** Luminance ***//

GuiDataRenderLuminance::GuiDataRenderLuminance(int luminance, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_luminance(luminance)
{}

guiDType GuiDataRenderLuminance::getType()
{
	return guiDType::renderLuminance;
}

//*** Saturation ***//

GuiDataRenderSaturation::GuiDataRenderSaturation(int saturation, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_saturation(saturation)
{}

guiDType GuiDataRenderSaturation::getType()
{
	return guiDType::renderSaturation;
}

//*** Transparency ***//
GuiDataRenderTransparency::GuiDataRenderTransparency(BlendMode mode, float _transparencyValue, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_mode(mode)
	, m_transparencyValue(_transparencyValue)
{}

guiDType GuiDataRenderTransparency::getType()
{
    return guiDType::renderTransparency;
}

//*** Transparency Options ***//

GuiDataRenderTransparencyOptions::GuiDataRenderTransparencyOptions(bool negativeEffect, bool reduceFlash, bool flashAdvanced, float flashControl, float hlt, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_negativeEffect(negativeEffect)
	, m_reduceFlash(reduceFlash)
    , m_flashAdvanced(flashAdvanced)
    , m_flashControl(flashControl)
	, m_highLuminosityThreshold(hlt)
{}

guiDType GuiDataRenderTransparencyOptions::getType()
{
	return guiDType::renderTransparencyOptions;
}

//*** Blending ***//

GuiDataRenderBlending::GuiDataRenderBlending(int blending, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_hue(blending)
{}

guiDType GuiDataRenderBlending::getType()
{
	return guiDType::renderBlending;
}

//*** PointCount ***//

GuiDataRenderPointCount::GuiDataRenderPointCount(uint64_t pointCount)
	: m_pointCount(pointCount)
{}

guiDType GuiDataRenderPointCount::getType()
{
	return guiDType::pointCountData;
}

//*** KeyPoint ***//

GuiDataRenderViewPoint::GuiDataRenderViewPoint(SafePtr<ViewPointNode> keypoint)
	: m_keypoint(keypoint)
{}
	
guiDType GuiDataRenderViewPoint::getType() 
{
	return guiDType::renderViewPoint;
}

//*** KeyPoint ***//

GuiDataRenderAnimationViewPoint::GuiDataRenderAnimationViewPoint(SafePtr<ViewPointNode> keypoint)
	: m_keypoint(keypoint)
{}
	
guiDType GuiDataRenderAnimationViewPoint::getType()
{
	return guiDType::renderViewPointAnimation;
}

/*** Flat Color ***/

GuiDataRenderFlatColor::GuiDataRenderFlatColor(const glm::vec3& color, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_color(color)
{}

GuiDataRenderFlatColor::GuiDataRenderFlatColor(const float& r, const float& g, const float& b, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_color(r,g,b)
{}

guiDType GuiDataRenderFlatColor::getType()
{
	return  guiDType::renderFlatColor;
}

/*** Post Rendering Normals ***/

GuiDataPostRenderingNormals::GuiDataPostRenderingNormals(const PostRenderingNormals& lightingParams, bool onlySimpleNormalsInfo, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_onlySimpleNormalsInfo(onlySimpleNormalsInfo)
	, m_normals(lightingParams)
{}

guiDType GuiDataPostRenderingNormals::getType()
{
	return  guiDType::renderPostRenderingNormals;
}

GuiDataRenderAmbientOcclusion::GuiDataRenderAmbientOcclusion(const PostRenderingAmbientOcclusion& aoParams, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_ao(aoParams)
{}

guiDType GuiDataRenderAmbientOcclusion::getType()
{
	return guiDType::renderAmbientOcclusion;
}

GuiDataEdgeAwareBlur::GuiDataEdgeAwareBlur(const EdgeAwareBlur& blurSettings, SafePtr<CameraNode> camera)
        : GuiDataActiveCamera(camera)
        , m_blur(blurSettings)
{}

guiDType GuiDataEdgeAwareBlur::getType()
{
        return  guiDType::renderEdgeAwareBlur;
}

GuiDataDepthLining::GuiDataDepthLining(const DepthLining& liningSettings, SafePtr<CameraNode> camera)
        : GuiDataActiveCamera(camera)
        , m_lining(liningSettings)
{}

guiDType GuiDataDepthLining::getType()
{
        return guiDType::renderDepthLining;
}

/*** Distance Ramp ***/

GuiDataRenderDistanceRampValues::GuiDataRenderDistanceRampValues(float min, float max, int steps, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_min(min)
	, m_max(max)
	, m_steps(steps)
{}

guiDType GuiDataRenderDistanceRampValues::getType()
{
	return  guiDType::renderDistanceRampValues;
}

/*** Diplay All markers texts ***/

GuiDataRenderDisplayObjectTexts::GuiDataRenderDisplayObjectTexts(const bool& display, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_display(display)
{}

guiDType GuiDataRenderDisplayObjectTexts::getType()
{
	return guiDType::renderDisplayAllMarkersTexts;
}

/*** Animation ***/

guiDType GuiDataRenderStartAnimation::getType()
{
	return guiDType::renderStartAnimation;
}

guiDType GuiDataRenderStopAnimation::getType()
{
	return guiDType::renderStopAnimation;
}

guiDType GuiDataRenderCleanAnimationList::getType()
{
	return guiDType::renderCleanAnimationList;
}

GuiDataRenderAnimationSpeed::GuiDataRenderAnimationSpeed(const uint16_t& speed)
	: m_speed(speed) 
{}

guiDType GuiDataRenderAnimationSpeed::getType()
{
	return guiDType::renderAnimationSpeed;
}

GuiDataRenderAnimationLoop::GuiDataRenderAnimationLoop(const bool& loop)
	:_loop(loop)
{}

guiDType GuiDataRenderAnimationLoop::getType()
{
	return guiDType::renderAnimationLoop;
}

GuiDataRenderRecordPerformances::GuiDataRenderRecordPerformances(const std::filesystem::path& file)
	: m_path(file)
{}

GuiDataRenderRecordPerformances::~GuiDataRenderRecordPerformances()
{}

guiDType GuiDataRenderRecordPerformances::getType()
{
	return guiDType::renderRecordPerformance;
}

GuiDataRenderImagesFormat::GuiDataRenderImagesFormat(const ImageFormat& format, bool includeAlpha)
	:m_format(format)
	, m_includeAlpha(includeAlpha)
{}

guiDType GuiDataRenderImagesFormat::getType()
{
	return guiDType::renderImagesFormat;
}

/*** DigitDisplay ***/

GuiDataRenderUnitUsage::GuiDataRenderUnitUsage(const UnitUsage& valueParameters, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_valueParameters(valueParameters)
{}

guiDType GuiDataRenderUnitUsage::getType()
{
	return guiDType::renderValueDisplay;
}

/*** Decimation Options ***/

GuiDataRenderDecimationOptions::GuiDataRenderDecimationOptions(DecimationOptions _options)
    : m_options(_options)
{}

guiDType GuiDataRenderDecimationOptions::getType()
{
    return guiDType::renderDecimationOptions;
}

/*** Octree Precision ***/

GuiDataRenderOctreePrecision::GuiDataRenderOctreePrecision(OctreePrecision precision)
	: m_precision(precision)
{}

guiDType GuiDataRenderOctreePrecision::getType()
{
	return guiDType::renderOctreePrecision;
}

/*** Navigation Constraint ***/

GuiDataRenderNavigationConstraint::GuiDataRenderNavigationConstraint(NaviConstraint constraint, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_constraint(constraint)
{}

guiDType GuiDataRenderNavigationConstraint::getType()
{
	return guiDType::renderNavigationConstraint;
}

/*** Apply Constraint ***/

GuiDataRenderApplyNavigationConstraint::GuiDataRenderApplyNavigationConstraint(bool checked, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_checked(checked)
{}

guiDType GuiDataRenderApplyNavigationConstraint::getType()
{
	return guiDType::renderApplyNavigationConstraint;
}

/*** Fov ***/

GuiDataRenderFov::GuiDataRenderFov(float fov, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_fov(fov)
{}

guiDType GuiDataRenderFov::getType()
{
	return guiDType::renderFovValueChanged;
}

/*** isNaviPanoramic ***/

GuiDataRenderIsPanoramic::GuiDataRenderIsPanoramic(bool pano) {
	m_pano = pano;
}

guiDType GuiDataRenderIsPanoramic::getType()
{
	return guiDType::renderIsPanoramic;
}

// **** GuiDataTextFilter ****

GuiDataRenderTextFilter::GuiDataRenderTextFilter(TextFilter textFilter, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_textFilter(textFilter)
{}

guiDType GuiDataRenderTextFilter::getType()
{
	return (guiDType::renderTextFilter);
}

// **** GuiDataTextTheme ****

GuiDataRenderTextTheme::GuiDataRenderTextTheme(int textTheme, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_textTheme(textTheme)
{}

guiDType GuiDataRenderTextTheme::getType()
{
	return (guiDType::renderTextTheme);
}

// **** GuiDataMarkerTextFontSize ****

GuiDataRenderTextFontSize::GuiDataRenderTextFontSize(float fontSize, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_textFontSize(fontSize)
{}

guiDType GuiDataRenderTextFontSize::getType()
{
	return (guiDType::renderTextFontSize);
}

// **** GuiDataMarkerDisplayOptions ****

GuiDataMarkerDisplayOptions::GuiDataMarkerDisplayOptions(const MarkerDisplayOptions& params, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_parameters(params)
{}

guiDType GuiDataMarkerDisplayOptions::getType()
{
    return (guiDType::renderMarkerDisplayOptions);
}

// **** GuiDataAlphaObjectsRendering ****

GuiDataAlphaObjectsRendering::GuiDataAlphaObjectsRendering(const float& alpha, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_alpha(alpha)
{}

guiDType GuiDataAlphaObjectsRendering::getType()
{
	return (guiDType::renderAlphaObjectsRendering);
}

//*** Render Measure ***//

GuiDataRenderMeasureOptions::GuiDataRenderMeasureOptions(MeasureShowMask showMask, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_showMask(showMask)
{ }

guiDType GuiDataRenderMeasureOptions::getType()
{
	return (guiDType::renderMeasureOptions);
}

//*** RenderNavigationParameters ***//

GuiDataRenderNavigationParameters::GuiDataRenderNavigationParameters(const NavigationParameters& navParams)
	: m_navParam(navParams)
{}

guiDType GuiDataRenderNavigationParameters::getType()
{
	return guiDType::renderNavigationParameters;
}

//*** PerspectiveZBounds ***//

GuiDataRenderPerspectiveZBounds::GuiDataRenderPerspectiveZBounds(const PerspectiveZBounds& zBounds, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_zBounds(zBounds)
{}

guiDType GuiDataRenderPerspectiveZBounds::getType()
{
	return guiDType::renderPerspectiveZ;
}

//*** OrthographicZBounds ***//

GuiDataRenderOrthographicZBounds::GuiDataRenderOrthographicZBounds(const OrthographicZBounds& zBounds, SafePtr<CameraNode> camera)
	: GuiDataActiveCamera(camera)
	, m_zBounds(zBounds)
{}

guiDType GuiDataRenderOrthographicZBounds::getType()
{
	return guiDType::renderOrthographicZ;
}

/*** Ramp Scale ***/

GuiDataRampScale::GuiDataRampScale(const RampScale& rampScale, const SafePtr<CameraNode>& camera)
	: GuiDataActiveCamera(camera)
	, m_rampScale(rampScale)
{}

guiDType GuiDataRampScale::getType()
{
	return  guiDType::renderRampScale;
}
