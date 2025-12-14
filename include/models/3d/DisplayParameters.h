#ifndef DISPLAY_PARAMETERS_H
#define DISPLAY_PARAMETERS_H

#include "pointCloudEngine/RenderingTypes.h"
#include "pointCloudEngine/GuiRenderingTypes.h"
#include "pointCloudEngine/ShowTypes.h"
#include "gui/UnitUsage.h"
#include "utils/Color32.hpp"

#include <glm/glm.hpp>

class DisplayParameters
{
public:
    DisplayParameters() {};
    ~DisplayParameters() {};

public:
    // Point Attributes
    UiRenderMode   m_mode = UiRenderMode::RGB;
    Color32        m_backgroundColor = Color32(0, 0, 0, 0);
    float          m_pointSize = 1.f;
    float          m_deltaFilling = 0.f;
	int            m_gapFillingTexelThreshold = 2; // Default value was 4. 2 seems to be slightly better to fill gaps.
    float          m_contrast = 0.f;
    float          m_brightness = 0.f;
    float          m_saturation = 0.f;
    float          m_luminance = 0.f;
    float          m_hue = 0.f;
    glm::vec3      m_flatColor = glm::vec3(0.5f);

    // Ramp Distance
    float          m_distRampMin = 0.f;
    float          m_distRampMax = 10.f;
    int            m_distRampSteps = 16; // max 240

    // Transparency
    BlendMode      m_blendMode = BlendMode::Opaque;
    bool           m_negativeEffect = false;
    bool           m_reduceFlash = true;
    float          m_transparency = 10.f; // [0.0, 100.0]

    // Post processing
    PostRenderingNormals    m_postRenderingNormals = { true, false, true, 0.5f, 1.f };

    // GUI
    RampScale      m_rampScale = { true, false, 20 }; // GUI
    bool           m_displayGizmo = true;
    bool           m_showExamineTarget = true;

    // Objects
    float          m_alphaObject = 0.6f;
    UnitUsage      m_unitUsage = unit_usage::by_default;
    MeasureShowMask         m_measureMask = SHOW_ALL_SEGMENT | SHOW_VALUES;
    MarkerShowMask          m_markerMask = SHOW_ALL_MARKER;
    MarkerDisplayOptions    m_markerOptions = { true, 30.0, 3.0, 20.0, 50.0, 12.0 };
    TextDisplayOptions      m_textOptions = { TEXT_SHOW_INIT_BIT, 0, 13.f };
    bool                    m_displayAllMarkersTexts = true;
    bool                    m_displayAllMeasures = true;

    //Ortho
    bool            m_orthoGridActive = false;
    Color32         m_orthoGridColor = Color32(128, 128, 128);
    float           m_orthoGridStep = 5.f;
    uint32_t        m_orthoGridLineWidth = 1;

};

#endif // !DISPLAYPARAMETERS_H_
