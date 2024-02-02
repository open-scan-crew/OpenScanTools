//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2020 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
//////////////////////////////////////////////////////////////////////////////
//
// This API extends AcGi to support backgrounds, environmental objects
// and render settings
//
#pragma once

#include "acgi.h"
#include "AcGiLightTraits.h"
#pragma pack (push, 8)

#ifdef ACGIENVIRONMENT_IMPL
#define ACGIENV_IMPEXP __declspec(dllexport)
#else
#define ACGIENV_IMPEXP __declspec(dllimport)
#endif


class  AcGiSolidBackgroundTraits : public AcGiNonEntityTraits
// 
// This class enables solid background definitions to be elaborated to 
// AcGi implementations
//
{
public:
    ACRX_DECLARE_MEMBERS_EXPIMP(AcGiSolidBackgroundTraits, ACDBCORE2D_PORT);

    // solid background
    ACDBCORE2D_PORT virtual void            setColorSolid           (const AcCmEntityColor & color) = 0;
    ACDBCORE2D_PORT virtual AcCmEntityColor colorSolid              (void) const            = 0;
};

class  AcGiGradientBackgroundTraits : public AcGiNonEntityTraits
// 
// This class enables gradient background definitions to be elaborated to 
// AcGi implementations
//
{
public:
    ACRX_DECLARE_MEMBERS_EXPIMP(AcGiGradientBackgroundTraits, ACDBCORE2D_PORT);

    ACDBCORE2D_PORT virtual void            setColorTop             (const AcCmEntityColor & color) = 0;
    ACDBCORE2D_PORT virtual AcCmEntityColor colorTop                (void) const           = 0;
    ACDBCORE2D_PORT virtual void            setColorMiddle          (const AcCmEntityColor & color) = 0;
    ACDBCORE2D_PORT virtual AcCmEntityColor colorMiddle             (void) const           = 0;
    ACDBCORE2D_PORT virtual void            setColorBottom          (const AcCmEntityColor & color) = 0;
    ACDBCORE2D_PORT virtual AcCmEntityColor colorBottom             (void) const           = 0;
    ACDBCORE2D_PORT virtual void            setHorizon              (double horizon)       = 0;
    ACDBCORE2D_PORT virtual double          horizon                 (void) const           = 0;
    ACDBCORE2D_PORT virtual void            setHeight               (double height)        = 0;
    ACDBCORE2D_PORT virtual double          height                  (void) const           = 0;
    ACDBCORE2D_PORT virtual void            setRotation             (double rotation)      = 0;
    ACDBCORE2D_PORT virtual double          rotation                (void) const           = 0;
};

class  AcGiImageBackgroundTraits : public AcGiNonEntityTraits
// 
// This class enables image background definitions to be elaborated to 
// AcGi implementations
//
{
public:
    ACRX_DECLARE_MEMBERS_EXPIMP(AcGiImageBackgroundTraits, ACDBCORE2D_PORT);

    ACDBCORE2D_PORT virtual void            setImageFilename        (const ACHAR* filename) = 0;
    ACDBCORE2D_PORT virtual const ACHAR *   imageFilename           (void) const           = 0;
    ACDBCORE2D_PORT virtual void            setFitToScreen          (bool bFitToScreen)    = 0;
    ACDBCORE2D_PORT virtual bool            fitToScreen             (void) const           = 0;
    ACDBCORE2D_PORT virtual void            setMaintainAspectRatio  (bool bMaintainAspectRatio) = 0;
    ACDBCORE2D_PORT virtual bool            maintainAspectRatio     (void) const           = 0;
    ACDBCORE2D_PORT virtual void            setUseTiling            (bool bUseTiling)      = 0;
    ACDBCORE2D_PORT virtual bool            useTiling               (void) const           = 0;
    ACDBCORE2D_PORT virtual void            setXOffset              (double xOffset)       = 0;
    ACDBCORE2D_PORT virtual double          xOffset                 (void) const           = 0;
    ACDBCORE2D_PORT virtual void            setYOffset              (double yOffset)       = 0;
    ACDBCORE2D_PORT virtual double          yOffset                 (void) const           = 0;
    ACDBCORE2D_PORT virtual void            setXScale               (double xScale)        = 0;
    ACDBCORE2D_PORT virtual double          xScale                  (void) const           = 0;
    ACDBCORE2D_PORT virtual void            setYScale               (double yScale)        = 0;
    ACDBCORE2D_PORT virtual double          yScale                  (void) const           = 0;
};

class  AcGiGroundPlaneBackgroundTraits : public AcGiNonEntityTraits
// 
// This class enables ground plane background definitions to be elaborated to 
// AcGi implementations
//
{
public:
    ACRX_DECLARE_MEMBERS_EXPIMP(AcGiGroundPlaneBackgroundTraits, ACDBCORE2D_PORT);

    ACDBCORE2D_PORT virtual void            setColorSkyZenith       (const AcCmEntityColor & color) = 0;
    ACDBCORE2D_PORT virtual AcCmEntityColor colorSkyZenith          (void) const           = 0;
    ACDBCORE2D_PORT virtual void            setColorSkyHorizon      (const AcCmEntityColor & color) = 0;
    ACDBCORE2D_PORT virtual AcCmEntityColor colorSkyHorizon         (void) const           = 0;
    ACDBCORE2D_PORT virtual void            setColorUndergroundHorizon  (const AcCmEntityColor & color) = 0;
    ACDBCORE2D_PORT virtual AcCmEntityColor colorUndergroundHorizon     (void) const           = 0;
    ACDBCORE2D_PORT virtual void            setColorUndergroundAzimuth  (const AcCmEntityColor & color) = 0;
    ACDBCORE2D_PORT virtual AcCmEntityColor colorUndergroundAzimuth     (void) const           = 0;
    ACDBCORE2D_PORT virtual void            setColorGroundPlaneNear (const AcCmEntityColor & color) = 0;
    ACDBCORE2D_PORT virtual AcCmEntityColor colorGroundPlaneNear    (void) const           = 0;
    ACDBCORE2D_PORT virtual void            setColorGroundPlaneFar  (const AcCmEntityColor & color) = 0;
    ACDBCORE2D_PORT virtual AcCmEntityColor colorGroundPlaneFar     (void) const           = 0;
};

/// <summary>
/// This class enables sky background definitions to be elaborated to 
/// AcGi implementations.
/// </summary>
///
class  AcGiSkyBackgroundTraits : public AcGiNonEntityTraits
{
public:
    ACRX_DECLARE_MEMBERS_EXPIMP(AcGiSkyBackgroundTraits, ACDBCORE2D_PORT);

    /// <summary>
    /// Sets the sky parameters for this light.
    /// </summary>
    ///
    /// <param name="params">
    /// An AcGiSkyParameters object that contains the sky properties used
    /// by this background.
    /// </param>
    ACDBCORE2D_PORT virtual void setSkyParameters(const AcGiSkyParameters& params) = 0;

    /// <summary>
    /// Provides access to sky parameters for this light.
    /// </summary>
    ///
    /// <param name="params">
    /// An AcGiSkyParameters object that contains the sky properties used
    /// by this background.
    /// </param>
    ACDBCORE2D_PORT virtual void skyParameters(AcGiSkyParameters& params) const = 0;
};

/// <summary>
/// This class enables IBL background definitions to be elaborated to 
/// AcGi implementations.
/// </summary>
///
class  AcGiIBLBackgroundTraits : public AcGiNonEntityTraits
{
public:
    ACRX_DECLARE_MEMBERS_EXPIMP(AcGiIBLBackgroundTraits, ACDBCORE2D_PORT);

    /// <summary>
    /// Sets whether IBL is currently active
    /// </summary>
    /// <param name="name">
    /// True if active
    /// </param>
    ACDBCORE2D_PORT virtual void            setEnable               (const bool bEnable)    = 0;
    /// <summary>
    /// Gets whether IBL is currently active
    /// </summary>
    /// <returns>
    /// True if active
    /// </returns>
    ACDBCORE2D_PORT virtual bool            enable                  (void) const            = 0;
    /// <summary>
    /// Sets the image used for IBL
    /// </summary>
    /// <param name="name">
    /// Name of IBL image
    /// </param>
    ACDBCORE2D_PORT virtual void            setIBLImageName         (const AcString& name)  = 0;
    /// <summary>
    /// Gets the image used for IBL
    /// </summary>
    /// <returns>
    /// Name of IBL image
    /// </returns>
    ACDBCORE2D_PORT virtual AcString        IBLImageName            (void) const            = 0;
    /// <summary>
    /// Sets the rotation value for the IBL image
    /// -180 to 180 degrees
    /// </summary>
    /// <param name="rotation">
    /// Rotation value in degrees (-180 to 180)
    /// </param>
    ACDBCORE2D_PORT virtual void            setRotation             (const double rotation) = 0;
    /// <summary>
    /// Gets the rotation value for the IBL image
    /// </summary>
    /// <returns>
    /// Rotation value in degrees
    /// </returns>
    ACDBCORE2D_PORT virtual double          rotation                (void) const            = 0;
    /// <summary>
    /// Sets whether to display the IBL image as the background
    /// </summary>
    /// <param name="bdisplay">
    /// Set to true to display IBL image in background
    /// </param>
    ACDBCORE2D_PORT virtual void            setDisplayImage         (const bool bdisplay)   = 0;
    /// <summary>
    /// Gets whether to display the IBL image as the background
    /// </summary>
    /// <returns>
    /// True if IBL image is to be displayed as background
    /// </returns>
    ACDBCORE2D_PORT virtual bool            displayImage            (void) const            = 0;
    /// <summary>
    /// Sets the background to display when DisplayImage is set to false
    /// Limited to 2D backgrounds only - Solid, Gradient, Image
    /// </summary>
    /// <param name="bg">
    /// Background db Id
    /// </param>
    ACDBCORE2D_PORT virtual void            setSecondaryBackground  (const AcDbObjectId bg) = 0;
    /// <summary>
    /// Gets the background to display when DisplayImage is set to false
    /// </summary>
    /// <returns>
    /// Background db Id
    /// </returns>
    ACDBCORE2D_PORT virtual AcDbObjectId    secondaryBackground     (void) const            = 0;
};

class AcGiMaterialTexture;

class  AcGiRenderEnvironmentTraits : public AcGiNonEntityTraits
// 
// This class enables render environment definitions to be elaborated to 
// AcGi implementations
//
{
public:
    ACRX_DECLARE_MEMBERS_EXPIMP(AcGiRenderEnvironmentTraits, ACDBCORE2D_PORT);

    ACDBCORE2D_PORT virtual void            setEnable               (const bool bEnable)    = 0;
    ACDBCORE2D_PORT virtual bool            enable                  (void) const            = 0;
    ACDBCORE2D_PORT virtual void            setIsBackground         (const bool bEnable)    = 0;
    ACDBCORE2D_PORT virtual bool            isBackground            (void) const            = 0;
    ACDBCORE2D_PORT virtual void            setFogColor             (const AcCmEntityColor & color) = 0;
    ACDBCORE2D_PORT virtual AcCmEntityColor fogColor                (void) const            = 0;
    ACDBCORE2D_PORT virtual void            setNearDistance         (const double nearDist) = 0;
    ACDBCORE2D_PORT virtual double          nearDistance            (void) const            = 0;
    ACDBCORE2D_PORT virtual void            setFarDistance          (const double farDist)  = 0;
    ACDBCORE2D_PORT virtual double          farDistance             (void) const            = 0;
    ACDBCORE2D_PORT virtual void            setNearPercentage       (const double nearPct)  = 0;
    ACDBCORE2D_PORT virtual double          nearPercentage          (void) const            = 0;
    ACDBCORE2D_PORT virtual void            setFarPercentage        (const double farPct)   = 0;
    ACDBCORE2D_PORT virtual double          farPercentage           (void) const            = 0;
    ACDBCORE2D_PORT virtual void            setEnvironmentMap       (const AcGiMaterialTexture * map) = 0;
    ACDBCORE2D_PORT virtual AcGiMaterialTexture * environmentMap    (void) const = 0;
};

class  AcGiRenderSettingsTraits : public AcGiNonEntityTraits
{
public:
    ACRX_DECLARE_MEMBERS_EXPIMP(AcGiRenderSettingsTraits, ACDBCORE2D_PORT);

    ACDBCORE2D_PORT virtual void setMaterialEnabled(bool enabled) = 0;
    ACDBCORE2D_PORT virtual bool materialEnabled() const = 0;

    ACDBCORE2D_PORT virtual void setTextureSampling(bool enabled) = 0;
    ACDBCORE2D_PORT virtual bool textureSampling() const = 0;

    ACDBCORE2D_PORT virtual void setBackFacesEnabled(bool enabled) = 0;
    ACDBCORE2D_PORT virtual bool backFacesEnabled() const = 0;

    ACDBCORE2D_PORT virtual void setShadowsEnabled(bool enabled) = 0;
    ACDBCORE2D_PORT virtual bool shadowsEnabled() const = 0;

    ACDBCORE2D_PORT virtual void setDiagnosticBackgroundEnabled(bool enabled) = 0;
    ACDBCORE2D_PORT virtual bool diagnosticBackgroundEnabled() const = 0;

    // Model scale relative to meters. For example, if model is drawn in mm,
    // scaleFactor is 0.001.
    ACDBCORE2D_PORT virtual void            setModelScaleFactor     (double scaleFactor)    = 0;
    ACDBCORE2D_PORT virtual double          modelScaleFactor        (void) const            = 0;
};

// The available filtering methods (kernels) used to combine samples into
// a pixel color.
//
typedef enum {
    krBox = 0,
    krTriangle,
    krGauss,
    krMitchell,
    krLanczos
} AcGiMrFilter;

// The available methods for computing ray-traced shadows.
// 
typedef enum {
    krSimple = 0,
    krSorted,
    krSegments
} AcGiMrShadowMode;

typedef enum {
    krOff = 0,
    krGrid,
    krPhoton,
    krSamples,
    krBSP
} AcGiMrDiagnosticMode;

typedef enum {
    krObject = 0,
    krWorld,
    krCamera
} AcGiMrDiagnosticGridMode;

typedef enum {
    krDensity = 0,
    krIrradiance
} AcGiMrDiagnosticPhotonMode;

typedef enum {
    krDepth = 0,
    krSize
} AcGiMrDiagnosticBSPMode;

typedef enum {
    krHilbert = 0,
    krSpiral,
    krLeftToRight,
    krRightToLeft,
    krTopToBottom,
    krBottomToTop
} AcGiMrTileOrder;

typedef enum {
    krAutomatic = 0,
    krLogarithmic
} AcGiMrExposureType;

typedef enum {
    krFinalGatherOff = 0,
    krFinalGatherOn,
    krFinalGatherAuto
} AcGiMrFinalGatheringMode;

typedef enum {
    krExportMIOff = 0,
    krExportMIWithRender,
    krExportMIOnly
} AcGiMrExportMIMode;

class  AcGiMentalRayRenderSettingsTraits : public AcGiRenderSettingsTraits
{
public:
    ACRX_DECLARE_MEMBERS_EXPIMP(AcGiMentalRayRenderSettingsTraits, ACDBCORE2D_PORT);

    ACDBCORE2D_PORT virtual void setSampling(int min, int max) = 0;
    ACDBCORE2D_PORT virtual void sampling(int& min, int& max) const = 0;

    ACDBCORE2D_PORT virtual void setSamplingFilter(AcGiMrFilter filter, double width, double height) = 0;
    ACDBCORE2D_PORT virtual void SamplingFilter(AcGiMrFilter& filter, double& width, double& height) const = 0;

    ACDBCORE2D_PORT virtual void setSamplingContrastColor(float r, float g, float b, float a) = 0;
    ACDBCORE2D_PORT virtual void samplingContrastColor(float& r, float& g, float& b, float& a) const = 0;

    ACDBCORE2D_PORT virtual void setShadowMode(AcGiMrShadowMode mode) = 0;
    ACDBCORE2D_PORT virtual AcGiMrShadowMode shadowMode() const = 0;
    
    ACDBCORE2D_PORT virtual void setShadowMapEnabled(bool enabled) = 0;
    ACDBCORE2D_PORT virtual bool shadowMapEnabled() const = 0;
    
    ACDBCORE2D_PORT virtual void setRayTraceEnabled(bool enabled) = 0;
    ACDBCORE2D_PORT virtual bool rayTraceEnabled() const = 0;
    
    ACDBCORE2D_PORT virtual void setRayTraceDepth(int reflection, int refraction, int sum) = 0;
    ACDBCORE2D_PORT virtual void rayTraceDepth(int& reflection, int& refraction, int& sum) const = 0;

    ACDBCORE2D_PORT virtual void setGlobalIlluminationEnabled(bool enabled) = 0;
    ACDBCORE2D_PORT virtual bool globalIlluminationEnabled() const = 0;
    
    ACDBCORE2D_PORT virtual void setGISampleCount(int num) = 0;
    ACDBCORE2D_PORT virtual int giSampleCount() const = 0;
    
    ACDBCORE2D_PORT virtual void setGISampleRadiusEnabled(bool enabled) = 0;
    ACDBCORE2D_PORT virtual bool giSampleRadiusEnabled() const = 0;
    
    ACDBCORE2D_PORT virtual void setGISampleRadius(double radius) = 0;
    ACDBCORE2D_PORT virtual double giSampleRadius() const = 0;

    ACDBCORE2D_PORT virtual void setGIPhotonsPerLight(int num) = 0;
    ACDBCORE2D_PORT virtual int giPhotonsPerLight() const = 0;
    
    ACDBCORE2D_PORT virtual void setPhotonTraceDepth(int reflection, int refraction, int sum) = 0;
    ACDBCORE2D_PORT virtual void photonTraceDepth(int& reflection, int& refraction, int& sum) const = 0;
    
    ACDBCORE2D_PORT virtual void setFinalGatheringEnabled(bool enabled) = 0;
    ACDBCORE2D_PORT virtual bool finalGatheringEnabled() const = 0;
    
    ACDBCORE2D_PORT virtual void setFGRayCount(int num) = 0;
    ACDBCORE2D_PORT virtual int fgRayCount() const = 0;

    ACDBCORE2D_PORT virtual void setFGRadiusState(bool bMin, bool bMax, bool bPixels) = 0;
    ACDBCORE2D_PORT virtual void fgSampleRadiusState(bool& bMin, bool& bMax, bool& bPixels) = 0;
    
    ACDBCORE2D_PORT virtual void setFGSampleRadius(double min, double max) = 0;
    ACDBCORE2D_PORT virtual void fgSampleRadius(double& min, double& max) const = 0;
    
    ACDBCORE2D_PORT virtual void setLightLuminanceScale(double luminance) = 0;
    ACDBCORE2D_PORT virtual double lightLuminanceScale() const = 0;
    
    ACDBCORE2D_PORT virtual void setDiagnosticMode(AcGiMrDiagnosticMode mode) = 0;
    ACDBCORE2D_PORT virtual AcGiMrDiagnosticMode diagnosticMode() const = 0;

    ACDBCORE2D_PORT virtual void setDiagnosticGridMode(AcGiMrDiagnosticGridMode mode, float fSize) = 0;
    ACDBCORE2D_PORT virtual void diagnosticGridMode(AcGiMrDiagnosticGridMode& mode, float& fSize) const = 0;

    ACDBCORE2D_PORT virtual void setDiagnosticPhotonMode(AcGiMrDiagnosticPhotonMode mode) = 0;
    ACDBCORE2D_PORT virtual AcGiMrDiagnosticPhotonMode diagnosticPhotonMode() const = 0;
    
    ACDBCORE2D_PORT virtual void setDiagnosticBSPMode(AcGiMrDiagnosticBSPMode mode) = 0;
    ACDBCORE2D_PORT virtual AcGiMrDiagnosticBSPMode diagnosticBSPMode() const = 0;
    
    ACDBCORE2D_PORT virtual void setExportMIEnabled(bool enabled) = 0;
    ACDBCORE2D_PORT virtual bool exportMIEnabled() const = 0;
    
    ACDBCORE2D_PORT virtual void setExportMIFileName(const ACHAR* miName) = 0;
    ACDBCORE2D_PORT virtual const ACHAR* exportMIFileName() const = 0;

    ACDBCORE2D_PORT virtual void setTileSize(int size) = 0;
    ACDBCORE2D_PORT virtual int tileSize() const = 0;

    ACDBCORE2D_PORT virtual void setTileOrder(AcGiMrTileOrder order) = 0;
    ACDBCORE2D_PORT virtual AcGiMrTileOrder tileOrder() const = 0;

    ACDBCORE2D_PORT virtual void setMemoryLimit(int limit) = 0;
    ACDBCORE2D_PORT virtual int memoryLimit() const = 0;

    ACDBCORE2D_PORT virtual void setEnergyMultiplier(float fScale) = 0;
    ACDBCORE2D_PORT virtual float energyMultiplier() const = 0;

    ACDBCORE2D_PORT virtual void setProgressMonitor(void* pMonitor) = 0;
    ACDBCORE2D_PORT virtual const void* progressMonitor(void) const = 0;

    ACDBCORE2D_PORT virtual void setExposureType(AcGiMrExposureType type) = 0;
    ACDBCORE2D_PORT virtual AcGiMrExposureType exposureType() const = 0;

    ACDBCORE2D_PORT virtual void setFinalGatheringMode(AcGiMrFinalGatheringMode mode) = 0;
    ACDBCORE2D_PORT virtual AcGiMrFinalGatheringMode finalGatheringMode() const = 0;

    ACDBCORE2D_PORT virtual void setShadowSamplingMultiplier(double multiplier) = 0;
    ACDBCORE2D_PORT virtual double shadowSamplingMultiplier() const = 0;

    ACDBCORE2D_PORT virtual void setExportMIMode(AcGiMrExportMIMode mode) = 0;
    ACDBCORE2D_PORT virtual AcGiMrExportMIMode exportMIMode() const = 0;
};

/// <summary>
/// Render Quit Conditions
/// </summary>
typedef enum
{
    /// <summary>
    /// Use render iteration as a quit condition
    /// The corresponding render level value needs to be set
    /// </summary>
    krEQuitByRenderLevel = 0,
    /// <summary>
    /// Use render time as a quit condition
    /// The corresponding target render time needs to be set
    /// </summary>
    krEQuitByRenderTime
}AcGiQuitCondition;

/// <summary>
/// Lighting Mode.
/// </summary>
typedef enum
{
    /// <summary>
    /// Global illumination off.
    /// Glossy reflection and refraction off.
    /// Simplified strategy for reflections/refractions to reduce noise.
    /// The trade off is that reflections and refractions may be less accurate compared to the Basic and Advanced lighting modes.
    /// </summary>
    krESimplistic = 0,
    /// <summary>
    /// Global illumination (indirect diffuse lighting) on.
    /// Glossy reflection and refraction off.
    /// </summary>
    krEBasic,
    /// <summary>
    /// Global illumination (indirect diffuse lighting) on.
    /// Glossy reflection and refraction on.
    /// </summary>
    krEAdvanced
}AcGiLightingMode;

/// <summary>
/// Filter Type.
/// </summary>
typedef enum 
{
    /// <summary>
    /// Box filter type
    /// Default size: 1.0
    /// </summary>
    krEBox = 0,
    /// <summary>
    /// Triangle filter type
    /// Default size: 2.0
    /// </summary>
    krETriangle,
    /// <summary>
    /// Gaussian filter type
    /// Default size: 3.0
    /// </summary>
    krEGaussian,
    /// <summary>
    /// Lanczos filter type
    /// Default size: 4.0
    /// </summary>
    krELanczos,
    /// <summary>
    /// Mitchell filter type
    /// Default size: 4.0
    /// </summary>
    krEMitchell
}AcGiFilterType;

/// <summary>
/// Abstract interface class that controls all the rendering settings.
/// </summary>
class  AcGiRapidRTRenderSettingsTraits : public AcGiNonEntityTraits 
{
public:
    ACRX_DECLARE_MEMBERS_EXPIMP(AcGiRapidRTRenderSettingsTraits, ACDBCORE2D_PORT);

    /// <summary>
    /// Two general options are provided to control the rendering quality. 
    /// One is by rendering time, the other by rendering level. 
    /// For either option, the bigger value set, the better quality we can get, which also means longer rendering time. 
    /// This function is to set the option.
    /// /summary>
    /// <param name="quitCondition">Set the quit condition that controls the rendering option, either by time or by level. </param>
    /// <returns>void</returns>
    ACDBCORE2D_PORT virtual void setQuitCondition(AcGiQuitCondition quitCondition) = 0;
    /// <summary>Get the quit condition</summary>
    /// <returns>Return the quit condition</returns>
    ACDBCORE2D_PORT virtual AcGiQuitCondition quitCondition() const = 0;

    /// <summary>
    /// If choosing to use the option of render time to control render quality, we can use this function to set the desired rendering time.
    /// </summary>
    /// <param name="renderTime">Desired render time to be set, which is by second. The range is from 1 minute to 1440 minutes.</param>
    /// <returns>void</returns>
    ACDBCORE2D_PORT virtual void setDesiredRenderTime(int renderTime) = 0;
    /// <summary>Get the desired render time.</summary>
    /// <returns>Return the desired render time.</returns>
    ACDBCORE2D_PORT virtual int desiredRenderTime() const = 0;

    /// <summary>If choosing to use the option of render level to control render quality, we can use this function to set the desired rendering level.</summary>
    /// <param name="renderLevel">Desired render level to be set. The range is from 1 to 50.</param>
    /// <returns>void</returns>
    ACDBCORE2D_PORT virtual void setDesiredRenderLevel(int renderLevel) = 0;
    /// <summary>Get the desired render level.</summary>
    /// <returns>Return the desired render level.</returns>
    ACDBCORE2D_PORT virtual int desiredRenderLevel() const = 0;

    /// <summary>
    /// Turn on basic lighting or advanced lighting according the lighting mode set. 
    /// Basic lighting uses ambient occlusion and a simpler model for ambient light and IBL. 
    /// Advanced lighting uses full global illumination and a more advanced model for ambient light and IBL.
    /// </summary>
    /// <param name="mode">Desired lighting mode to be set.</param>
    /// <returns>void</returns>
    ACDBCORE2D_PORT virtual void setLightingMode(AcGiLightingMode mode) = 0;
    /// <summary>Get the lighting mode being set.</summary>
    /// <returns>Return the lighting mode being set.</returns>
    ACDBCORE2D_PORT virtual AcGiLightingMode lightingMode() const = 0;

    /// <summary>
    /// Set the filter type applied to the image samples when super sampling. Different filter type favors different filter width / height.
    /// </summary>
    /// <param name="filterInfo">Filter type to be set.</param>
    /// <returns>void</returns>
    ACDBCORE2D_PORT virtual void setFilterType(AcGiFilterType filterInfo) = 0;
    /// <summary>Get the current filter type.</summary>
    /// <returns>Return the current filter type.</returns>
    ACDBCORE2D_PORT virtual AcGiFilterType filterType() const = 0;

    /// <summary>
    /// Sets the filter width. 1.0 means one pixel (image sample).
    /// Recommended default sizes: box = 1.0, triangle = 2.0, Gaussian = 3.0, Lanczos = 4.0, Mitchell = 4.0
    /// </summary>
    /// <param name="width">Filter width to be set.</param>
    /// <returns>void</returns>
    ACDBCORE2D_PORT virtual void setFilterWidth(float width) = 0;
    /// <summary>Get the current filter width.</summary>
    /// <returns>Return the current filter width.</returns>
    ACDBCORE2D_PORT virtual float filterWidth() const = 0;

    /// <summary>
    /// Sets the filter height. 1.0 means one pixel (image sample).
    /// Recommended default sizes: box = 1.0, triangle = 2.0, Gaussian = 3.0, Lanczos = 4.0, Mitchell = 4.0
    /// </summary>
    /// <param name="height">Filter height to be set.</param>
    /// <returns>void</returns>
    ACDBCORE2D_PORT virtual void setFilterHeight(float height) = 0;
    /// <summary>
    /// Get the current filter height
    /// </summary>
    /// <returns>Current filter height</returns>
    ACDBCORE2D_PORT virtual float filterHeight() const = 0;
};

/// <summary>
/// Container class for all tone operator parameters.
/// </summary>
///
class  AcGiToneOperatorParameters : public AcRxObject
{
public:
    ACRX_DECLARE_MEMBERS_EXPIMP(AcGiToneOperatorParameters, ACDBCORE2D_PORT);

    ACDBCORE2D_PORT AcGiToneOperatorParameters();
    ACDBCORE2D_PORT ~AcGiToneOperatorParameters();

    ACDBCORE2D_PORT AcGiToneOperatorParameters& operator=(const AcGiToneOperatorParameters& params);
    ACDBCORE2D_PORT bool operator==(const AcGiToneOperatorParameters& params) const;

    /// <summary>
    /// Indicates whether or not the tone operator is active.
    /// </summary>
    ///
    /// <param name="active">
    /// If true, the tone operator is active.
    /// </param>
    ACDBCORE2D_PORT virtual void setIsActive(bool active);

    /// <summary>
    /// Is the tone operator active?
    /// </summary>
    ///
    /// <returns>
    /// True if the tone operator is active.
    /// </returns>
    ACDBCORE2D_PORT virtual bool isActive() const;

    /// <summary>
    /// Reserved for future use.
    /// </summary>
    ACDBCORE2D_PORT virtual void setChromaticAdaptation(bool enable);

    /// <summary>
    /// Reserved for future use.
    /// </summary>
    ACDBCORE2D_PORT virtual bool chromaticAdaptation() const;
    
    /// <summary>
    /// Reserved for future use.
    /// </summary>
    ACDBCORE2D_PORT virtual void setColorDifferentiation(bool enable);

    /// <summary>
    /// Reserved for future use.
    /// </summary>
    ACDBCORE2D_PORT virtual bool colorDifferentiation() const;

    /// <summary>
    /// Sets the reference white color
    /// </summary>
    /// <param name="color">Reference White</param>
    ACDBCORE2D_PORT virtual void setWhiteColor(const AcCmColor& color);

    /// <summary>
    /// Gets the reference white color
    /// </summary>
    ACDBCORE2D_PORT virtual AcCmColor whiteColor() const;
    
    /// <summary>
    /// Specifies if the background should be processed by exposure control at render time.
    /// </summary>
    ///
    /// <param name="processBG">
    /// Set to true to enable background processing, otherwise false.
    /// </param>
    ///
    /// <remarks>
    /// Only applies if the tone operator is active.
    /// </remarks>
    ACDBCORE2D_PORT virtual void setProcessBackground(bool processBG);

    /// <summary>
    /// Is background processing enabled?
    /// </summary>
    ///
    /// <returns>
    /// True if background processing is enabled.
    /// </returns>
    ///
    /// <remarks>
    /// Only applies if the tone operator is active.
    /// </remarks>
    ACDBCORE2D_PORT virtual bool processBackground() const;

    /// <summary>
    /// Sets the brightness for the tone operator.
    /// </summary>
    ///
    /// <param name="brightness">
    /// The brightness for the tone operator.
    /// </param>
    ///
    /// <returns>
    /// Returns true if a valid brightness value is passed in.
    /// </returns>
    ///
    /// <remarks>
    /// This value only applies if the tone operator is active.
    /// </remarks>
    ACDBCORE2D_PORT virtual bool setBrightness(double brightness);

    /// <summary>
    /// The brightness for the tone operator.
    /// </summary>
    ///
    /// <returns>
    /// The brightness for the tone operator.
    /// </returns>
    ///
    /// <remarks>
    /// Only applies if the tone operator is active.
    /// </remarks>
    ACDBCORE2D_PORT virtual double brightness() const;

    /// <summary>
    /// Sets the contrast for the tone operator.
    /// </summary>
    ///
    /// <param name="contrast">
    /// The contrast for the tone operator.
    /// </param>
    ///
    /// <returns>
    /// Returns true if a valid contrast value is passed in.
    /// </returns>
    ///
    /// <remarks>
    /// This value only applies if the tone operator is active.
    /// </remarks>
    ACDBCORE2D_PORT virtual bool setContrast(double contrast);

    /// <summary>
    /// The contrast for the tone operator.
    /// </summary>
    ///
    /// <returns>
    /// The contrast for the tone operator.
    /// </returns>
    ///
    /// <remarks>
    /// Only applies if the tone operator is active.
    /// </remarks>
    ACDBCORE2D_PORT virtual double contrast() const;

    /// <summary>
    /// Sets the mid tones for the tone operator.
    /// </summary>
    ///
    /// <param name="midTones">
    /// The mid tones for the tone operator.
    /// </param>
    ///
    /// <returns>
    /// Returns true if a valid midtones value is passed in.
    /// </returns>
    ///
    /// <remarks>
    /// This value only applies if the tone operator is active.
    /// </remarks>
    ACDBCORE2D_PORT virtual bool setMidTones(double midTones);

    /// <summary>
    /// The mid tones for the tone operator.
    /// </summary>
    ///
    /// <returns>
    /// The mid tones for the tone operator.
    /// </returns>
    ///
    /// <remarks>
    /// Only applies if the tone operator is active.
    /// </remarks>
    ACDBCORE2D_PORT virtual double midTones() const;
    
    /// <summary>
    /// The exterior daylight mode.
    /// </summary>
    enum ExteriorDaylightMode {
        /// <summary>
        /// Off.
        /// </summary>
        kDaylightOff = 0,
        /// <summary>
        /// On.
        /// </summary>
        kDaylightOn,
        /// <summary>
        /// Auto - key off of the sun/sky background status.
        /// </summary>
        kDaylightAuto
    };

    /// <summary>
    /// Indicates whether or not exterior daylight is enabled.
    /// </summary>
    ///
    /// <param name="mode">
    /// If kDaylightOff, daylight is disabled. If kDaylightOn, daylight is enabled.  If kDaylightAuto, exterior daylight is enabled if a sky background or a sun light is enabled.
    /// </param>
    ///
    /// <returns>
    /// Returns true if a valid exterior daylight mode is passed in.
    /// </returns>
    ///
    /// <remarks>
    /// This value only applies if the tone operator is active.
    /// </remarks>
    ACDBCORE2D_PORT virtual bool setExteriorDaylight(ExteriorDaylightMode mode);

    /// <summary>
    /// Is exterior daylight enabled?
    /// </summary>
    ///
    /// <returns>
    /// True if exterior daylight is enabled.
    /// </returns>
    ///
    /// <remarks>
    /// Only applies if the tone operator is active.
    /// </remarks>
    ACDBCORE2D_PORT virtual ExteriorDaylightMode exteriorDaylight() const;
    
    /// <summary>
    /// Copies member data from source object
    /// </summary>
    ///
    /// <returns>
    /// Acad::eOk if copy is successful
    /// </returns>
    ACDBCORE2D_PORT Acad::ErrorStatus copyFrom(const AcRxObject* other) override;

private:
    bool                    mbIsActive;
    bool                    mbChromaticAdaptation;
    bool                    mbColorDifferentiation;
    AcCmColor               mWhiteColor;
    bool                    mbProcessBackground;
    double                  mBrightness;
    double                  mContrast;
    double                  mMidTones;
    ExteriorDaylightMode    mExteriorDaylight;
};

/// <summary>
/// Container class for all photographic exposure parameters.
/// </summary>
/// <remarks>
/// Properties defined here used by RENDERENGINE=1. 
/// Base class properties used by RENDERENGINE=0.
/// </remarks>
class  AcGiPhotographicExposureParameters : public AcGiToneOperatorParameters
{
public:
    ACRX_DECLARE_MEMBERS_EXPIMP(AcGiPhotographicExposureParameters, ACDBCORE2D_PORT);

    ACDBCORE2D_PORT AcGiPhotographicExposureParameters();
    ACDBCORE2D_PORT ~AcGiPhotographicExposureParameters();

    ACDBCORE2D_PORT AcGiPhotographicExposureParameters& operator=(const AcGiPhotographicExposureParameters& params);
    ACDBCORE2D_PORT bool operator==(const AcGiPhotographicExposureParameters& params) const;
    /// <summary>
    /// Sets Exposure Value for photographic exposure parameters.
    /// This value is a combination of a camera's shutter speed (exposure time) and f-number (depth of field) 
    /// and determines the amount of motion blur in a scene.
    /// </summary>
    ///
    /// <param name="exposure">
    /// Exposure Value for photographic exposure parameters.
    /// </param>
    ACDBCORE2D_PORT bool setExposure(double exposure);

    /// <summary>
    /// Gets Exposure for photographic exposure parameters.
    /// </summary>
    ///
    /// <returns>
    /// Exposure for photographic exposure parameters
    /// </returns>
    ACDBCORE2D_PORT double exposure(void) const;

    /// <summary>
    /// Sets White Point for photographic exposure parameters in Kelvins
    /// Affects the chromaticity of a scene
    /// </summary>
    ///
    /// <param name="whitePoint">
    /// Temperature of White Point in Kelvins
    /// </param>
    ACDBCORE2D_PORT bool setWhitePoint(double whitePoint);

    /// <summary>
    /// Gets White Point for photographic exposure parameters
    /// </summary>
    ///
    /// <returns>
    /// Temperature of White Point in Kelvins
    /// </returns>
    ACDBCORE2D_PORT double whitePoint(void) const;

    /// <summary>
    /// Sets exact brightness and updates exposure calculated from brightness
    /// </summary>
    ACDBCORE2D_PORT bool setBrightness(double) override;

    /// <summary>
    /// Copies member data from source object
    /// </summary>
    ///
    /// <returns>
    /// Acad::eOk if copy is successful
    /// </returns>
    ACDBCORE2D_PORT Acad::ErrorStatus copyFrom(const AcRxObject*) override;

public:
    /// <summary>
    /// Converts EV to a calibrated Brightness value
    /// </summary>
    /// <param name="exp">Exposure Value</param>
    /// <returns>Brightness value</returns>
    ACDBCORE2D_PORT static double convertExposureToBrightness(double exp);
    /// <summary>
    /// Converts Brightness value to EV
    /// </summary>
    /// <param name="brt">Brightness Value</param>
    /// <returns>Exposure value</returns>
    ACDBCORE2D_PORT static double convertBrightnessToExposure(double brt);

private:
    double                    mExposure;
    double                    mWhitePoint;
};

#pragma pack (pop)
