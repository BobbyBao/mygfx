#include "Framework.h"
#include "vulkan/VulkanDevice.h"

namespace mygfx {

Framework::Framework()
    : mTitle("mygfx")
{
    msInstance = this;
}

Framework::~Framework()
{
    msInstance = nullptr;
}

bool Framework::init()
{
    void* window = nullptr;
    void* windowInstance = nullptr;
    if (!createWindow(&window, &windowInstance)) {
        return false;
    }

    mSettings.name = mTitle.c_str();

    auto mDevice = new VulkanDevice();
    if (!mDevice->create(mSettings)) {
        return false;
    }

    mGraphicsApi = std::make_unique<GraphicsApi>(*mDevice);

    SwapChainDesc desc {
        .windowInstance = windowInstance,
        .window = window,
        .width = mWidth,
        .height = mHeight,
    };

    mSwapchain = mGraphicsApi->createSwapchain(desc);

    Texture::staticInit();

    onStart();

    mStartTime = Clock::now();
    mLastTimestamp = mStartTime;
    mTimePrevEnd = mLastTimestamp;
    mPrepared = true;
    return true;
}

bool Framework::createWindow(void** window, void** windowInstance)
{
    return false;
}

Ref<View> Framework::createView(uint16_t width, uint16_t height, Format format, TextureUsage usage, SampleCount msaa)
{
    return mViews.emplace(new View(width, height, format, usage, msaa)).first.operator*();
}

Ref<View> Framework::createView(HwSwapchain* swapChain)
{
    return mViews.emplace(new View(swapChain)).first.operator*();
}

void Framework::destroyView(const Ref<View>& view)
{
    mViews.erase(view);
}

void Framework::run()
{
    init();

    while (!mQuit) {
        processEvent();
        updateFrame();
    }

    destroy();
}

void Framework::windowResize(int w, int h)
{
    if (!mPrepared) {
        return;
    }

    gfxApi().resize(mSwapchain, w, h);

    mWidth = w;
    mHeight = h;

    onResized(w, h);
}

void Framework::processEvent()
{
}

void Framework::updateFrame()
{
    auto tStart = std::chrono::high_resolution_clock::now();
    onPreUpdate(mFrameTimer);

    onUpdate(mFrameTimer);

    for (auto& view : mViews) {
        view->update(mFrameTimer);
    }

    Material::updateAll();

    auto& cmd = gfxApi();

    cmd.beginFrame();

    onPreDraw(cmd);

    cmd.makeCurrent(mSwapchain);

    RenderPassInfo renderInfo {
        .clearFlags = TargetBufferFlags::ALL,
        .clearColor = { 0.15f, 0.15f, 0.15f, 1.0f }
    };

    renderInfo.viewport = { .left = 0, .top = 0, .width = mWidth, .height = mHeight };

    cmd.beginRendering(mSwapchain->renderTarget, renderInfo);

    onDraw(cmd);

    for (auto& view : mViews) {
        view->render(cmd);
    }

    onPostDraw(cmd);

    cmd.endRendering(mSwapchain->renderTarget);

    cmd.commit(mSwapchain);

    cmd.endFrame();

    cmd.flush();

    mFrameCounter++;

    auto tEnd = Clock::now();
    auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();

    mFrameTimer = tDiff / 1000.0f;

    float fpsTimer = (float)(std::chrono::duration<double, std::milli>(tEnd - mLastTimestamp).count());
    if (fpsTimer > 1000.0f) {
        mLastFPS = static_cast<uint32_t>((float)mFrameCounter * (1000.0f / fpsTimer));
        mFrameCounter = 0;
        mLastTimestamp = tEnd;
    }

    mTimePrevEnd = tEnd;
}

void Framework::destroy()
{
    onDestroy();
    
    mViews.clear();

    Texture::staticDeinit();

    mGraphicsApi.reset();
}

void Framework::onResized(int w, int h)
{
}

void Framework::onStart()
{
}

void Framework::onDestroy()
{
}

void Framework::onPreUpdate(double delta)
{
}

void Framework::onUpdate(double delta)
{
}

void Framework::onPreDraw(GraphicsApi& cmd)
{
}

void Framework::onDraw(GraphicsApi& cmd)
{
}

void Framework::onPostDraw(GraphicsApi& cmd)
{
}

}
