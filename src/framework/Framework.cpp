#include "Framework.h"
#include "core/System.h"
#include "resource/ShaderCompiler.h"
#include "vulkan/VulkanDevice.h"

namespace mygfx {

Framework::Framework()
    : mTitle("mygfx")
{
    msInstance = this;

    registerSystem<ShaderCompiler>();
}

Framework::~Framework()
{
    msInstance = nullptr;
}

void Framework::registerSystem(System* sys)
{
    mSystems.emplace_back(sys);

    if (mInitialized) {
        sys->init();
    }
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

bool Framework::init()
{
    if (!createWindow(&mWindow, &mHInstance)) {
        return false;
    }

    mSettings.name = mTitle.c_str();

    mDevice = new VulkanDevice();
    if (!mDevice->create(mSettings)) {
        return false;
    }

    mGraphicsApi = std::make_unique<GraphicsApi>(*mDevice);

    Texture::staticInit();

    for (auto& sys : mSystems) {
        sys->init();
    }

    onInit();

    mInitialized = true;

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

void Framework::onResized(int w, int h)
{
}

void Framework::onInit()
{
}

void Framework::onStart()
{
    SwapChainDesc desc {
        .width = mWidth,
        .height = mHeight,
        .windowInstance = mHInstance,
        .window = mWindow,
    };

    mSwapchain = mGraphicsApi->createSwapchain(desc);
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

    for (auto it = mSystems.rbegin(); it != mSystems.rend(); ++it) {
        (*it)->shutdown();
    }

    mSystems.clear();

    Texture::staticDeinit();

    mGraphicsApi.reset();
}


}
