#include "GraphicsDevice.h"
#include "FrameListener.h"
#include "api/Dispatcher.h"
#include "utils/Log.h"

namespace mygfx {

GraphicsDevice* gInstance;

GraphicsDevice::GraphicsDevice()
{
    gInstance = this;
}

GraphicsDevice::~GraphicsDevice()
{
    gInstance = nullptr;
}

Dispatcher GraphicsDevice::getDispatcher() const noexcept
{
    return {};
}

void GraphicsDevice::execute(std::function<void(void)> const& fn) noexcept
{
    fn();
}

void GraphicsDevice::swapContext()
{
    SyncContext::swapContext();

    FrameChangeListener::callFrameChange();

    Stats::clear();

    ++frameNum;
}

void GraphicsDevice::beginRender()
{
    mainSemWait();
    mLastRenderTime = Clock::now();
}

void GraphicsDevice::endRender()
{
    for (auto it = mPostCall.begin(); it != mPostCall.end();) {
        auto& timer = std::get<1>(*it);
        if (--timer == 0) {
            auto& fn = std::get<0>(*it);
            fn();
            it = mPostCall.erase(it);
        } else {
            ++it;
        }
    }

    Stats::renderTime() = std::chrono::duration<double, std::milli>(Clock::now() - mLastRenderTime).count();
    renderSemPost();
}

void GraphicsDevice::post(const std::function<void()>& fn, int delay)
{
    mPostCall.push_back(std::make_tuple(fn, delay));
}

void GraphicsDevice::executeAll()
{
    for (auto& t : mPostCall) {
        auto& fn = std::get<0>(t);
        fn();
    }

    mPostCall.clear();
}

std::vector<RenderCommand>& HwRenderQueue::getWriteCommands()
{
    return mRenderables[gInstance->workContext()];
}

const std::vector<RenderCommand>& HwRenderQueue::getReadCommands() const
{
    return mRenderables[gInstance->renderFrame()];
}

void HwRenderQueue::clear()
{
    mRenderables[gInstance->workContext()].clear();
}

uint32_t Stats::getDrawCall()
{
    return sDrawCall[gInstance->workContext()];
}

uint32_t Stats::getTriCount()
{
    return sTriCount[gInstance->workContext()];
}

double Stats::getRenderTime()
{
    return sRenderTime[gInstance->workContext()];
}

void Stats::clear()
{
    drawCall() = 0;
    triCount() = 0;
    renderTime() = 0;
}
}