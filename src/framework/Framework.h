#pragma once

#include "GraphicsApi.h"

#include "render/View.h"
#include "resource/Material.h"
#include "resource/Mesh.h"
#include "resource/ModelLoader.h"
#include "resource/Shader.h"
#include "resource/Texture.h"
#include "scene/Camera.h"
#include "scene/Light.h"
#include "scene/Renderable.h"
#include "scene/Scene.h"
#include "scene/Skybox.h"

namespace mygfx {

using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
using Clock = std::chrono::high_resolution_clock;
class System;

class Framework : public Object {
public:
    Framework();
    ~Framework();

    uint32_t getWidth() const { return mWidth; }
    uint32_t getHeight() const { return mHeight; }
    HwSwapchain* getSwapChain() const { return mSwapchain; }
    auto getDeltaTime() const { return mFrameTimer; }
    double getTime() const { return std::chrono::duration<double, std::ratio<1>>(Clock::now() - mStartTime).count(); }

    bool init();
    void run();
    void windowResize(int w, int h);
    virtual void processEvent();
    virtual void updateFrame();
    void quit() { mQuit = true; }
    void destroy();

    template <typename T, typename... Args>
    T* registerSystem(Args... args)
    {
        T* sys = new T(std::forward<Args>(args)...);
        registerSystem(sys);
        return sys;
    }

    void registerSystem(System* sys);

    Ref<View> createView(uint16_t width, uint16_t height, Format format, TextureUsage usage, SampleCount msaa);
    Ref<View> createView(HwSwapchain* swapChain);
    void destroyView(const Ref<View>& view);

    inline static Framework* msInstance = nullptr;

protected:
    virtual bool createWindow(void** window, void** windowInstance);
    virtual void onResized(int w, int h);
    virtual void onStart();
    virtual void onDestroy();
    virtual void onPreUpdate(double delta);
    virtual void onUpdate(double delta);
    virtual void onPreDraw(GraphicsApi& cmd);
    virtual void onDraw(GraphicsApi& cmd);
    virtual void onPostDraw(GraphicsApi& cmd);

    Vector<String> mArgs;
    String mTitle;
    String mCPUName;
    Settings mSettings;
    uint32_t mWidth = 1920;
    uint32_t mHeight = 1080;
    Vector<Ref<System>> mSystems;
    bool mInitialized = false;
    std::unique_ptr<GraphicsApi> mGraphicsApi;
    Ref<HwSwapchain> mSwapchain;
    HashSet<Ref<View>> mViews;
    bool mPrepared = false;
    bool mQuit = false;
    double mFrameTimer = 0.01;
    uint32_t mFrameCounter = 0;
    uint32_t mLastFPS = 0;
    TimePoint mStartTime;
    TimePoint mLastTimestamp, mTimePrevEnd;
};

}