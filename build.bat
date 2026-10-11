@echo off
setlocal
pushd "%~dp0"
if not defined CXX if exist "C:\Program Files (x86)\Dev-Cpp\MinGW64\bin\g++.exe" set "CXX=C:\Program Files (x86)\Dev-Cpp\MinGW64\bin\g++.exe"
if not defined CXX set "CXX=g++"
echo Compiling CSOPESY Desktop OS...
"%CXX%" -std=gnu++11 -m64 -DIMGUI_DISABLE_TIME_FUNCTIONS -DGLFW_DLL main.cpp DesktopUI.cpp imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp -I. -Iimgui -Iimgui/backends -Iglfw/include -Lglfw/lib-mingw-w64 -lglfw3dll -lopengl32 -lgdi32 -o main.exe
if errorlevel 1 (
    echo Build failed. Install a 64-bit MinGW-w64 compiler or set CXX to its g++.exe path.
    popd
    exit /b 1
)
copy /Y "glfw\lib-mingw-w64\glfw3.dll" "glfw3.dll" >nul
if errorlevel 1 (
    echo Could not copy glfw3.dll beside main.exe.
    popd
    exit /b 1
)
echo Build successful!
if /I "%~1"=="--build-only" (
    popd
    exit /b 0
)
main.exe
set "APP_EXIT=%errorlevel%"
popd
exit /b %APP_EXIT%
