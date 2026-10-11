#include <ctime>
#include <string>

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

        // Render Desktop Background, Taskbar, and Start Menu
        bool shutdownRequested = desktopUI.Render(
            (ImTextureID)(intptr_t)wallpaperTexture,
            showExplorer,
            showTaskManager,
            showAboutWindow,
            showCalendarWindow
        );

        if (shutdownRequested) {
            glfwSetWindowShouldClose(window, GLFW_TRUE); // Clean shutdown
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