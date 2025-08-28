#ifndef IMGUIUTILS_H_
#define IMGUIUTILS_H_

#include "pointCloudEngine/FrameStats.h"

#include "imgui/imgui.h"
#include <string>
#include <vector>
#include <array>

class DisplayParameters;

struct ImUtilsText
{
    std::string text;
    float wx;
    float wy;
    bool selected;
    bool hovered;
};

class ImGuiUtils
{
public:
    // Color themes
    static void giveTheme(const DisplayParameters& displayParam, ImU32& fill, ImU32& text);
    static void giveMeasureTheme(const DisplayParameters& displayParam, int segment, ImU32& fill, ImU32& text);

    // Quick math
    static void calcTextRect(const DisplayParameters& displayParam, float guiScale, const std::string& text, float wx, float wy, ImVec2& textPos, ImVec2& upLeft, ImVec2& botRight);
    static void computeRampScale(int steps, std::vector<ImU32>& rampScale);

    // Custom widgets
    static void label_big_number(const char* label, uint64_t num);
    static bool drawText(const DisplayParameters& displayParam, float guiScale, const ImUtilsText& text);
    static void plotMultiHistogram(const std::array<FrameStats, 120>& stats);
};

#endif