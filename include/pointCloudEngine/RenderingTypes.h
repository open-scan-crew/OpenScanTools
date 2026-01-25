#ifndef RENDERING_TYPES_H
#define RENDERING_TYPES_H

#include <string>
#include <unordered_map>

// RenderColorInput
enum class UiRenderMode
{
    Intensity,
    RGB,
    IntensityRGB_Combined,
    Grey_Colored,
    Scans_Color,
    Clusters_Color,
    Flat,
    Distance_Ramp,
    Flat_Distance_Ramp,
    Fake_Color,
    Normals_Colored,
    UiRenderMode_MaxEnum
};

enum RenderMode
{
    Intensity,
    RGB,
    IntensityRGB_Combined,
    Grey_Colored,
    Flat,
    Grey_Distance_Ramp,
    Flat_Distance_Ramp,
    Fake_Color,
    RenderMode_MaxEnum
};

enum class BlendMode {
    Opaque,
    Transparent,
    MaxEnum
};

std::unordered_map<UiRenderMode, std::string> getTradUiRenderMode();

const static std::unordered_map<UiRenderMode, RenderMode> correspUiRenderMode = {
    { UiRenderMode::Intensity, RenderMode::Intensity},
    { UiRenderMode::RGB, RenderMode::RGB},
    { UiRenderMode::IntensityRGB_Combined, RenderMode::IntensityRGB_Combined},
    { UiRenderMode::Grey_Colored, RenderMode::Grey_Colored},
    { UiRenderMode::Scans_Color, RenderMode::Grey_Colored},
    { UiRenderMode::Clusters_Color, RenderMode::Grey_Colored},
    { UiRenderMode::Flat, RenderMode::Flat},
    { UiRenderMode::Distance_Ramp, RenderMode::Grey_Distance_Ramp},
    { UiRenderMode::Flat_Distance_Ramp, RenderMode::Flat_Distance_Ramp},
    { UiRenderMode::Fake_Color, RenderMode::Fake_Color},
    { UiRenderMode::Normals_Colored, RenderMode::Intensity }
};


struct PerspectiveZBounds
{
    // On défini les bornes en Z par le plan focal puis l'écart entre le plan lointain au plan focal.
    // On peut alors calculer les valeurs [near, far] pour la matrice de projection comme suit :
    // near = pow(2, near_plan_log2)
    // far  = pow(2, near_plan_log2 + near_far_ratio_log2)
    int near_plan_log2;
    int near_far_ratio_log2;
};

// les bornes en Z sont centrées sur zéro
typedef int OrthographicZBounds;

struct PostRenderingNormals
{
    bool show;
    bool inverseTone;
    bool blendColor;
    float normalStrength;
    float gloss;
};

struct PostRenderingAmbientOcclusion
{
    bool enabled = true;
    float radius = 9.6f;
    float intensity = 0.4f;
};

struct EdgeAwareBlur
{
    bool enabled = true;
    float radius = 2.0f;
    float depthThreshold = 0.35f;
    float blendStrength = 0.25f;
    float resolutionScale = 1.0f; // 1.0 = full res, 0.5 = half res
};

struct DepthLining
{
    bool enabled = true;
    float strength = 0.60f;
    float threshold = 0.006f;
    float sensitivity = 3.0f;
    bool strongMode = false;
};

enum class ProjectionMode
{
    Perspective = 0,
    Orthographic = 1,
    MAX_ENUM
};

enum class AlignView
{
    Top,
    Bottom,
    Left,
    Right,
    Front,
    Back,
    Iso
};

enum class DecimationMode
{
    None,
    Constant,
    Adaptive
};

using OctreePrecision = float;

constexpr float kMinOctreePrecision = 0.1f;
constexpr float kMaxOctreePrecision = 10.0f;
constexpr float kDefaultOctreePrecision = 1.0f;

inline float getOctreePrecisionMultiplier(OctreePrecision precision)
{
    return precision;
}

struct DecimationOptions
{
    DecimationMode mode = DecimationMode::Constant;
    float constantValue = 0.2f;  // as percentage of the total points
    float dynamicMin = 0.1f;     // as percentage of the total points
};

struct MarkerDisplayOptions
{
    bool improveVisibility;
    double maximumDisplayDistance;
    float nearLimit;
    float farLimit;
    float nearSize;
    float farSize;
};

#endif
