#include "imgui/imgui.h"
#include "impl/imgui_impl_qt.h"

#include <QtGui/QWindow>

#include <chrono>

static std::chrono::system_clock::time_point g_Time;

bool ImGui_ImplQt_InitForVulkan()
{
    g_Time = std::chrono::system_clock::now();

    // to complete
    return true;
}

void ImGui_Impl_NewFrame(float width, float height)
{
    IM_ASSERT(width > 0 && height > 0);

    ImGuiIO& io = ImGui::GetIO();

    io.DisplaySize = ImVec2((float)width, (float)height);
    io.DisplayFramebufferScale = ImVec2(1.f, 1.f);

    // Setup time step
    std::chrono::system_clock::time_point current_time = std::chrono::system_clock::now();
    float ratio = (float)std::chrono::system_clock::period::num / (float)std::chrono::system_clock::period::den;
    io.DeltaTime = (current_time - g_Time).count() > 0 ? (float)(current_time - g_Time).count() * ratio : (float)(1.0f / 60.0f);
    g_Time = current_time;

    // NOTE - The mouse and buttons update must be done directly with the system event in Qt
    // ImGui_ImplQt_UpdateMousePosAndButtons();
    // ImGui_ImplQt_UpdateMouseCursor();

    // Update Gamepads() -> not supported
}
