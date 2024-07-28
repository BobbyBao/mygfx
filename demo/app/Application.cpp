#include "Application.h"

#ifdef _WIN32

#ifdef FAR
#undef FAR
#endif

#include <ShellScalingAPI.h>
#endif

#include "imgui/ImGui.h"

namespace mygfx {

	std::vector<const char*> Application::args;
	Application* Application::msInstance = nullptr;

	void* getNativeWindow(SDL_Window* sdlWindow)
	{
#if defined(WIN32) && !defined(__WINRT__)
		return (HWND)SDL_GetProperty(SDL_GetWindowProperties(sdlWindow), "SDL.window.win32.hwnd", nullptr);
#elif defined(__APPLE__) && defined(SDL_VIDEO_DRIVER_COCOA)
		return (void*)SDL_GetProperty(SDL_GetWindowProperties(window), "SDL.window.cocoa.window", nullptr);
#endif
		return nullptr;
	}

	Application::Application(const char* title) : mTitle(title)
	{
		SDL_Init(SDL_INIT_VIDEO);

		// Validation for all samples can be forced at compile time using the FORCE_VALIDATION define
#if defined(FORCE_VALIDATION)
#ifndef NDEBUG
		settings.validation = true;
#endif
#endif

#if defined(_WIN32)
		setupConsole(title);
		setupDPIAwareness();
#endif

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
		typedef HRESULT* (__stdcall* SetProcessDpiAwarenessFunc)(PROCESS_DPI_AWARENESS);

		HMODULE shCore = LoadLibraryA("Shcore.dll");
		if (shCore)
		{
			SetProcessDpiAwarenessFunc setProcessDpiAwareness =
				(SetProcessDpiAwarenessFunc)GetProcAddress(shCore, "SetProcessDpiAwareness");

			if (setProcessDpiAwareness != nullptr)
			{
				setProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
			}

			FreeLibrary(shCore);
		}
	}

#endif

	std::string Application::getWindowTitle()
	{
		std::string windowTitle;
		windowTitle = mTitle;
		windowTitle += " - " + std::to_string(frameCounter) + " fps";
		return windowTitle;
	}

	void Application::init(void* hinstance)
	{
		settings.name = mTitle.c_str();
		settings.width = width;
		settings.height = height;

		windowInstance = hinstance;
		sdlWindow = SDL_CreateWindow(mTitle.c_str(), width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
		window = getNativeWindow(sdlWindow);		
		mUI = new UIOverlay(sdlWindow);
	}

	void Application::start()
	{
		

		onStart();

		prepared = true;

		renderLoop();
	}


	void Application::renderLoop()
	{
		lastTimestamp = std::chrono::high_resolution_clock::now();
		tPrevEnd = lastTimestamp;

		bool close = false;
		while (!close) {
			SDL_Event event;
			while (SDL_PollEvent(&event)) {

				mUI->handleEvent(event);

				switch (event.type) {
				case SDL_EVENT_QUIT:
					// handling of close button
					close = true;
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

			updateFrame();
		}

		onDestroy();


	}

	void Application::updateFrame()
	{
		auto tStart = std::chrono::high_resolution_clock::now();

		frameCounter++;

		auto tEnd = std::chrono::high_resolution_clock::now();
		auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();

		frameTimer = tDiff / 1000.0f;

		// Convert to clamped timer value
		if (!paused)
		{
			timer += timerSpeed * frameTimer;
			if (timer > 1.0)
			{
				timer -= 1.0f;
			}
		}

		float fpsTimer = (float)(std::chrono::duration<double, std::milli>(tEnd - lastTimestamp).count());
		if (fpsTimer > 1000.0f)
		{
			lastFPS = static_cast<uint32_t>((float)frameCounter * (1000.0f / fpsTimer));
			frameCounter = 0;
			lastTimestamp = tEnd;
		}

		tPrevEnd = tEnd;

	}

	void  Application::onStart()
	{
	}

	void Application::onDestroy()
	{
	}

	void Application::updateGUI()
	{
		mUI->update();

		if (mShowProfiler) {

		}
		
		if (mShowSampleUI) {
			onGUI();
		}

	}

	void Application::onGUI()
	{
		//ImGui::ShowDemoWindow();
	}

	void Application::onUpdate(double delta)
	{
	}

	void Application::onDraw(GraphicsApi& cmd)
	{
	}

	void Application::keyDown(uint32_t key)
	{
		if (key == SDLK_F1) {
			mShowSampleUI = !mShowSampleUI;
		}

		if (key == SDLK_F2) {
			mShowProfiler = !mShowProfiler;
		}

	}

	void Application::windowResize(int w, int h)
	{
		if (!prepared)
		{
			return;
		}

		prepared = false;
		resized = true;

		width = w;
		height = h;

		mUI->resize(width, height);

		onResized(w, h);

		prepared = true;
	}
		
	void Application::onResized(int w, int h) {

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