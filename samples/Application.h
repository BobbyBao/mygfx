#pragma once
#include "GraphicsApi.h"
#include "UIOverlay.h"

namespace mygfx {

class Engine;
class CameraController;

class Application {
public:
    Application();
    virtual ~Application();

    uint32_t getWidth() const { return mWidth; }
    uint32_t getHeight() const { return mHeight; }
	
	bool init();

    void handleEvent(const SDL_Event& e);
protected:
    virtual void onInit();
    virtual void onStart();
    virtual void onDestroy();
    virtual void processEvent();
    virtual void onPreUpdate(double delta);
    virtual void onUpdate(double delta);
    virtual void onPreDraw(GraphicsApi& cmd);
    virtual void onDraw(GraphicsApi& cmd);
    virtual void onPostDraw(GraphicsApi& cmd);
    
    virtual void onGUI();
    virtual void onResized(int w, int h);
    virtual void keyDown(uint32_t key);
    virtual void keyUp(uint32_t key);
    virtual void mouseDown(int button, float x, float y);
    virtual void mouseUp(int button, float x, float y);
    virtual void mouseWheel(float wheelDelta);
    virtual void mouseMove(float x, float y);
	
    inline static Application* msInstance = nullptr;
protected:
    void setupConsole(std::string title);
    void setupDPIAwareness();
    
    Vector<String> mArgs;
    String mTitle = "Phantom Player";
	Settings mSettings;
	uint32_t mWidth = 1920;
	uint32_t mHeight = 1080;
    SDL_Window* mSdlWindow;
	void* mWindow;
	void* mHInstance;    
    std::unique_ptr<GraphicsApi> mGraphicsApi;
    Ref<HwSwapchain> mSwapchain;
	bool mPrepared = false;
    bool mQuit = false;
    double mFrameTimer = 0.01;
    uint32_t mFrameCounter = 0;
    uint32_t mLastFPS = 0;
    TimePoint mStartTime;
    TimePoint mLastTimestamp, mTimePrevEnd;
    Ref<UIOverlay> mUI;
    bool mShowProfiler = false;
    bool mShowGUI = true;
};







}
