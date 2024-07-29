#include "Application.h"
#include "vulkan/VulkanDevice.h"
#include "utils/FileUtils.h"

#ifdef _WIN32

#ifdef FAR
#undef FAR
#endif

#include <ShellScalingAPI.h>
#endif

#include "imgui/ImGui.h"

#ifndef MIN_COMMAND_BUFFERS_SIZE_IN_MB
#    define MIN_COMMAND_BUFFERS_SIZE_IN_MB 2
#endif

#ifndef COMMAND_BUFFER_SIZE_IN_MB
#    define COMMAND_BUFFER_SIZE_IN_MB (MIN_COMMAND_BUFFERS_SIZE_IN_MB * 3)
#endif

namespace mygfx {

	std::vector<const char*> Application::args;
	Application* Application::msInstance = nullptr;

	void* getNativeWindow(SDL_Window* sdlWindow)
	{
		
#if defined(WIN32) && !defined(__WINRT__)
		return (HWND)SDL_GetProperty(SDL_GetWindowProperties(sdlWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#elif defined(__APPLE__) && defined(SDL_VIDEO_DRIVER_COCOA)
		return (void*)SDL_GetProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
#endif
		return nullptr;
	}

	std::string readAllText(const std::string& filePath) {
		auto file = SDL_IOFromFile(filePath.c_str(), "rb");
		auto fileSize = SDL_GetIOSize(file);
		std::string str;
		str.resize(fileSize);
		SDL_ReadIO(file, str.data(), fileSize);
		SDL_CloseIO(file);
		return str;
	}

	Application::Application() : mTitle("mygfx"),
		mCommandBufferQueue(MIN_COMMAND_BUFFERS_SIZE_IN_MB * 1024 * 1024, COMMAND_BUFFER_SIZE_IN_MB * 1024 * 1024)
	{
		SDL_Init(SDL_INIT_VIDEO);

#ifndef NDEBUG
		mSettings.validation = true;
#endif

#if defined(_WIN32)
		setupConsole(mTitle);
		setupDPIAwareness();
#endif
		FileUtils::readFileFn = readAllText;
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

	void Application::init()
	{
		mSettings.name = mTitle.c_str();
		mSettings.width = mWidth;
		mSettings.height = mHeight;

		void* window;
		void* windowInstance;

		mSdlWindow = SDL_CreateWindow(mTitle.c_str(), mWidth, mHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
		window = getNativeWindow(mSdlWindow);

#if defined(WIN32)
		windowInstance = SDL_GetProperty(SDL_GetWindowProperties(mSdlWindow), SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER, nullptr);
#endif
		mDevice = std::make_unique<VulkanDevice>();
		mDevice->init(mSettings);
		mGraphicsApi = std::make_unique<GraphicsApi>(*mDevice, mCommandBufferQueue.getCircularBuffer());

		Texture::staticInit();

		mDevice->create(windowInstance, window);
		mSwapchain = mDevice->getSwapChain();

		mUI = new UIOverlay(mSdlWindow);

		onStart();

		mPrepared = true;

		mLastTimestamp = std::chrono::high_resolution_clock::now();
		mTimePrevEnd = mLastTimestamp;
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

		auto& cmd = getGraphicsApi();

		cmd.beginFrame();
		cmd.prepareFrame();

		RenderPassInfo renderInfo { 
			.clearFlags = TargetBufferFlags::ALL,
			.clearColor = {0.25f, 0.25f, 0.25f, 1.0f}
		};

		renderInfo.viewport = { .left = 0, .top = 0, .width = mWidth, .height = mHeight };

		cmd.beginRendering(mSwapchain->renderTarget, renderInfo);

		onDraw(cmd);

		cmd.endRendering(mSwapchain->renderTarget);

		cmd.submitFrame();
		cmd.endFrame();

		auto& gfx = device();
		if (gfx.singleLoop()) {
			gfx.swapContext();
		}
		else {

			gfx.waitRender();

			mCommandBufferQueue.flush();

			gfx.swapContext();

			if (mRenderThread == nullptr) {
				mRendering = true;
				mRenderThread = std::make_unique<std::thread>(&Application::renderLoop, this);
			}

			gfx.mainSemPost();
		}


		mFrameCounter++;

		auto tEnd = std::chrono::high_resolution_clock::now();
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

	void Application::renderLoop()
	{
		GraphicsDevice::renderThreadID = std::this_thread::get_id();

		while (mRendering) {
			auto buffers = mCommandBufferQueue.waitForCommands();
			if (UTILS_UNLIKELY(buffers.empty())) {
				continue;
			}

			device().beginRender();
			// execute all command buffers
			auto& driver = getGraphicsApi();
			for (auto& item : buffers) {
				if (UTILS_LIKELY(item.begin)) {
					driver.execute(item.begin);
					mCommandBufferQueue.releaseBuffer(item);
				}
			}
			device().endRender();
		}
	}

	void Application::destroy() {
		
		onDestroy();

		mRendering = false;

		device().mainSemPost();

		device().waitRender();

		// now wait for all pending commands to be executed and the thread to exit
		mCommandBufferQueue.requestExit();

		if (mRenderThread != nullptr)
			mRenderThread->join();
		
		Texture::staticDeinit();

		mDevice.release();
		mGraphicsApi.release();
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

		auto& cmd = getGraphicsApi();
		cmd.reload(w, h);

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