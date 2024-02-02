#include "gui/UITransparencyConverter.h"

#include <cmath>

#define TRANSPARENCY_BASE_FACTOR 0.1f
#define TRANSPARENCY_GROWTH_RATE -0.0554518f

float ui::transparency::uiValue_to_trueValue(int uiTransparency)
{
    return (1.f - TRANSPARENCY_BASE_FACTOR * std::exp(uiTransparency * TRANSPARENCY_GROWTH_RATE));
}

int ui::transparency::trueValue_to_uiValue(float trueTransparency)
{
    float ft = std::log((1.f - trueTransparency) / TRANSPARENCY_BASE_FACTOR) / TRANSPARENCY_GROWTH_RATE;
    return (int)round(ft);
}