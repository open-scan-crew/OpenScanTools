#ifndef IMGUIUTILS_H_
#define IMGUIUTILS_H_

#include "imgui/imgui.h"
#include <string>

class DisplayParameters;

struct ImUtilsText
{
    std::string text;
    float wx;
    float wy;
    bool selected;
    bool hovered;
};

namespace utils
{
    void giveTheme(const DisplayParameters& displayParam, ImU32& fill, ImU32& text);
    void giveMeasureTheme(const DisplayParameters& displayParam, int segment, ImU32& fill, ImU32& text);
    void calcTextRect(const DisplayParameters& displayParam, float guiScale, const std::string& text, float wx, float wy, ImVec2& textPos, ImVec2& upLeft, ImVec2& botRight);
    bool drawText(const DisplayParameters& displayParam, float guiScale, const ImUtilsText& text);
}

#endif // QTUTILS_H_