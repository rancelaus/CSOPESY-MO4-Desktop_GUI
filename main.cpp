#include <ctime>
#include <string>
#include <cmath>

// Windows API for sound and threading
#include <windows.h>
#include <process.h>

// OpenGL & GLFW
#include "glfw/include/GLFW/glfw3.h"

// Dear ImGui
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

// Desktop UI
#include "DesktopUI.h"

// Image Loading (STB Image)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// ─── Boot Screen ────────────────────────────────────────────

enum class BootPhase { Booting, FadeOut, Desktop, ShutdownFade };

static const float BOOT_DURATION     = 5.0f;   // seconds of boot animation
static const float FADE_DURATION     = 0.8f;   // fade-to-desktop seconds
static const float SHUTDOWN_DURATION = 3.0f;   // seconds of shutdown animation

// Boot chime thread — plays a short ascending tone sequence
static unsigned __stdcall BootChimeThread(void*) {
    // Short ascending chime: C5 → E5 → G5 → C6
    Beep(523, 120);   // C5
    Sleep(30);
    Beep(659, 120);   // E5
    Sleep(30);
    Beep(784, 120);   // G5
    Sleep(30);
    Beep(1047, 220);  // C6 (held longer)
    return 0;
}

// Shutdown chime thread — descending tone sequence
static unsigned __stdcall ShutdownChimeThread(void*) {
    Beep(784, 140);   // G5
    Sleep(40);
    Beep(659, 140);   // E5
    Sleep(40);
    Beep(523, 200);   // C5
    Sleep(40);
    Beep(392, 300);   // G4 (held longer, lower)
    return 0;
}

// Helper: ease-out cubic
static float EaseOutCubic(float t) {
    t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
    return 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);
}

// Render the boot screen.  Returns true when boot is finished.
static bool RenderBootScreen(float bootTime, float fadeAlpha) {
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    const ImVec2 org  = vp->Pos;
    const ImVec2 sz   = vp->Size;
    ImDrawList* bg    = ImGui::GetBackgroundDrawList();

    // ── Dark background ──────────────────────────────────────
    bg->AddRectFilled(org, ImVec2(org.x + sz.x, org.y + sz.y),
                      IM_COL32(8, 12, 21, 255));

    // ── Subtle radial gradient glow behind logo ─────────────
    float pulse = 0.45f + 0.15f * sinf(bootTime * 2.2f);
    float glowRadius = sz.y * 0.38f;
    ImVec2 center(org.x + sz.x * 0.5f, org.y + sz.y * 0.38f);
    for (int i = 40; i >= 0; --i) {
        float r = glowRadius * (float)i / 40.0f;
        int a = (int)(pulse * 60.0f * (1.0f - (float)i / 40.0f));
        bg->AddCircleFilled(center, r, IM_COL32(30, 100, 180, a), 48);
    }

    // ── Animated Windows-style 4-square logo ────────────────
    float logoFade = EaseOutCubic(bootTime / 1.0f);
    float blockSize = 28.0f;
    float gap = 5.0f;
    float totalLogo = blockSize * 2 + gap;
    float logoX = center.x - totalLogo * 0.5f;
    float logoY = center.y - totalLogo * 0.5f;

    // Each block animates in with a staggered delay
    struct { float dx, dy; ImU32 color; float delay; } blocks[4] = {
        { 0, 0,               IM_COL32(60, 160, 240, 255),  0.0f  },  // top-left
        { blockSize + gap, 0, IM_COL32(75, 200, 110, 255),  0.15f },  // top-right
        { 0, blockSize + gap, IM_COL32(240, 180, 50, 255),  0.30f },  // bottom-left
        { blockSize + gap, blockSize + gap, IM_COL32(230, 70, 70, 255), 0.45f }  // bottom-right
    };

    for (int i = 0; i < 4; ++i) {
        float t = EaseOutCubic((bootTime - blocks[i].delay) / 0.6f);
        int alpha = (int)(t * logoFade * 255.0f);
        if (alpha <= 0) continue;
        ImU32 col = (blocks[i].color & 0x00FFFFFF) | ((ImU32)alpha << 24);

        // Scale-up animation: blocks grow from center
        float scale = 0.3f + 0.7f * t;
        float cx = logoX + blocks[i].dx + blockSize * 0.5f;
        float cy = logoY + blocks[i].dy + blockSize * 0.5f;
        float hs = blockSize * 0.5f * scale;

        bg->AddRectFilled(ImVec2(cx - hs, cy - hs),
                          ImVec2(cx + hs, cy + hs), col, 3.0f);
    }

    // ── OS Title text ───────────────────────────────────────
    float titleFade = EaseOutCubic((bootTime - 0.7f) / 0.6f);
    if (titleFade > 0.0f) {
        const char* title = "CSOPESY OS";
        ImVec2 textSz = ImGui::CalcTextSize(title);
        float tx = center.x - textSz.x * 0.5f;
        float ty = logoY + totalLogo + 20.0f;
        bg->AddText(ImVec2(tx, ty),
                    IM_COL32(200, 220, 255, (int)(titleFade * 255)), title);

        // Version subtitle
        const char* ver = "v1.0  |  Chongqing Grp8";
        ImVec2 verSz = ImGui::CalcTextSize(ver);
        bg->AddText(ImVec2(center.x - verSz.x * 0.5f, ty + textSz.y + 4),
                    IM_COL32(120, 140, 170, (int)(titleFade * 200)), ver);
    }

    // ── Progress bar ────────────────────────────────────────
    float barW = sz.x * 0.35f;
    float barH = 4.0f;
    float barX = center.x - barW * 0.5f;
    float barY = org.y + sz.y * 0.68f;
    float progress = EaseOutCubic(bootTime / BOOT_DURATION);

    // Track background
    bg->AddRectFilled(ImVec2(barX, barY),
                      ImVec2(barX + barW, barY + barH),
                      IM_COL32(40, 50, 70, 180), 2.0f);

    // Filled portion with gradient shimmer
    float fillW = barW * progress;
    if (fillW > 0) {
        bg->AddRectFilled(ImVec2(barX, barY),
                          ImVec2(barX + fillW, barY + barH),
                          IM_COL32(60, 160, 240, 255), 2.0f);

        // Shimmering highlight that sweeps across the bar
        float shimmerPos = fmodf(bootTime * 1.8f, 1.0f);
        float shimmerX = barX + fillW * shimmerPos;
        float shimmerW = fillW * 0.15f;
        int shimmerAlpha = (int)(100 * sinf(shimmerPos * 3.14159f));
        bg->AddRectFilled(ImVec2(shimmerX, barY),
                          ImVec2(shimmerX + shimmerW, barY + barH),
                          IM_COL32(180, 220, 255, shimmerAlpha), 2.0f);
    }

    // ── Status text messages that cycle ─────────────────────
    struct StatusMsg { float startTime; const char* msg; };
    StatusMsg messages[] = {
        { 0.0f, "Initializing kernel..." },
        { 0.8f, "Loading drivers..." },
        { 1.6f, "Mounting filesystems..." },
        { 2.4f, "Starting services..." },
        { 3.4f, "Loading desktop environment..." },
        { 4.2f, "Welcome!" }
    };
    const int msgCount = 6;
    int activeMsg = 0;
    for (int i = msgCount - 1; i >= 0; --i) {
        if (bootTime >= messages[i].startTime) { activeMsg = i; break; }
    }
    float msgFade = EaseOutCubic((bootTime - messages[activeMsg].startTime) / 0.4f);
    const char* statusText = messages[activeMsg].msg;
    ImVec2 sSz = ImGui::CalcTextSize(statusText);
    bg->AddText(ImVec2(center.x - sSz.x * 0.5f, barY + barH + 14),
                IM_COL32(160, 180, 210, (int)(msgFade * 220)), statusText);

    // ── Animated spinner dots ───────────────────────────────
    if (bootTime < BOOT_DURATION - 0.5f) {
        float dotY = barY + barH + 40;
        for (int d = 0; d < 3; ++d) {
            float phase = bootTime * 4.0f + (float)d * 0.7f;
            float dotAlpha = 0.3f + 0.7f * (0.5f + 0.5f * sinf(phase));
            float dotX = center.x - 12.0f + (float)d * 12.0f;
            bg->AddCircleFilled(ImVec2(dotX, dotY), 2.5f,
                                IM_COL32(60, 160, 240, (int)(dotAlpha * 200)));
        }
    }

    // ── Fade-out overlay when transitioning to desktop ──────
    if (fadeAlpha > 0.0f) {
        bg->AddRectFilled(org, ImVec2(org.x + sz.x, org.y + sz.y),
                          IM_COL32(8, 12, 21, (int)(fadeAlpha * 255)));
    }

    // ── Bottom-right build info ─────────────────────────────
    float infoFade = EaseOutCubic((bootTime - 1.5f) / 0.5f);
    if (infoFade > 0.0f) {
        const char* info = "CSOPESY MO4  |  Desktop GUI";
        ImVec2 infoSz = ImGui::CalcTextSize(info);
        bg->AddText(ImVec2(org.x + sz.x - infoSz.x - 16,
                           org.y + sz.y - infoSz.y - 12),
                    IM_COL32(80, 90, 110, (int)(infoFade * 150)), info);
    }

    return (bootTime >= BOOT_DURATION);
}

// Render the shutdown / restart screen.  Returns true when animation is finished.
static bool RenderShutdownScreen(float shutdownTime, bool isRestart) {
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    const ImVec2 org  = vp->Pos;
    const ImVec2 sz   = vp->Size;
    ImDrawList* bg    = ImGui::GetBackgroundDrawList();
    ImDrawList* fg    = ImGui::GetForegroundDrawList();

    // ── Darkening overlay that fades in over the desktop ─────
    float overlayAlpha = EaseOutCubic(shutdownTime / 0.8f);
    fg->AddRectFilled(org, ImVec2(org.x + sz.x, org.y + sz.y),
                      IM_COL32(5, 8, 16, (int)(overlayAlpha * 245)));

    ImVec2 center(org.x + sz.x * 0.5f, org.y + sz.y * 0.45f);

    // ── Spinning dots ring ───────────────────────────────────
    float spinFade = EaseOutCubic((shutdownTime - 0.3f) / 0.5f);
    if (spinFade > 0.0f) {
        float ringRadius = 22.0f;
        int dotCount = 8;
        for (int d = 0; d < dotCount; ++d) {
            float angle = (float)d / (float)dotCount * 6.28318f + shutdownTime * 3.0f;
            float dx = cosf(angle) * ringRadius;
            float dy = sinf(angle) * ringRadius;

            // Dots trail off in opacity
            float dotAlpha = (float)(dotCount - d) / (float)dotCount;
            dotAlpha *= spinFade;
            float dotRadius = 2.2f + 1.2f * dotAlpha;

            fg->AddCircleFilled(
                ImVec2(center.x + dx, center.y + dy), dotRadius,
                IM_COL32(80, 170, 240, (int)(dotAlpha * 220)));
        }
    }

    // ── Status text ──────────────────────────────────────────
    float textFade = EaseOutCubic((shutdownTime - 0.5f) / 0.5f);
    if (textFade > 0.0f) {
        const char* msg = isRestart ? "Restarting..." : "Shutting down...";
        ImVec2 msgSz = ImGui::CalcTextSize(msg);
        fg->AddText(ImVec2(center.x - msgSz.x * 0.5f, center.y + 38),
                    IM_COL32(190, 210, 240, (int)(textFade * 230)), msg);
    }

    // ── Subtle bottom text ──────────────────────────────────
    float infoFade = EaseOutCubic((shutdownTime - 1.0f) / 0.5f);
    if (infoFade > 0.0f) {
        const char* sub = isRestart
            ? "Please wait while the system restarts"
            : "Please do not power off your computer";
        ImVec2 subSz = ImGui::CalcTextSize(sub);
        fg->AddText(ImVec2(center.x - subSz.x * 0.5f, center.y + 58),
                    IM_COL32(100, 120, 150, (int)(infoFade * 160)), sub);
    }

    // ── Final black-out at the end ──────────────────────────
    if (shutdownTime > SHUTDOWN_DURATION - 0.6f) {
        float blackout = EaseOutCubic((shutdownTime - (SHUTDOWN_DURATION - 0.6f)) / 0.6f);
        fg->AddRectFilled(org, ImVec2(org.x + sz.x, org.y + sz.y),
                          IM_COL32(0, 0, 0, (int)(blackout * 255)));
    }

    return (shutdownTime >= SHUTDOWN_DURATION);
}

// Simple Texture Loader Helper Function
GLuint LoadTextureFromFile(const char* filename) {
    int w, h, channels;
    unsigned char* data = stbi_load(filename, &w, &h, &channels, 4);
    if (!data) return 0; // returns 0 if wallpaper.jpg is missing

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    
    stbi_image_free(data);
    return texture;
}

// Task Manager Dummy Data Structure
struct Process {
    const char* name;
    int pid;
    float cpu;
    int memoryMB;
};

Process processList[] = {
    { "System",            4,    0.2f, 120 },
    { "csopesy_os.exe",    1042, 1.5f, 45  },
    { "explorer.exe",      2110, 0.4f, 85  },
    { "chrome.exe",        3400, 3.2f, 450 },
    { "discord.exe",       4120, 1.1f, 210 }
};

// Main Application Entry Point
int main() {
    // Setup GLFW
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(1280, 720, "CSOPESY Desktop OS", NULL, NULL);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // enable VSync

    // Setup ImGui & Desktop UI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    DesktopUI desktopUI;
    desktopUI.Initialize();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Load Wallpaper Image (wallpaper.jpg)
    GLuint wallpaperTexture = LoadTextureFromFile("wallpaper.jpg");

    // Boot Screen State
    BootPhase bootPhase = BootPhase::Booting;
    float bootStartTime = (float)glfwGetTime();
    float fadeStartTime = 0.0f;
    bool chimePlayed = false;

    // Shutdown / Restart State
    float shutdownStartTime = 0.0f;
    bool shutdownChimePlayed = false;
    bool pendingRestart = false;

    // UI Window Toggles
    bool showExplorer       = false;
    bool showTaskManager    = false;
    bool showAboutWindow    = false;
    bool showCalendarWindow = false;

    // Main Loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        float currentTime = (float)glfwGetTime();
        bool shutdownRequested = false;

        if (bootPhase == BootPhase::Booting) {
            float bootElapsed = currentTime - bootStartTime;

            // Play boot chime once, near the start
            if (!chimePlayed && bootElapsed > 0.3f) {
                chimePlayed = true;
                _beginthreadex(NULL, 0, BootChimeThread, NULL, 0, NULL);
            }

            bool done = RenderBootScreen(bootElapsed, 0.0f);
            if (done) {
                bootPhase = BootPhase::FadeOut;
                fadeStartTime = currentTime;
            }
        }
        else if (bootPhase == BootPhase::FadeOut) {
            float fadeElapsed = currentTime - fadeStartTime;
            float fadeAlpha = 1.0f - EaseOutCubic(fadeElapsed / FADE_DURATION);

            // Render desktop behind the fade overlay
            shutdownRequested = desktopUI.Render(
                (ImTextureID)(intptr_t)wallpaperTexture,
                showExplorer, showTaskManager,
                showAboutWindow, showCalendarWindow
            );

            // Draw fade overlay on top
            ImDrawList* fg = ImGui::GetForegroundDrawList();
            const ImGuiViewport* vp = ImGui::GetMainViewport();
            fg->AddRectFilled(vp->Pos,
                ImVec2(vp->Pos.x + vp->Size.x, vp->Pos.y + vp->Size.y),
                IM_COL32(8, 12, 21, (int)(fadeAlpha * 255)));

            if (fadeElapsed >= FADE_DURATION) {
                bootPhase = BootPhase::Desktop;
            }
        }
        else if (bootPhase == BootPhase::Desktop) {
            // Normal desktop
            shutdownRequested = desktopUI.Render(
                (ImTextureID)(intptr_t)wallpaperTexture,
                showExplorer, showTaskManager,
                showAboutWindow, showCalendarWindow
            );

            // Check for shutdown or restart
            if (shutdownRequested || desktopUI.restartRequested) {
                pendingRestart = desktopUI.restartRequested;
                desktopUI.restartRequested = false;
                bootPhase = BootPhase::ShutdownFade;
                shutdownStartTime = currentTime;
                shutdownChimePlayed = false;
            }
        }
        else if (bootPhase == BootPhase::ShutdownFade) {
            float shutdownElapsed = currentTime - shutdownStartTime;

            // Play shutdown chime once
            if (!shutdownChimePlayed && shutdownElapsed > 0.1f) {
                shutdownChimePlayed = true;
                _beginthreadex(NULL, 0, ShutdownChimeThread, NULL, 0, NULL);
            }

            // Render desktop frozen underneath
            desktopUI.Render(
                (ImTextureID)(intptr_t)wallpaperTexture,
                showExplorer, showTaskManager,
                showAboutWindow, showCalendarWindow
            );

            bool done = RenderShutdownScreen(shutdownElapsed, pendingRestart);
            if (done) {
                if (pendingRestart) {
                    // Reset everything for a fresh boot
                    bootPhase = BootPhase::Booting;
                    bootStartTime = currentTime;
                    chimePlayed = false;
                    pendingRestart = false;
                    showExplorer = false;
                    showTaskManager = false;
                    showAboutWindow = false;
                    showCalendarWindow = false;
                } else {
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                }
            }
        }

        // UI SCREEN 1: File Explorer Window (Dummy File Structure)
        if (showExplorer) {
            desktopUI.PrepareAppWindow("File Explorer", ImVec2(400, 250));
            if (ImGui::Begin("File Explorer", &showExplorer)) {
                desktopUI.KeepAppInWorkArea();
                ImGui::Text("Local Disk (C:)");
                ImGui::Separator();
                ImGui::BulletText("Documents");
                ImGui::BulletText("Pictures / wallpaper.jpg");
                ImGui::BulletText("System32");
            }
            ImGui::End();
        }

        // UI SCREEN 2: Task Manager Window (Dummy Process List)
        if (showTaskManager) {
            desktopUI.PrepareAppWindow("Task Manager", ImVec2(450, 280));
            if (ImGui::Begin("Task Manager", &showTaskManager)) {
                desktopUI.KeepAppInWorkArea();
                if (ImGui::BeginTable("ProcTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("Process Name");
                    ImGui::TableSetupColumn("PID");
                    ImGui::TableSetupColumn("CPU (%)");
                    ImGui::TableSetupColumn("Memory (MB)");
                    ImGui::TableHeadersRow();

                    for (int i = 0; i < 5; i++) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0); ImGui::Text("%s", processList[i].name);
                        ImGui::TableSetColumnIndex(1); ImGui::Text("%d", processList[i].pid);
                        ImGui::TableSetColumnIndex(2); ImGui::Text("%.1f%%", processList[i].cpu);
                        ImGui::TableSetColumnIndex(3); ImGui::Text("%d MB", processList[i].memoryMB);
                    }
                    ImGui::EndTable();
                }
            }
            ImGui::End();
        }

        // UI SCREEN 3: About My PC Window
        desktopUI.RenderAboutMyPCWindow(showAboutWindow);

        // UI SCREEN 4: Calendar Window
        desktopUI.RenderCalendarWindow(showCalendarWindow);

        // Rendering Pipeline
        ImGui::Render();
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup Code
    if (wallpaperTexture) glDeleteTextures(1, &wallpaperTexture);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}