#include "DesktopUI.h"

#include <algorithm>
#include <ctime>

namespace {
enum class ShellIcon { Start, Folder, Processes };

// making icon
bool IconButton(const char* label, ShellIcon icon, float width) {
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.92f, 0.5f));
    const bool clicked = ImGui::Button(label, ImVec2(width, 32));
    ImGui::PopStyleVar();
    const ImVec2 min = ImGui::GetItemRectMin();
    const float x = min.x + 8;
    const float y = min.y + 9;
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);
    if (icon == ShellIcon::Start) {
        for (int row = 0; row < 2; ++row)
            for (int col = 0; col < 2; ++col)
                draw->AddRectFilled(ImVec2(x + col * 8, y + row * 8),
                    ImVec2(x + col * 8 + 6, y + row * 8 + 6), color);
    } else if (icon == ShellIcon::Folder) {
        draw->AddRectFilled(ImVec2(x, y), ImVec2(x + 7, y + 5), color, 1);
        draw->AddRectFilled(ImVec2(x, y + 3), ImVec2(x + 16, y + 14), color, 2);
    } else {
        draw->AddRectFilled(ImVec2(x, y + 8), ImVec2(x + 4, y + 14), color);
        draw->AddRectFilled(ImVec2(x + 6, y + 3), ImVec2(x + 10, y + 14), color);
        draw->AddRectFilled(ImVec2(x + 12, y), ImVec2(x + 16, y + 14), color);
    }
    return clicked;
}
}

void DesktopUI::Initialize() {
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.PopupRounding = 8.0f;
    style.FrameRounding = 5.0f;
    style.WindowPadding = ImVec2(12, 12);
    style.FramePadding = ImVec2(10, 7);
    style.ItemSpacing = ImVec2(8, 8);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.055f, 0.075f, 0.12f, 0.98f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.075f, 0.10f, 0.16f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.13f, 0.20f, 0.30f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.20f, 0.35f, 0.49f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.43f, 0.58f, 1.0f);
    style.Colors[ImGuiCol_Header] = style.Colors[ImGuiCol_ButtonActive];
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.11f, 0.24f, 0.35f, 1.0f);
}

void DesktopUI::Launch(const char* name, bool& visible) {
    visible = true;
    pendingFocus = name;
}

void DesktopUI::PrepareAppWindow(const char* name, ImVec2 initialSize) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowSize(initialSize, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + 32, viewport->Pos.y + 32),
                            ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(180, 100),
        ImVec2(std::max(180.0f, viewport->Size.x),
               std::max(100.0f, viewport->Size.y - taskbarHeight)));
    if (pendingFocus == name) {
        ImGui::SetNextWindowCollapsed(false);
        ImGui::SetNextWindowFocus();
        pendingFocus.clear();
    }
}

void DesktopUI::KeepAppInWorkArea() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 position = ImGui::GetWindowPos();
    const ImVec2 windowSize = ImGui::GetWindowSize();
    const float maxX = viewport->Pos.x + std::max(0.0f, viewport->Size.x - windowSize.x);
    const float maxY = viewport->Pos.y + std::max(0.0f, viewport->Size.y - taskbarHeight - windowSize.y);
    ImGui::SetWindowPos(ImVec2(std::max(viewport->Pos.x, std::min(position.x, maxX)),
                               std::max(viewport->Pos.y, std::min(position.y, maxY))));
}

bool DesktopUI::Render(ImTextureID wallpaper, bool& showExplorer, bool& showTaskManager) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 origin = viewport->Pos;
    const ImVec2 size = viewport->Size;
    if (size.x <= 0 || size.y <= 0) return false;
    const bool compact = size.x < 680;
    taskbarHeight = compact ? 88.0f : 52.0f;
    const float barY = origin.y + std::max(0.0f, size.y - taskbarHeight);
    ImDrawList* background = ImGui::GetBackgroundDrawList();
    const ImVec2 bottomRight(origin.x + size.x, origin.y + size.y);
    if (wallpaper != 0)
        background->AddImage(wallpaper, origin, bottomRight);
    else
        background->AddRectFilledMultiColor(origin, bottomRight,
            IM_COL32(20, 45, 80, 255), IM_COL32(20, 45, 80, 255),
            IM_COL32(35, 95, 135, 255), IM_COL32(35, 95, 135, 255));

    bool shutdown = false;
    ImGui::SetNextWindowPos(ImVec2(origin.x, barY));
    ImGui::SetNextWindowSize(ImVec2(size.x, std::min(taskbarHeight, size.y)));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin("Taskbar", nullptr, flags);
    ImGui::PopStyleVar(2);

    const bool startShortcut = ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Escape, false);
    if (IconButton("Start", ShellIcon::Start, 80) || startShortcut)
        ImGui::OpenPopup("Start Menu");
    ImGui::SetItemTooltip("Open Start Menu (Ctrl+Esc). Tab and arrow keys navigate; Enter activates.");
    ImGui::SameLine();
    auto taskButton = [](const char* label, ShellIcon icon, bool& visible, float width) {
        if (visible) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.43f, 0.58f, 1));
        const bool clicked = IconButton(label, icon, width);
        if (visible) ImGui::PopStyleColor();
        ImGui::SetItemTooltip(visible ? "Hide window" : "Open window");
        if (clicked) visible = !visible;
        return clicked && visible;
    };
    if (taskButton("Explorer", ShellIcon::Folder, showExplorer, 100)) Launch("File Explorer", showExplorer);
    ImGui::SameLine();
    if (taskButton("Task Manager", ShellIcon::Processes, showTaskManager, 132)) Launch("Task Manager", showTaskManager);

    std::time_t now = std::time(nullptr);
    char clock[64] = "Clock unavailable";
    const std::tm* local = std::localtime(&now);
    if (local) std::strftime(clock, sizeof(clock), compact ? "%H:%M:%S" : "%a %b %d, %Y  %H:%M:%S", local);
    const float rightWidth = ImGui::CalcTextSize(clock).x + 76;
    if (!compact) {
        ImGui::SameLine(std::max(ImGui::GetCursorPosX(), size.x - rightWidth));
    } else {
        ImGui::SetCursorPosX(std::max(8.0f, size.x - rightWidth));
    }
    ImGui::TextUnformatted(clock);
    if (local) {
        char date[64];
        std::strftime(date, sizeof(date), "%A, %B %d, %Y", local);
        ImGui::SetItemTooltip("%s", date);
    }
    ImGui::SameLine();
    shutdown = ImGui::Button("PWR", ImVec2(52, 0));
    ImGui::SetItemTooltip("Shut down CSOPESY Desktop OS");

    const float menuWidth = std::min(300.0f, size.x);
    const float menuHeight = std::min(260.0f, std::max(1.0f, barY - origin.y));
    ImGui::SetNextWindowPos(ImVec2(origin.x, barY - menuHeight));
    ImGui::SetNextWindowSize(ImVec2(menuWidth, menuHeight));
    if (ImGui::BeginPopup("Start Menu", ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
        ImGui::TextUnformatted("CSOPESY OS v1.0");
        ImGui::TextDisabled("Your desktop workspace");
        ImGui::Separator();
        if (ImGui::Selectable("Open File Explorer", false, 0, ImVec2(0, 32)))
            Launch("File Explorer", showExplorer);
        if (ImGui::Selectable("Open Task Manager", false, 0, ImVec2(0, 32)))
            Launch("Task Manager", showTaskManager);
        ImGui::Separator();
        if (ImGui::Selectable("Shutdown System", false, 0, ImVec2(0, 32))) shutdown = true;
        ImGui::EndPopup();
    }
    ImGui::End();
    return shutdown;
}
