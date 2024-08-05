#pragma once
#include "GraphicsHandles.h"
#include "GraphicsApi.h"
#include "UIOverlay.h"
#include "SDL.h"
#include "utils/RefCounted.h"

#include <concurrencpp/concurrencpp.h>

struct SDL_Window;
union SDL_Event;

namespace mygfx {

	class Engine;
	class CameraController;
	using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
	using Clock = std::chrono::high_resolution_clock;

	using namespace concurrencpp;

	template<typename T>
	using Result = concurrencpp::result<T>;

	class Application : public utils::RefCounted, public concurrencpp::runtime
	{
	public:
		Application(int argc = 0, char** argv = nullptr);
		virtual ~Application();

		void start();

		bool init();
		void updateFrame();
		void handleEvent(const SDL_Event& e);
		void quit() { mQuit = true; }
		void destroy();

		uint32_t getWidth() const { return mWidth; }
		uint32_t getHeight() const { return mHeight; }
		HwSwapchain* getSwapChain() const { return mSwapchain; }

		auto getDeltaTime() const {
			return mFrameTimer;
		}

		double getTime() const {
			return std::chrono::duration<double, std::ratio<1>>(Clock::now() - mStartTime).count();
		}

		const std::shared_ptr<concurrencpp::manual_executor>& getMainExecutor() const { return mMainExecutor; }

		static Application* msInstance;
	protected:
		virtual Result<void> onStart();	
		virtual void onDestroy();
		void updateGUI();
		virtual void onGUI();
		virtual void onUpdate(double delta);
		virtual void onPreDraw(GraphicsApi& cmd);
		virtual void onDraw(GraphicsApi& cmd);
		void windowResize(int w, int h);
		virtual void onResized(int w, int h);
		virtual void keyDown(uint32_t key);
		virtual void keyUp(uint32_t key);
		virtual void mouseDown(int button, float x, float y);
		virtual void mouseUp(int button, float x, float y);
		virtual void mouseWheel(float wheelDelta);
		virtual void mouseMove(float x, float y);
	protected:
		void setupConsole(std::string title);
		void setupDPIAwareness();
		void mainLoop();

		std::vector<String> mArgs;
		std::string mTitle;
		std::string mCPUName;
		Settings mSettings;
		uint32_t mWidth = 1920;
		uint32_t mHeight = 1080;

		SDL_Window* mSdlWindow;

		std::unique_ptr<GraphicsApi> mGraphicsApi;
		Ref<HwSwapchain> mSwapchain;
		Ref<UIOverlay>	mUI;

		bool mPrepared = false;
		bool mQuit = false;
		double mFrameTimer = 0.01;
		uint32_t mFrameCounter = 0;
		uint32_t mLastFPS = 0;
		TimePoint mStartTime;
		TimePoint mLastTimestamp, mTimePrevEnd;
		bool mShowProfiler = false;
		bool mShowGUI = true;
		std::shared_ptr<concurrencpp::manual_executor> mMainExecutor;
	};

}
