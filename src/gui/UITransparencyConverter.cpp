#include "gui/UITransparencyConverter.h"

#include <cmath>
#include <algorithm>

#define TRANSPARENCY_BASE_FACTOR 0.1f
#define TRANSPARENCY_GROWTH_RATE -0.0554518f
#define FLASH_CONTROL_MIN_EXPOSURE 0.2f
#define FLASH_CONTROL_MAX_EXPOSURE 8.0f

float ui::transparency::uiValue_to_trueValue(int uiTransparency)
{
    return (1.f - TRANSPARENCY_BASE_FACTOR * std::exp(uiTransparency * TRANSPARENCY_GROWTH_RATE));
}

int ui::transparency::trueValue_to_uiValue(float trueTransparency)
{
    float ft = std::log((1.f - trueTransparency) / TRANSPARENCY_BASE_FACTOR) / TRANSPARENCY_GROWTH_RATE;
    return (int)round(ft);
}

float ui::flashControl::uiValue_to_exposure(int uiValue)
{
    float v = static_cast<float>(std::clamp(uiValue, 0, 100)) / 100.f;
    // Logarithmic interpolation for stronger effect near the upper end.
    float logRange = std::log(FLASH_CONTROL_MAX_EXPOSURE / FLASH_CONTROL_MIN_EXPOSURE);
    return FLASH_CONTROL_MIN_EXPOSURE * std::exp(logRange * v);
}

int ui::flashControl::exposure_to_uiValue(float exposure)
{
    float clamped = std::clamp(exposure, FLASH_CONTROL_MIN_EXPOSURE, FLASH_CONTROL_MAX_EXPOSURE);
    float logRange = std::log(FLASH_CONTROL_MAX_EXPOSURE / FLASH_CONTROL_MIN_EXPOSURE);
    float t = std::log(clamped / FLASH_CONTROL_MIN_EXPOSURE) / logRange;
    return static_cast<int>(std::round(std::clamp(t, 0.0f, 1.0f) * 100.f));
}
