#pragma once
#include "GraphicsDevice.h"

#include "UIOverlay.h"
#include "SDL.h"
#include "utils/RefCounted.h"

struct SDL_Window;

namespace mygfx {

	class Engine;
	class CameraController;

	class Application : public utils::RefCounted
	{
	public:
		Application(const char* title = "");
		virtual ~Application();
		
		void init(void* hinstance);
		void start();

		void renderLoop();

		struct Config {
		};

		static std::vector<const char*> args;
		static Application* msInstance;
	protected:
		virtual void onStart();
		virtual void onDestroy();
		void updateGUI();
		virtual void onGUI();
		virtual void onUpdate(double delta);
		virtual void onDraw(GraphicsApi& cmd);
		void windowResize(int w, int h);
		virtual void onResized(int w, int h);
		virtual void keyDown(uint32_t key);
		virtual void keyUp(uint32_t key);
		virtual void mouseDown(int button, float x, float y);
		virtual void mouseUp(int button, float x, float y);
		virtual void mouseWheel(float wheelDelta);
		virtual void mouseMove(float x, float y);

		std::string mTitle = "Demo";
		Settings settings;
		uint32_t width = 1920;
		uint32_t height = 1080;

		SDL_Window* sdlWindow;
		void* window;
		void* windowInstance;

		Ref<UIOverlay>	mUI;

		bool prepared = false;
		bool resized = false;
		double frameTimer = 0.01;
		double timer = 0.0f;
		double timerSpeed = 0.25f;
		uint32_t frameCounter = 0;
		uint32_t lastFPS = 0;
		std::chrono::time_point<std::chrono::high_resolution_clock> lastTimestamp, tPrevEnd;

		bool paused = false;
		
		Config mConfig;

		bool mShowProfiler = false;
		bool mShowSampleUI = true;
		bool mShowHierarchy = false;
		bool mShowInspector = false;

	private:
		std::string getWindowTitle();
		void setupConsole(std::string title);
		void setupDPIAwareness();
		void updateFrame();
		bool resizing = false;
	};

}

// OS specific macros for the example main entry points
#if defined(_WIN32)
#include <Windows.h>
// Windows entry point
#define PHANTOM_EXAMPLE_MAIN(TYPE)																		\
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)									\
{																									\
	for (int32_t i = 0; i < __argc; i++) { phantom::Application::args.push_back(__argv[i]); };  			\
	phantom::Application::msInstance = new TYPE();															\
	phantom::Application::msInstance->init(hInstance);																	\
	phantom::Application::msInstance->start();																		\
	delete(phantom::Application::msInstance);																			\
	return 0;																						\
}
#elif (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
#define PHANTOM_EXAMPLE_MAIN(TYPE)																	\
int main(const int argc, const char *argv[])														\
{																									\
	@autoreleasepool																				\
	{																								\
		for (size_t i = 0; i < argc; i++) { phantom::Application::args.push_back(argv[i]); };				\
		phantom::Application::msInstance = new TYPE();														\
		phantom::Application::msInstance->init(nullptr);																\
		phantom::Application::msInstance->start();														\
		delete(phantom::Application::msInstance);																		\
	}																								\
	return 0;																						\
}
#else
#define PHANTOM_EXAMPLE_MAIN(TYPE)
#endif