#include "DesktopUI.h"

#include <algorithm>
#include <cstdio>
#include <ctime>

namespace {
enum class ShellIcon { Start, Folder, Processes, About, Calendar };

// making icon
bool IconButton(const char *label, ShellIcon icon, float width) {
  ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.90f, 0.5f));
  const bool clicked = ImGui::Button(label, ImVec2(width, 32));
  ImGui::PopStyleVar();
  const ImVec2 min = ImGui::GetItemRectMin();
  const float x = min.x + 8;
  const float y = min.y + 9;
  ImDrawList *draw = ImGui::GetWindowDrawList();
  const ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);
  if (icon == ShellIcon::Start) {
    for (int row = 0; row < 2; ++row)
      for (int col = 0; col < 2; ++col)
        draw->AddRectFilled(ImVec2(x + col * 8, y + row * 8),
                            ImVec2(x + col * 8 + 6, y + row * 8 + 6), color);
  } else if (icon == ShellIcon::Folder) {
    draw->AddRectFilled(ImVec2(x, y), ImVec2(x + 7, y + 5), color, 1);
    draw->AddRectFilled(ImVec2(x, y + 3), ImVec2(x + 16, y + 14), color, 2);
  } else if (icon == ShellIcon::Processes) {
    draw->AddRectFilled(ImVec2(x, y + 8), ImVec2(x + 4, y + 14), color);
    draw->AddRectFilled(ImVec2(x + 6, y + 3), ImVec2(x + 10, y + 14), color);
    draw->AddRectFilled(ImVec2(x + 12, y), ImVec2(x + 16, y + 14), color);
  } else if (icon == ShellIcon::About) {
    // Retro Monitor Icon
    draw->AddRectFilled(ImVec2(x, y), ImVec2(x + 16, y + 10), color, 1.5f);
    const ImU32 screenBg = ImGui::GetColorU32(ImGuiCol_WindowBg);
    draw->AddRectFilled(ImVec2(x + 2, y + 2), ImVec2(x + 14, y + 8), screenBg);
    draw->AddLine(ImVec2(x + 4, y + 5), ImVec2(x + 9, y + 5), color, 1.0f);
    draw->AddRectFilled(ImVec2(x + 6, y + 10), ImVec2(x + 10, y + 12), color);
    draw->AddRectFilled(ImVec2(x + 3, y + 12), ImVec2(x + 13, y + 14), color,
                        1.0f);
  } else if (icon == ShellIcon::Calendar) {
    // Retro Calendar Icon
    draw->AddRectFilled(ImVec2(x + 3, y), ImVec2(x + 5, y + 3), color);
    draw->AddRectFilled(ImVec2(x + 11, y), ImVec2(x + 13, y + 3), color);
    draw->AddRectFilled(ImVec2(x, y + 2), ImVec2(x + 16, y + 6), color, 1.0f);
    draw->AddRect(ImVec2(x, y + 5), ImVec2(x + 16, y + 15), color, 1.0f);
    draw->AddRectFilled(ImVec2(x + 3, y + 8), ImVec2(x + 5, y + 10), color);
    draw->AddRectFilled(ImVec2(x + 7, y + 8), ImVec2(x + 9, y + 10), color);
    draw->AddRectFilled(ImVec2(x + 11, y + 8), ImVec2(x + 13, y + 10), color);
    draw->AddRectFilled(ImVec2(x + 3, y + 11), ImVec2(x + 5, y + 13), color);
    draw->AddRectFilled(ImVec2(x + 7, y + 11), ImVec2(x + 9, y + 13), color);
    draw->AddRectFilled(ImVec2(x + 11, y + 11), ImVec2(x + 13, y + 13), color);
  }
  return clicked;
}
} // namespace

void DesktopUI::Initialize() {
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::StyleColorsDark();
  ImGuiStyle &style = ImGui::GetStyle();
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

void DesktopUI::Launch(const char *name, bool &visible) {
  visible = true;
  pendingFocus = name;
}

void DesktopUI::PrepareAppWindow(const char *name, ImVec2 initialSize) {
  const ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowSize(initialSize, ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + 32, viewport->Pos.y + 32),
                          ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSizeConstraints(
      ImVec2(180, 100),
      ImVec2(std::max(180.0f, viewport->Size.x),
             std::max(100.0f, viewport->Size.y - taskbarHeight)));
  if (pendingFocus == name) {
    ImGui::SetNextWindowCollapsed(false);
    ImGui::SetNextWindowFocus();
    pendingFocus.clear();
  }
}

void DesktopUI::KeepAppInWorkArea() {
  const ImGuiViewport *viewport = ImGui::GetMainViewport();
  const ImVec2 position = ImGui::GetWindowPos();
  const ImVec2 windowSize = ImGui::GetWindowSize();
  const float maxX =
      viewport->Pos.x + std::max(0.0f, viewport->Size.x - windowSize.x);
  const float maxY =
      viewport->Pos.y +
      std::max(0.0f, viewport->Size.y - taskbarHeight - windowSize.y);
  ImGui::SetWindowPos(
      ImVec2(std::max(viewport->Pos.x, std::min(position.x, maxX)),
             std::max(viewport->Pos.y, std::min(position.y, maxY))));
}

bool DesktopUI::Render(ImTextureID wallpaper, bool &showExplorer,
                       bool &showTaskManager) {
  static bool dummyAbout = false;
  static bool dummyCalendar = false;
  return Render(wallpaper, showExplorer, showTaskManager, dummyAbout,
                dummyCalendar);
}

bool DesktopUI::Render(ImTextureID wallpaper, bool &showExplorer,
                       bool &showTaskManager, bool &showAboutWindow,
                       bool &showCalendarWindow) {
  const ImGuiViewport *viewport = ImGui::GetMainViewport();
  const ImVec2 origin = viewport->Pos;
  const ImVec2 size = viewport->Size;
  if (size.x <= 0 || size.y <= 0)
    return false;
  const bool compact = size.x < 740;
  taskbarHeight = compact ? 88.0f : 52.0f;
  const float barY = origin.y + std::max(0.0f, size.y - taskbarHeight);
  ImDrawList *background = ImGui::GetBackgroundDrawList();
  const ImVec2 bottomRight(origin.x + size.x, origin.y + size.y);
  if (wallpaper != 0)
    background->AddImage(wallpaper, origin, bottomRight);
  else
    background->AddRectFilledMultiColor(
        origin, bottomRight, IM_COL32(20, 45, 80, 255),
        IM_COL32(20, 45, 80, 255), IM_COL32(35, 95, 135, 255),
        IM_COL32(35, 95, 135, 255));

  bool shutdown = false;
  ImGui::SetNextWindowPos(ImVec2(origin.x, barY));
  ImGui::SetNextWindowSize(ImVec2(size.x, std::min(taskbarHeight, size.y)));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
  const ImGuiWindowFlags flags =
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
      ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;
  ImGui::Begin("Taskbar", nullptr, flags);
  ImGui::PopStyleVar(2);

  const bool startShortcut =
      ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Escape, false);
  if (IconButton("Start", ShellIcon::Start, 80) || startShortcut)
    ImGui::OpenPopup("Start Menu");
  ImGui::SetItemTooltip("Open Start Menu (Ctrl+Esc). Tab and arrow keys "
                        "navigate; Enter activates.");
  ImGui::SameLine();

  auto taskButton = [](const char *label, ShellIcon icon, bool &visible,
                       float width) {
    if (visible) {
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.48f, 0.68f, 1.0f));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                            ImVec4(0.24f, 0.56f, 0.78f, 1.0f));
    }
    const bool clicked = IconButton(label, icon, width);
    if (visible) {
      const ImVec2 min = ImGui::GetItemRectMin();
      const ImVec2 max = ImGui::GetItemRectMax();
      ImGui::GetWindowDrawList()->AddRectFilled(
          ImVec2(min.x + 4, max.y - 2), ImVec2(max.x - 4, max.y),
          IM_COL32(90, 200, 255, 255), 1.0f);
      ImGui::PopStyleColor(2);
    }
    ImGui::SetItemTooltip(visible ? "Hide window" : "Open window");
    if (clicked)
      visible = !visible;
    return clicked && visible;
  };

  if (taskButton("Explorer", ShellIcon::Folder, showExplorer, 100))
    Launch("File Explorer", showExplorer);
  ImGui::SameLine();
  if (taskButton("Task Manager", ShellIcon::Processes, showTaskManager, 132))
    Launch("Task Manager", showTaskManager);
  ImGui::SameLine();
  if (taskButton("About PC", ShellIcon::About, showAboutWindow, 105))
    Launch("About My PC", showAboutWindow);
  ImGui::SameLine();
  if (taskButton("Calendar", ShellIcon::Calendar, showCalendarWindow, 105))
    Launch("Calendar", showCalendarWindow);

  std::time_t now = std::time(nullptr);
  char clock[64] = "Clock unavailable";
  const std::tm *local = std::localtime(&now);
  if (local)
    std::strftime(clock, sizeof(clock),
                  compact ? "%H:%M:%S" : "%a %b %d, %Y  %H:%M:%S", local);
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
  const float menuHeight = std::min(320.0f, std::max(1.0f, barY - origin.y));
  ImGui::SetNextWindowPos(ImVec2(origin.x, barY - menuHeight));
  ImGui::SetNextWindowSize(ImVec2(menuWidth, menuHeight));
  if (ImGui::BeginPopup("Start Menu",
                        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
    ImGui::TextUnformatted("CSOPESY OS v1.0");
    ImGui::TextDisabled("Your desktop workspace");
    ImGui::Separator();
    if (ImGui::Selectable("Open File Explorer", false, 0, ImVec2(0, 30)))
      Launch("File Explorer", showExplorer);
    if (ImGui::Selectable("Open Task Manager", false, 0, ImVec2(0, 30)))
      Launch("Task Manager", showTaskManager);
    if (ImGui::Selectable("About My PC", false, 0, ImVec2(0, 30)))
      Launch("About My PC", showAboutWindow);
    if (ImGui::Selectable("Open Calendar", false, 0, ImVec2(0, 30)))
      Launch("Calendar", showCalendarWindow);
    ImGui::Separator();
    if (ImGui::Selectable("Restart System", false, 0, ImVec2(0, 30)))
      restartRequested = true;
    if (ImGui::Selectable("Shutdown System", false, 0, ImVec2(0, 30)))
      shutdown = true;
    ImGui::EndPopup();
  }
  ImGui::End();
  return shutdown;
}

void DesktopUI::RenderAboutMyPCWindow(bool &showAboutWindow) {
  if (!showAboutWindow)
    return;

  PrepareAppWindow("About My PC", ImVec2(460, 360));
  const ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
  if (ImGui::Begin("About My PC", &showAboutWindow, flags)) {
    KeepAppInWorkArea();

    // System Header / Branding
    ImGui::BeginGroup();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.35f, 0.75f, 1.0f, 1.0f));
    ImGui::TextUnformatted("CSOPESY MO4");
    ImGui::PopStyleColor();
    ImGui::TextDisabled("Operating System Emulator");
    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // System Specifications Table
    ImGui::TextColored(ImVec4(0.85f, 0.90f, 1.0f, 1.0f),
                       "System Specifications");
    if (ImGui::BeginTable("AboutSpecsTable", 2,
                          ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_BordersInnerH)) {
      ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed,
                              145.0f);
      ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

      auto renderSpecRow = [](const char *prop, const char *val) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextDisabled("%s", prop);
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(val);
      };

      renderSpecRow("OS Name / Version:", "Grp8 OS");
      renderSpecRow("Processor:", "AMD Ryzen 7 5700X3D");
      renderSpecRow("Installed RAM:", "64 GB");
      renderSpecRow("Graphics:", "AMD Radeon RX 7600XT");

      // Dynamic System Uptime
      double uptime = ImGui::GetTime();
      int hrs = static_cast<int>(uptime) / 3600;
      int mins = (static_cast<int>(uptime) % 3600) / 60;
      int secs = static_cast<int>(uptime) % 60;
      char uptimeBuffer[64];
      snprintf(uptimeBuffer, sizeof(uptimeBuffer),
               "%02d:%02d:%02d (%d hr, %d min, %d sec)", hrs, mins, secs, hrs,
               mins, secs);

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextDisabled("System Uptime:");
      ImGui::TableSetColumnIndex(1);
      ImGui::TextColored(ImVec4(0.40f, 0.88f, 0.55f, 1.0f), "%s", uptimeBuffer);

      ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Simulated Storage / Disk Usage
    ImGui::TextColored(ImVec4(0.85f, 0.90f, 1.0f, 1.0f),
                       "Simulated Disk / Storage Usage");
    const float usedGB = 42.0f;
    const float totalGB = 64.0f;
    const float storageFraction = usedGB / totalGB;
    char storageOverlay[64];
    snprintf(storageOverlay, sizeof(storageOverlay),
             "%.1f GB / %.1f GB (%.1f%% used)", usedGB, totalGB,
             storageFraction * 100.0f);

    ImGui::ProgressBar(storageFraction, ImVec2(-1.0f, 22.0f), storageOverlay);
    ImGui::TextDisabled("Drive C: [System] - 22.0 GB free of 64.0 GB total");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Bottom Action Button
    if (ImGui::Button("Close", ImVec2(80, 26))) {
      showAboutWindow = false;
    }
  }
  ImGui::End();
}

void DesktopUI::RenderCalendarWindow(bool &showCalendarWindow) {
  if (!showCalendarWindow)
    return;

  PrepareAppWindow("Calendar", ImVec2(300, 230));
  const ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
  if (ImGui::Begin("Calendar", &showCalendarWindow, flags)) {
    KeepAppInWorkArea();

    // Centered October 2026 Header
    const char *headerTitle = "October 2026";
    const float titleWidth = ImGui::CalcTextSize(headerTitle).x;
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - titleWidth) * 0.5f);
    ImGui::TextColored(ImVec4(0.35f, 0.75f, 1.0f, 1.0f), "%s", headerTitle);

    ImGui::Spacing();

    // October 2026 configuration (31 days, starts on Thursday = column 4)
    const int totalDays = 31;
    const int firstDOW =
        4; // Thursday (0 = Sun, 1 = Mon, 2 = Tue, 3 = Wed, 4 = Thu)
    const int todayDay = 11;

    static int selectedDay = 11;

    // 7-Column Table for Calendar Grid (Sun - Sat)
    const ImGuiTableFlags tableFlags =
        ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersInnerV |
        ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_SizingStretchSame;

    if (ImGui::BeginTable("MonthGrid", 7, tableFlags)) {
      static const char *const weekDays[7] = {"Sun", "Mon", "Tue", "Wed",
                                              "Thu", "Fri", "Sat"};
      for (int i = 0; i < 7; ++i) {
        ImGui::TableSetupColumn(weekDays[i],
                                ImGuiTableColumnFlags_WidthStretch);
      }
      ImGui::TableHeadersRow();

      int dayCounter = 1;
      int cellIdx = 0;
      while (dayCounter <= totalDays) {
        ImGui::TableNextRow(0, 22.0f);
        for (int col = 0; col < 7; ++col) {
          ImGui::TableSetColumnIndex(col);
          if (cellIdx < firstDOW || dayCounter > totalDays) {
            ImGui::TextDisabled(" ");
          } else {
            const bool isToday = (dayCounter == todayDay);
            const bool isSelected = (dayCounter == selectedDay);

            if (isToday) {
              ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                     IM_COL32(38, 110, 160, 230));
            } else if (isSelected) {
              ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                     IM_COL32(30, 60, 95, 180));
            }

            char cellLabel[32];
            snprintf(cellLabel, sizeof(cellLabel), "%2d##day_%d", dayCounter,
                     dayCounter);

            if (isToday) {
              ImGui::PushStyleColor(ImGuiCol_Text,
                                    IM_COL32(255, 235, 130, 255));
            }

            if (ImGui::Selectable(cellLabel, isSelected,
                                  ImGuiSelectableFlags_None, ImVec2(0, 18))) {
              selectedDay = dayCounter;
            }

            if (isToday) {
              ImGui::PopStyleColor();
              if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Today (October %d, 2026)", dayCounter);
              }
            }

            dayCounter++;
          }
          cellIdx++;
        }
      }
      ImGui::EndTable();
    }
  }
  ImGui::End();
}
