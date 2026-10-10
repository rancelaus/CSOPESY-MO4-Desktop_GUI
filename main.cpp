#include <ctime>
#include <string>

// OpenGL & GLFW
#include "glfw/include/GLFW/glfw3.h"

// Dear ImGui
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

// Image Loading (STB Image)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Load Wallpaper Image (wallpaper.jpg)
    GLuint wallpaperTexture = LoadTextureFromFile("wallpaper.jpg");

    // UI Window Toggles
    bool showStartMenu   = false;
    bool showExplorer    = false;
    bool showTaskManager = false;

    // Main Loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        float screenW = (float)display_w;
        float screenH = (float)display_h;

        // Desktop Wallpaper Rendering
        ImDrawList* bgList = ImGui::GetBackgroundDrawList();
        if (wallpaperTexture != 0) {
            bgList->AddImage((ImTextureID)(intptr_t)wallpaperTexture, ImVec2(0, 0), ImVec2(screenW, screenH));
        } else {
            // Blue fallback background if wallpaper.jpg is missing
            bgList->AddRectFilled(ImVec2(0, 0), ImVec2(screenW, screenH), IM_COL32(30, 80, 150, 255));
        }

        // Taskbar Rendering at Bottom of Screen
        float barHeight = 45.0f;
        ImGui::SetNextWindowPos(ImVec2(0, screenH - barHeight));
        ImGui::SetNextWindowSize(ImVec2(screenW, barHeight));

        ImGuiWindowFlags barFlags = ImGuiWindowFlags_NoTitleBar | 
                                    ImGuiWindowFlags_NoResize   | 
                                    ImGuiWindowFlags_NoMove;

        ImGui::Begin("Taskbar", nullptr, barFlags);

        // 3 Clickable Buttons on Taskbar (Start, Explorer, Task Manager)
        if (ImGui::Button("Start", ImVec2(60, 28)))        showStartMenu = !showStartMenu;
        ImGui::SameLine();
        if (ImGui::Button("Explorer", ImVec2(75, 28)))     showExplorer = !showExplorer;
        ImGui::SameLine();
        if (ImGui::Button("Task Manager", ImVec2(100, 28))) showTaskManager = !showTaskManager;

        // Real-Time Clock (Updated every frame)
        std::time_t now = std::time(nullptr);
        char timeText[64];
        std::strftime(timeText, sizeof(timeText), "%a %b %d, %Y  %I:%M:%S %p", std::localtime(&now));

        ImGui::SameLine(screenW - 320.0f);
        ImGui::Text("%s", timeText);

        // PWR (Shutdown) Button - Exits the Application
        ImGui::SameLine();
        if (ImGui::Button("PWR", ImVec2(45, 24))) {
            glfwSetWindowShouldClose(window, GLFW_TRUE); // Clean shutdown
        }

        ImGui::End();

        // UI SCREEN 1: Start Menu
        if (showStartMenu) {
            ImGui::SetNextWindowPos(ImVec2(5, screenH - barHeight - 260));
            ImGui::SetNextWindowSize(ImVec2(220, 250));

            if (ImGui::Begin("Start Menu", &showStartMenu)) {
                ImGui::Text("CSOPESY OS v1.0");
                ImGui::Separator();
                if (ImGui::Selectable("Open File Explorer")) showExplorer = true;
                if (ImGui::Selectable("Open Task Manager")) showTaskManager = true;
                ImGui::Separator();
                if (ImGui::Button("Shutdown System", ImVec2(-1, 28))) {
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                }
            }
            ImGui::End();
        }

        // UI SCREEN 2: File Explorer Window (Dummy File Structure)
        if (showExplorer) {
            ImGui::SetNextWindowSize(ImVec2(400, 250), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("File Explorer", &showExplorer)) {
                ImGui::Text("Local Disk (C:)");
                ImGui::Separator();
                ImGui::BulletText("Documents");
                ImGui::BulletText("Pictures / wallpaper.jpg");
                ImGui::BulletText("System32");
            }
            ImGui::End();
        }

        // UI SCREEN 3: Task Manager Window (Dummy Process List)
        if (showTaskManager) {
            ImGui::SetNextWindowSize(ImVec2(450, 280), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Task Manager", &showTaskManager)) {
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