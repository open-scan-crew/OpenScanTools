#include "utils/ColorimetricFilterUtils.h"

#include <algorithm>

namespace
{
    float luminance(const Color32& color)
    {
        return 0.2126f * color.Red() + 0.7152f * color.Green() + 0.0722f * color.Blue();
    }
}

namespace ColorimetricFilterUtils
{
    std::vector<Color32> getOrderedActiveColors(const ColorimetricFilterSettings& settings, UiRenderMode mode)
    {
        std::vector<Color32> colors;
        colors.reserve(4);

        if (mode == UiRenderMode::Intensity || mode == UiRenderMode::Fake_Color)
        {
            if (settings.colorsEnabled[0])
                colors.push_back(settings.colors[0]);
        }
        else
        {
            for (size_t index = 0; index < settings.colors.size(); ++index)
            {
                if (settings.colorsEnabled[index])
                    colors.push_back(settings.colors[index]);
            }
        }

        std::sort(colors.begin(), colors.end(), [](const Color32& a, const Color32& b)
        {
            return luminance(a) < luminance(b);
        });

        return colors;
    }

    std::array<OrderedColorEntry, 4> normalizeSettings(const ColorimetricFilterSettings& settings, UiRenderMode mode)
    {
        std::array<OrderedColorEntry, 4> ordered = {};
        std::vector<Color32> colors = getOrderedActiveColors(settings, mode);

        for (size_t i = 0; i < ordered.size(); ++i)
        {
            if (i < colors.size())
            {
                ordered[i].color = colors[i];
                ordered[i].enabled = true;
            }
            else
            {
                ordered[i].color = Color32(0, 0, 0);
                ordered[i].enabled = false;
            }
        }

        return ordered;
    }

    float clampTolerancePercent(float tolerance)
    {
        return std::clamp(tolerance, 0.0f, 100.0f);
    }

    glm::vec3 normalizeRgb(const Color32& color)
    {
        return glm::vec3(color.Red() / 255.0f, color.Green() / 255.0f, color.Blue() / 255.0f);
    }

    float normalizeIntensity(const Color32& color)
    {
        return color.Red() / 255.0f;
    }
}
