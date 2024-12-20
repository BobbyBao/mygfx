#include "SyncContext.h"

namespace mygfx {

bool SyncContext::singleLoop() const
{
#if SINGLE_LOOP
    return true;
#else
    return false;
#endif
}

void SyncContext::mainSemPost()
{
#if SINGLE_LOOP
#else
    mainSem_.release();
#endif
}

bool SyncContext::mainSemWait()
{
#if SINGLE_LOOP
    return true;
#else
    auto ok = mainSem_.try_acquire();
    if (ok) {
        return true;
    }
    return false;
#endif
}

void SyncContext::renderSemPost()
{
#if SINGLE_LOOP
#else
    renderSem_.release();
#endif
}

void SyncContext::waitRender()
{
#if SINGLE_LOOP
#else
    renderSem_.acquire();
#endif
}

void SyncContext::swapContext()
{
    renderFrame_ = workFrame_;
    workFrame_ = (workFrame_ + 1) % (int)MAX_FRAME_COUNT;
}

}