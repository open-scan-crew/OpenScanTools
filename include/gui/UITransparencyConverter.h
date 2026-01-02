#ifndef UI_TRANSPARENCY_CONVERTER
#define UI_TRANSPARENCY_CONVERTER

namespace ui::transparency
{
    float uiValue_to_trueValue(int uiTransparency);
    int trueValue_to_uiValue(float trueTransparency);
}

namespace ui::flashControl
{
    float uiValue_to_exposure(int uiValue);
    int exposure_to_uiValue(float exposure);
}

#endif // UI_TRANSPARENCY_CONVERTER
