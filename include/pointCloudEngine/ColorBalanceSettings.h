#ifndef COLOR_BALANCE_SETTINGS_H
#define COLOR_BALANCE_SETTINGS_H

#include <cstdint>

struct ColorBalanceSettings
{
    int kMin = 8;
    int kMax = 16;
    double trimPercent = 20.0;
    bool applyOnRgb = true;
    bool applyOnIntensity = false;
    bool preserveDetails = false;
};

#endif
