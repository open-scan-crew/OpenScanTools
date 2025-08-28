#define IMGUI_DEFINE_MATH_OPERATORS
#include "utils/ImGuiUtils.h"
#include "imgui/imgui_internal.h"
#include "models/3d/DisplayParameters.h"
#include "utils/ColorConversion.h"

void ImGuiUtils::giveTheme(const DisplayParameters& displayParam, ImU32& fill, ImU32& text)
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

void ImGuiUtils::giveMeasureTheme(const DisplayParameters& displayParam, int segment, ImU32& fill, ImU32& text)
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

void ImGuiUtils::calcTextRect(const DisplayParameters& displayParam, float guiScale, const std::string& text, float wx, float wy, ImVec2& textPos, ImVec2& upLeft, ImVec2& botRight)
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

void ImGuiUtils::computeRampScale(int steps, std::vector<ImU32>& rampScale)
{
    for (int s = 0; s < steps; ++s)
    {
        float q = (float)s / (steps - 1);
        float hue = (q * 2.f) / 3.f;
        glm::vec3 rgb = utils::color::hsl2rgb(glm::vec3(hue, 1.f, 0.5f));
        rampScale.push_back(IM_COL32(rgb.x * 255, rgb.y * 255, rgb.z * 255, 255));
    }
}

void ImGuiUtils::label_big_number(const char* label, uint64_t num)
{
    std::string str;
    constexpr uint64_t GG = 1000000000000000000; // 10^18
    bool zeroPadding = false;
    for (uint64_t d = GG; d > 0; d /= 1000)
    {
        uint64_t pack = num / d;
        if (pack == 0 && d > 1 && !zeroPadding)
            continue;
        num = num - pack * d;
        char buffer[8];
        if (zeroPadding)
            snprintf(buffer, sizeof(buffer), " %03llu", pack);
        else
        {
            snprintf(buffer, sizeof(buffer), "%llu", pack);
            zeroPadding = true;
        }
        str += buffer;
    }

    ImGui::LabelText(label, "%s", str.c_str());
}

bool ImGuiUtils::drawText(const DisplayParameters& displayParam, float guiScale, const ImUtilsText& text)
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

void ImGuiUtils::plotMultiHistogram(const std::array<FrameStats, 120>& stats)
{
    // CONSTANT PARAMETERS
    // General
    constexpr float size_y = 120.f;

    // Axis
    constexpr int axis_max_div = 6;
    constexpr char axis_label_unit[] = "time [ms]";
    constexpr float axis_padding = 5.f;

    // Values
    constexpr float axis_value_base = 2.f;

    // Colors
    ImColor colors[] = {
        { 0.90f, 0.20f, 0.75f, 1.0f },  // purple
        { 0.50f, 0.80f, 0.30f, 1.0f }, // green
        { 0.10f, 0.60f, 0.90f, 1.0f }, // blue
        { 0.80f, 0.00f, 0.20f, 1.0f }   // red
    };

    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;
    const ImGuiStyle& style = g.Style;

    float maxRenderTime = 0.f;
    for (int i = 0; i < stats.size(); ++i)
    {
        maxRenderTime = std::max(maxRenderTime, stats[i].getCumulValue(2));
    }

    // Dynamically adjust the axis maximum and graduation to the data
    int axis_div = (int)std::ceil(maxRenderTime / axis_value_base);
    int axis_div_factor = (axis_div - 1) / axis_max_div + 1;
    int axis_div_count = axis_div / axis_div_factor;
    float axis_div_value = axis_value_base * axis_div_factor;
    float axis_value_max = axis_value_base * axis_div;

    float step_x = 3.f;
    float L = stats.size() * step_x; // length of the histogram

    char grad_text[16];
    snprintf(grad_text, 16, "%.0f", axis_value_max);

    // 
    ImVec2 grad_text_size = ImGui::CalcTextSize(grad_text, NULL, true);

    ImVec2 axis_size = grad_text_size + ImVec2(axis_padding, size_y);
    ImVec2 frame_size(L, size_y);
    ImVec2 axis_min = window->DC.CursorPos;
    ImVec2 axis_max = axis_min + axis_size;
    ImVec2 frame_min(axis_max.x, window->DC.CursorPos.y + grad_text_size.y / 2.f);
    ImVec2 frame_max = frame_min + frame_size;

    ImRect total_bb(axis_min, ImVec2(frame_max.x, axis_max.y));
    ImGui::ItemSize(total_bb, style.FramePadding.y);

    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Histogram background
    ImGui::RenderFrame(frame_min, frame_max, ImGui::GetColorU32(ImGuiCol_FrameBg), true, 0.f);

    // Draw Axis & Frame div
    const ImU32 col_line = ImGui::GetColorU32(ImGuiCol_PlotLines);
    for (int g = 0; g <= axis_div_count; ++g)
    {
        float t = g * axis_div_value;
        float y = ImLerp(frame_max.y, frame_min.y, t / axis_value_max);
        ImVec2 pos0(frame_min.x - 0.5f, y);
        ImVec2 pos1(frame_max.x - 0.5f, y);
        dl->AddLine(pos0, pos1, col_line, 1.0f);

        snprintf(grad_text, 16, "%.0f", t);
        grad_text_size = ImGui::CalcTextSize(grad_text, NULL, true);
        ImVec2 text_pos(axis_min.x, y - grad_text_size.y / 2.0f);
        dl->AddText(NULL, window->CalcFontSize(), text_pos, IM_COL32_WHITE, grad_text);
    }

    // Draw Histogram
    ImVec2 zero_histo(frame_min.x, frame_max.y);
    for (int j = 0; j < 3; ++j)
    {
        float t = stats[0].getCumulValue(j) / axis_value_max;
        ImVec2 pos1 = zero_histo + ImVec2(0.0f + 0.5f, -t * frame_size.y);
        for (int i = 1; i < stats.size(); ++i)
        {
            ImVec2 pos0 = pos1;
            const FrameStats& f = stats[i];
            t = stats[i].getCumulValue(j) / axis_value_max;
            pos1 = zero_histo + ImVec2(i * step_x + 0.5f, -t * frame_size.y);
            dl->AddLine(pos0, pos1, colors[j], 1.0f);
        }
    }
}