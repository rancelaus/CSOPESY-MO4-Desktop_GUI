@echo off
echo Compiling CSOPESY Desktop OS...
g++ -std=c++17 main.cpp imgui/*.cpp imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp -I. -Iimgui -Iimgui/backends -Iglfw/include -Lglfw/lib-mingw-w64 -lglfw3 -lopengl32 -lgdi32 -o main.exe
if %errorlevel% equ 0 (
    echo Build successful! Running main.exe...
    main.exe
) else (
    echo Build failed!
)