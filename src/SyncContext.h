#pragma once

#include "GraphicsConsts.h"
#include <functional>
#include <semaphore>
#include <thread>

namespace mygfx {

static constexpr int MAX_FRAME_COUNT = 2;

#define CHECK_MAIN_THREAD()\
    assert(SyncContext::mainThreadID == std::this_thread::get_id())

#define CHECK_RENDER_THREAD()\
    assert(SyncContext::renderThreadID == std::this_thread::get_id())

class SyncContext {
public:
    inline static std::thread::id mainThreadID;
    inline static std::thread::id renderThreadID;

    inline static double waitLogicMSec;
    inline static double waitRenderMSec;

    bool singleLoop() const;

    uint32_t workContext() const
    {
        return workFrame_;
    }

    int renderFrame() const
    {
        return renderFrame_;
    }

    void mainSemPost();
    bool mainSemWait();
    void renderSemPost();
    void waitRender();
    virtual void swapContext();

protected:
    int workFrame_ = 0;
    int renderFrame_ = -1;

    std::binary_semaphore renderSem_ = std::binary_semaphore { 1 };
    std::binary_semaphore mainSem_ = std::binary_semaphore { 0 };
};



}
