#include "gui/UITransparencyConverter.h"

#include <cmath>
#include <algorithm>

#define TRANSPARENCY_BASE_FACTOR 0.1f
#define TRANSPARENCY_GROWTH_RATE -0.0554518f
#define FLASH_CONTROL_MIN_EXPOSURE 0.2f
#define FLASH_CONTROL_MAX_EXPOSURE 5.0f

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
    return FLASH_CONTROL_MIN_EXPOSURE + v * (FLASH_CONTROL_MAX_EXPOSURE - FLASH_CONTROL_MIN_EXPOSURE);
}

int ui::flashControl::exposure_to_uiValue(float exposure)
{
    float clamped = std::clamp(exposure, FLASH_CONTROL_MIN_EXPOSURE, FLASH_CONTROL_MAX_EXPOSURE);
    float t = (clamped - FLASH_CONTROL_MIN_EXPOSURE) / (FLASH_CONTROL_MAX_EXPOSURE - FLASH_CONTROL_MIN_EXPOSURE);
    return static_cast<int>(std::round(t * 100.f));
}
