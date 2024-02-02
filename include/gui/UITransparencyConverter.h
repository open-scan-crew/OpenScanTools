#ifndef UI_TRANSPARENCY_CONVERTER
#define UI_TRANSPARENCY_CONVERTER

namespace ui::transparency
{
    float uiValue_to_trueValue(int uiTransparency);
    int trueValue_to_uiValue(float trueTransparency);
}

#endif // UI_TRANSPARENCY_CONVERTER