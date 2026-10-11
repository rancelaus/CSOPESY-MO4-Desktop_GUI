#pragma once

#include "imgui/imgui.h"
#include <string>

class DesktopUI {
public:
    bool restartRequested = false;

    void Initialize();
    bool Render(ImTextureID wallpaper, bool& showExplorer, bool& showTaskManager);
    bool Render(ImTextureID wallpaper, bool& showExplorer, bool& showTaskManager, bool& showAboutWindow, bool& showCalendarWindow);
    void RenderAboutMyPCWindow(bool& showAboutWindow);
    void RenderCalendarWindow(bool& showCalendarWindow);
    void PrepareAppWindow(const char* name, ImVec2 initialSize);
    void KeepAppInWorkArea();

private:
    std::string pendingFocus;
    float taskbarHeight = 52.0f;
    void Launch(const char* name, bool& visible);
};
