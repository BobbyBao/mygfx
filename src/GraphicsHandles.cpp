#include "GraphicsHandles.h"
#include "GraphicsDevice.h"
#include <deque>
#include <mutex>
#ifdef _MSVC_
#include <memory_resource>
#endif
namespace mygfx {

static std::recursive_mutex mLock;
static std::deque<std::pair<HwObject*, int>> mDisposables;
#ifdef _MSVC_
static size_t max_blocks_per_chunk = 1024;
static size_t largest_required_pool_block = 1024;
static std::pmr::synchronized_pool_resource sPool { std::pmr::pool_options { max_blocks_per_chunk, largest_required_pool_block } };

void* HwObject::operator new(std::size_t size)
{
    return sPool.allocate(size);
}

void* HwObject::operator new(std::size_t size, void* p)
{
    return p;
}

void HwObject::operator delete(void* ptr, std::size_t size)
{
    sPool.deallocate(ptr, size);
}
#endif
void HwObject::deleteThis()
{
    std::lock_guard<std::recursive_mutex> locker(mLock);
    mDisposables.emplace_back(this, 4);
}

void HwObject::gc(bool force)
{
    std::lock_guard<std::recursive_mutex> locker(mLock);
    while (!mDisposables.empty()) {
        auto& f = mDisposables.front();
        if (f.second == 0 || force) {
            delete f.first;
            mDisposables.pop_front();
        } else {
            break;
        }
    }

    for (auto& it : mDisposables) {
        --it.second;
    }
}

void HwResource::initState(ResourceState initialState)
{
#if RESOURCE_STATE
    mCurrentStates.emplace_back(initialState);
#endif
}

ResourceState HwResource::getCurrentResourceState(uint32_t subResource) const
{
#if RESOURCE_STATE
    if (subResource == 0xffffffff)
        return mCurrentStates.front();
    // CauldronAssert(ASSERT_CRITICAL, subResource < m_CurrentStates.size(), L"Trying to get state of sub-resource out of range!");
    return mCurrentStates.at(subResource);
#else
    return ResourceState::Undefined;
#endif
}

void HwResource::setCurrentResourceState(ResourceState newState, uint32_t subResource)
{
#if RESOURCE_STATE
    if (subResource == 0xffffffff)
        std::fill(mCurrentStates.begin(), mCurrentStates.end(), newState);
    else {
        // CauldronAssert(ASSERT_CRITICAL, subResource < m_CurrentStates.size(), L"Trying to get state of sub-resource out of range!");
        mCurrentStates.at(subResource) = newState;
    }
#endif
}

void HwResource::initSubResourceCount(uint32_t subResourceCount)
{
#if RESOURCE_STATE
    // CauldronAssert(ASSERT_CRITICAL, subResourceCount < 0xffffffff && subResourceCount > 0, L"Wrong number of sub-resources!");

    mCurrentStates.resize(subResourceCount);
    if (subResourceCount > 1)
        std::fill(mCurrentStates.begin() + 1, mCurrentStates.end(), mCurrentStates.front());
#endif
}

Ref<HwTexture> HwTexture::Black;
Ref<HwTexture> HwTexture::White;
Ref<HwTexture> HwTexture::Magenta;

SamplerHandle SamplerHandle::NearestRepeat;
SamplerHandle SamplerHandle::NearestClampToEdge;
SamplerHandle SamplerHandle::NearestClampToBorder;
SamplerHandle SamplerHandle::LinearRepeat;
SamplerHandle SamplerHandle::LinearClampToEdge;
SamplerHandle SamplerHandle::LinearClampToBorder;
SamplerHandle SamplerHandle::Shadow;

void SamplerHandle::init()
{
    SamplerInfo point;
    point.magFilter = Filter::NEAREST;
    point.minFilter = Filter::NEAREST;
    point.mipmapMode = Filter::NEAREST;
    NearestRepeat = device().createSampler(point);

    point.addressModeU = SamplerAddressMode::CLAMP_TO_EDGE;
    point.addressModeV = SamplerAddressMode::CLAMP_TO_EDGE;
    point.addressModeW = SamplerAddressMode::CLAMP_TO_EDGE;
    NearestClampToEdge = device().createSampler(point);

    point.addressModeU = SamplerAddressMode::CLAMP_TO_BORDER;
    point.addressModeV = SamplerAddressMode::CLAMP_TO_BORDER;
    point.addressModeW = SamplerAddressMode::CLAMP_TO_BORDER;
    NearestClampToBorder = device().createSampler(point);

    point.compareEnable = true;
    point.compareOp = CompareOp::LESS_OR_EQUAL;
    Shadow = device().createSampler(point);

    SamplerInfo linear;
    linear.magFilter = Filter::NEAREST;
    linear.minFilter = Filter::NEAREST;
    linear.mipmapMode = Filter::NEAREST;
    LinearRepeat = device().createSampler(linear);

    linear.addressModeU = SamplerAddressMode::CLAMP_TO_EDGE;
    linear.addressModeV = SamplerAddressMode::CLAMP_TO_EDGE;
    linear.addressModeW = SamplerAddressMode::CLAMP_TO_EDGE;
    LinearClampToEdge = device().createSampler(linear);

    linear.addressModeU = SamplerAddressMode::CLAMP_TO_BORDER;
    linear.addressModeV = SamplerAddressMode::CLAMP_TO_BORDER;
    linear.addressModeW = SamplerAddressMode::CLAMP_TO_BORDER;
    LinearClampToBorder = device().createSampler(linear);
}

void SamplerHandle::shutdown()
{
}

HwRenderPrimitive::HwRenderPrimitive(VertexData* geo, const DrawPrimitiveCommand& primitive)
    : mGeometry(geo)
    , drawArgs(primitive)
{
}

}
