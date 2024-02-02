#include "utils/ImGuiUtils.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"
#include "models/3d/DisplayParameters.h"

namespace utils
{
	void giveTheme(const DisplayParameters& displayParam, ImU32& fill, ImU32& text)
	{
        switch (displayParam.m_textOptions.m_textTheme) {
        case 0:
            fill = IM_COL32(48, 48, 48, 192);
            text = IM_COL32_WHITE;
            break;
        case 1:
            fill = IM_COL32(208, 208, 208, 192);
            text = IM_COL32_BLACK;
            break;
        default:
            fill = IM_COL32(48, 48, 48, 192);
            text = IM_COL32_WHITE;
            break;
        }
	}

	void giveMeasureTheme(const DisplayParameters& displayParam, int segment, ImU32& fill, ImU32& text)
	{
        ImU32 colorsText[3] = {
           IM_COL32(255, 237, 72, 255), // light yellow (hue = 36)
           IM_COL32(255, 106, 255, 255), // light pink   (hue = 200)
           IM_COL32(106, 181, 255, 255)  // light cyan   (hue = 140)
        };

        ImU32 colorsFill[3] = {
            IM_COL32(255, 237, 72, 192), // light yellow (hue = 36)
            IM_COL32(255, 106, 255, 192), // light pink   (hue = 200)
            IM_COL32(106, 181, 255, 192)  // light cyan   (hue = 140)
        };

        switch (displayParam.m_textOptions.m_textTheme) {
        case 0:
            fill = IM_COL32(48, 48, 48, 192);
            text = colorsText[segment];
            break;
        case 1:
            fill = colorsFill[segment];
            text = IM_COL32_BLACK;
            break;
        default:
            break;
        }
	}

    void calcTextRect(const DisplayParameters& displayParam, float guiScale, const std::string& text, float wx, float wy, ImVec2& textPos, ImVec2& upLeft, ImVec2& botRight)
    {
        ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
        float defaultFontSize = ImGui::CalcTextSize("").y;

        float fontSize = displayParam.m_textOptions.m_textFontSize;
        textSize.x *= fontSize / defaultFontSize;
        textSize.y *= fontSize / defaultFontSize;

        textPos = ImVec2(wx - textSize.x * 0.5f, wy);

        ImVec2 margin(4.f * guiScale, 2.f * guiScale);
        textPos.y += margin.y;
        upLeft = ImVec2(textPos.x - margin.x, textPos.y - margin.y);
        botRight = ImVec2(textPos.x + textSize.x + margin.x, textPos.y + textSize.y + margin.y);
    }

    bool drawText(const DisplayParameters& displayParam, float guiScale, const ImUtilsText& text)
    {
        if (text.text == "")
            return false;

        ImDrawList* dl = ImGui::GetWindowDrawList();
        float fontSize = displayParam.m_textOptions.m_textFontSize;
        // Compute the text position and the text background

        ImVec2 textPos, upLeft, botRight;
        calcTextRect(displayParam, guiScale, text.text, text.wx, text.wy, textPos, upLeft, botRight);

        ImU32 textFillColor, textTextColor;
        giveTheme(displayParam, textFillColor, textTextColor);

        if (text.selected || text.hovered)
        {
            ImVec2 A = upLeft - ImVec2(1.f * guiScale, 1.f * guiScale);
            ImVec2 B = botRight + ImVec2(1.f * guiScale, 1.f * guiScale);
            dl->AddRect(A, B, text.selected ? IM_COL32(234, 178, 0, 255) : IM_COL32(150, 13, 222, 255), 2.f, 0, 1.5f);
        }
        dl->AddRectFilled(upLeft, botRight, textFillColor, 2.f * guiScale);
        dl->AddText(NULL, fontSize, textPos, textTextColor, text.text.c_str());

        return true;
    }
}