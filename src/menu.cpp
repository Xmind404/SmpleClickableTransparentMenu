#include "menu.h"
#include "imgui.h"
#include <cstdlib>
#include <Windows.h>

namespace Menu
{
    static bool Visible = true;

    void Draw()
    {
        // ── Toggle visibility ─────────────────────────────────────────────────
        if (GetAsyncKeyState(VK_RSHIFT) & 1)
            Visible = !Visible;

        if (!Visible)
            return;

        ImGui::SetNextWindowSize({ 300.f, 200.f }, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos({ 40.f, 40.f }, ImGuiCond_FirstUseEver);

        ImGui::Begin("whoami", nullptr,
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoScrollbar);

        // ── Identity ──────────────────────────────────────────────────────────
        ImGui::Spacing();
        ImGui::Text("User :");
        ImGui::SameLine();

        if (const char* user = getenv("USERNAME"))
        {
            ImGui::TextColored(
                ImVec4(0.f, 1.f, 0.55f, 1.f),
                "%s",
                user
            );
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Hello World! From Xmind404");

        // ── Footer ────────────────────────────────────────────────────────────
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 24.f);
        ImGui::TextDisabled("[RSHIFT] toggle");

        ImGui::End();
    }
}
