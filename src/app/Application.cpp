#include "Application.h"
#include "core/FileSystem.h"
#include "resource/Material.h"
#include "resource/Texture.h"
#include "vulkan/VulkanDevice.h"

#ifdef _WIN32

#ifdef FAR
#undef FAR
#endif

#include <ShellScalingAPI.h>
#endif

#include "imgui/ImGui.h"
#include <filesystem>

#include <SDL3/SDL_vulkan.h>

namespace mygfx {

void* getNativeWindow(SDL_Window* sdlWindow)
{
#if defined(WIN32) && !defined(__WINRT__)
    return (HWND)SDL_GetProperty(SDL_GetWindowProperties(sdlWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#else // /*defined(__APPLE__) && */defined(SDL_VIDEO_DRIVER_COCOA)
    return (void*)SDL_GetProperty(SDL_GetWindowProperties(sdlWindow), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
#endif
    return nullptr;
}

Application::Application(int argc, char** argv)
{
    for (int i = 0; i < argc; i++) {
        mArgs.push_back(argv[i]);
    }

    SDL_Init(SDL_INIT_VIDEO);

#ifndef NDEBUG
    mSettings.validation = true;
#endif

#if defined(_WIN32)
    setupConsole(mTitle);
    setupDPIAwareness();
#endif
    FileSystem::setBasePath("../../media");
}

Application::~Application()
{
    SDL_Quit();
}

#if defined(_WIN32)
// Win32 : Sets up a console window and redirects standard output to it
void Application::setupConsole(std::string title)
{
    AllocConsole();
    AttachConsole(GetCurrentProcessId());
    FILE* stream;
    freopen_s(&stream, "CONIN$", "r", stdin);
    freopen_s(&stream, "CONOUT$", "w+", stdout);
    freopen_s(&stream, "CONOUT$", "w+", stderr);
    // Enable flags so we can color the output
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(consoleHandle, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(consoleHandle, dwMode);
    SetConsoleTitle(TEXT(title.c_str()));
}

void Application::setupDPIAwareness()
{
    typedef HRESULT*(__stdcall * SetProcessDpiAwarenessFunc)(PROCESS_DPI_AWARENESS);

    HMODULE shCore = LoadLibraryA("Shcore.dll");
    if (shCore) {
        SetProcessDpiAwarenessFunc setProcessDpiAwareness = (SetProcessDpiAwarenessFunc)GetProcAddress(shCore, "SetProcessDpiAwareness");

        if (setProcessDpiAwareness != nullptr) {
            setProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
        }

        FreeLibrary(shCore);
    }
}

#endif

void getCPUDescription(std::string& cpuName)
{
#if defined(_WINDOWS)
    int32_t nIDs = 0;
    int32_t nExIDs = 0;

    char strCPUName[0x40] = {};

    std::array<int, 4> cpuInfo;
    std::vector<std::array<int, 4>> extData;

    __cpuid(cpuInfo.data(), 0);

    // Calling __cpuid with 0x80000000 as the function_id argument
    // gets the number of the highest valid extended ID.
    __cpuid(cpuInfo.data(), 0x80000000);

    nExIDs = cpuInfo[0];
    for (int i = 0x80000000; i <= nExIDs; ++i) {
        __cpuidex(cpuInfo.data(), i, 0);
        extData.push_back(cpuInfo);
    }

    // Interpret CPU strCPUName string if reported
    if (nExIDs >= 0x80000004) {
        memcpy(strCPUName, extData[2].data(), sizeof(cpuInfo));
        memcpy(strCPUName + 16, extData[3].data(), sizeof(cpuInfo));
        memcpy(strCPUName + 32, extData[4].data(), sizeof(cpuInfo));
    }

    cpuName = strCPUName;
#else
#pragma message("Please add code to fetch CPU name for this platform")
    cpuName = "Unavailable";
#endif // _WINDOWS
}

bool Application::createWindow(void** window, void** windowInstance)
{
    getCPUDescription(mCPUName);

    mSdlWindow = SDL_CreateWindow(mTitle.c_str(), mWidth, mHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
    if (!mSdlWindow) {
        return false;
    }
    
    *window = getNativeWindow(mSdlWindow);
    *windowInstance = nullptr;
#if defined(WIN32)
    *windowInstance = SDL_GetProperty(SDL_GetWindowProperties(mSdlWindow), SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER, nullptr);
#endif

    return true;
}

void Application::onInit()
{
}

void Application::onStart()
{
    VkSurfaceKHR surface = nullptr;
    if (!SDL_Vulkan_CreateSurface(mSdlWindow, mDevice->instance, nullptr, &surface)) {
        return;
    }

    SwapChainDesc desc {
        .width = mWidth,
        .height = mHeight,
        .windowInstance = mHInstance,
        .window = mWindow,
        .surface = surface,
    };

    mSwapchain = mGraphicsApi->createSwapchain(desc);

    mUI = registerSystem<UIOverlay>(mSdlWindow);
}

void Application::onDestroy()
{
    mUI.reset();
}

void Application::processEvent()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        handleEvent(event);
    }
}

void Application::handleEvent(const SDL_Event& event)
{
    mUI->handleEvent(event);

    switch (event.type) {
    case SDL_EVENT_QUIT:
        // handling of close button
        mQuit = true;
        break;
    case SDL_EVENT_WINDOW_RESIZED:
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        windowResize(event.window.data1, event.window.data2);
        break;
    case SDL_EVENT_KEY_DOWN:
        keyDown(event.key.keysym.sym);
        break;
    case SDL_EVENT_KEY_UP:
        keyUp(event.key.keysym.sym);
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        mouseDown(event.button.button, event.button.x, event.button.y);
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        mouseUp(event.button.button, event.button.x, event.button.y);
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        mouseWheel(event.wheel.y);
        break;
    case SDL_EVENT_MOUSE_MOTION:
        mouseMove(event.motion.x, event.motion.y);
        break;
    }
}

void Application::onPreUpdate(double delta)
{
    mUI->update();

    if (mShowProfiler) {
    }

    if (mShowGUI) {
        onGUI();
    }
}

void Application::onPostDraw(GraphicsApi& cmd)
{
    mUI->draw(cmd);
}

void Application::onGUI()
{
}

void Application::keyDown(uint32_t key)
{
    if (key == SDLK_F1) {
        mShowGUI = !mShowGUI;
    }

    if (key == SDLK_F2) {
        mShowProfiler = !mShowProfiler;
    }
}

void Application::onResized(int w, int h)
{
    mUI->resize(w, h);
}

void Application::keyUp(uint32_t key)
{
}

void Application::mouseDown(int button, float x, float y)
{
}

void Application::mouseUp(int button, float x, float y)
{
}

void Application::mouseMove(float x, float y)
{
}

void Application::mouseWheel(float wheelDelta)
{
}

}