#pragma once
#include "GraphicsHandles.h"
#include "GraphicsApi.h"
#include "api/CommandBufferQueue.h"
#include "UIOverlay.h"
#include "SDL.h"
#include "utils/RefCounted.h"

struct SDL_Window;
union SDL_Event;

namespace mygfx {

	class Engine;
	class CameraController;

	class Application : public utils::RefCounted
	{
	public:
		Application();
		virtual ~Application();

		void start();

		bool init();
		void updateFrame();
		void handleEvent(const SDL_Event& e);
		void quit() { mQuit = true; }
		void destroy();

		GraphicsApi& getGraphicsApi() noexcept {
			return *mGraphicsApi;
		}

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
	private:
		void setupConsole(std::string title);
		void setupDPIAwareness();
		void mainLoop();
		void renderLoop();

		std::string mTitle;
		Settings mSettings;
		uint32_t mWidth = 1920;
		uint32_t mHeight = 1080;

		SDL_Window* mSdlWindow;

		std::unique_ptr<GraphicsDevice> mDevice;
		CommandBufferQueue mCommandBufferQueue;
		std::unique_ptr<GraphicsApi> mGraphicsApi;
		Ref<HwSwapchain> mSwapchain;
		Ref<UIOverlay>	mUI;

		std::unique_ptr<std::thread> mRenderThread;
		bool mRendering = false;
		bool mPrepared = false;
		bool mQuit = false;
		double mFrameTimer = 0.01;
		uint32_t mFrameCounter = 0;
		uint32_t mLastFPS = 0;
		std::chrono::time_point<std::chrono::high_resolution_clock> mLastTimestamp, mTimePrevEnd;
		bool mShowProfiler = false;
		bool mShowGUI = true;
	};

}
