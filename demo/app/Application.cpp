#include "Application.h"
#include "vulkan/VulkanDevice.h"
#include "utils/FileUtils.h"
#include "Texture.h"

#ifdef _WIN32

#ifdef FAR
#undef FAR
#endif

#include <ShellScalingAPI.h>
#endif

#include "imgui/ImGui.h"

namespace mygfx {

	Application* Application::msInstance = nullptr;

	void* getNativeWindow(SDL_Window* sdlWindow) {
#if defined(WIN32) && !defined(__WINRT__)
		return (HWND)SDL_GetProperty(SDL_GetWindowProperties(sdlWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#elif defined(__APPLE__) && defined(SDL_VIDEO_DRIVER_COCOA)
		return (void*)SDL_GetProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
#endif
		return nullptr;
	}

	std::string readAllText(const std::string& filePath) {
		auto file = SDL_IOFromFile(filePath.c_str(), "rb"); if (!file) {
			return {};
		}
		auto fileSize = SDL_GetIOSize(file);
		std::string str;
		str.resize(fileSize);
		SDL_ReadIO(file, str.data(), fileSize);
		SDL_CloseIO(file);
		return str;
	}

	std::vector<uint8_t> readAll(const std::string& filePath) {
		auto file = SDL_IOFromFile(filePath.c_str(), "rb");
		if (!file) {
			return {};
		}
		auto fileSize = SDL_GetIOSize(file);
		std::vector<uint8_t> bytes;
		bytes.resize(fileSize);
		SDL_ReadIO(file, bytes.data(), fileSize);
		SDL_CloseIO(file);
		return bytes;
	}

	Application::Application(int argc, char** argv) : mTitle("mygfx")
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
		FileUtils::readFileFn = readAll;
		FileUtils::readTextFn = readAllText;
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

	bool Application::init()
	{
		mSettings.name = mTitle.c_str();
		mSettings.width = mWidth;
		mSettings.height = mHeight;

		void* window;
		void* windowInstance;

		mSdlWindow = SDL_CreateWindow(mTitle.c_str(), mWidth, mHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
		if (!mSdlWindow) {
			return false;
		}

		window = getNativeWindow(mSdlWindow);

#if defined(WIN32)
		windowInstance = SDL_GetProperty(SDL_GetWindowProperties(mSdlWindow), SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER, nullptr);
#endif
		auto mDevice = new VulkanDevice();
		if (!mDevice->init(mSettings)) {
			return false;
		}

		mGraphicsApi = std::make_unique<GraphicsApi>(*mDevice);

		mDevice->create(windowInstance, window);
		auto& gfx = gfxApi();

		Texture::staticInit();

		mSwapchain = mDevice->getSwapChain();

		mUI = new UIOverlay(mSdlWindow);
		mUI->init();

		onStart();

		mStartTime = Clock::now();
		mLastTimestamp = mStartTime;
		mTimePrevEnd = mLastTimestamp;
		mPrepared = true;
		return true;
	}

	void Application::start()
	{
		init();

		mainLoop();
	}


	void Application::mainLoop()
	{
		while (!mQuit) {
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				handleEvent(event);
			}

			updateFrame();
		}

		destroy();

	}

	void Application::handleEvent(const SDL_Event& event) {

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

	void Application::updateFrame()
	{
		auto tStart = std::chrono::high_resolution_clock::now();
		updateGUI();
		
		onUpdate(mFrameTimer);

		auto& cmd = gfxApi();

		cmd.beginFrame();

		onPreDraw(cmd);

		cmd.prepareFrame();

		RenderPassInfo renderInfo { 
			.clearFlags = TargetBufferFlags::ALL,
			.clearColor = {0.25f, 0.25f, 0.25f, 1.0f}
		};

		renderInfo.viewport = { .left = 0, .top = 0, .width = mWidth, .height = mHeight };

		cmd.beginRendering(mSwapchain->renderTarget, renderInfo);

		onDraw(cmd);
		
		mUI->draw(cmd);

		cmd.endRendering(mSwapchain->renderTarget);

		cmd.submitFrame();
		cmd.endFrame();
		
		cmd.flush();


		mFrameCounter++;

		auto tEnd = Clock::now();
		auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();

		mFrameTimer = tDiff / 1000.0f;

		float fpsTimer = (float)(std::chrono::duration<double, std::milli>(tEnd - mLastTimestamp).count());
		if (fpsTimer > 1000.0f)
		{
			mLastFPS = static_cast<uint32_t>((float)mFrameCounter * (1000.0f / fpsTimer));
			mFrameCounter = 0;
			mLastTimestamp = tEnd;
		}

		mTimePrevEnd = tEnd;

	}


	void Application::destroy() {
		
		onDestroy();

		Texture::staticDeinit();

		mGraphicsApi.reset();

	}

	void Application::updateGUI()
	{
		mUI->update();

		if (mShowProfiler) {

		}

		if (mShowGUI) {
			onGUI();
		}

	}

	void  Application::onStart()
	{
	}

	void Application::onDestroy()
	{
	}

	void Application::onGUI()
	{
	}

	void Application::onUpdate(double delta)
	{
	}

	void Application::onPreDraw(GraphicsApi& cmd) {
	}

	void Application::onDraw(GraphicsApi& cmd) {
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

	void Application::windowResize(int w, int h)
	{
		if (!mPrepared)
		{
			return;
		}

		mPrepared = false;

		gfxApi().reload(w, h);

		mWidth = w;
		mHeight = h;

		mUI->resize(mWidth, mHeight);

		onResized(w, h);

		mPrepared = true;
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