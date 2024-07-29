#include "GraphicsHandles.h"
#include "GraphicsDevice.h"
#include <deque>

namespace mygfx {

	static std::recursive_mutex mLock;
	static std::deque<std::pair<HwObject*, int>> mDisposables;
	        
	void HwObject::deleteThis() {

		std::lock_guard<std::recursive_mutex> locker(mLock);
		mDisposables.emplace_back(this, 4);
	}

	void HwObject::gc(bool force)
	{
		std::lock_guard<std::recursive_mutex> locker(mLock);
		while (!mDisposables.empty())
		{
			auto& f = mDisposables.front();
			if (f.second == 0 || force) {
				delete f.first;
				mDisposables.pop_front();
			}
			else {
				break;
			}

		}

		for (auto& it : mDisposables) {
			--it.second;
		}

	}

	void HwResource::initState(ResourceState initialState) {
#if RESOURCE_STATE
		mCurrentStates.emplace_back(initialState);
#endif
	}
	
    ResourceState HwResource::getCurrentResourceState(uint32_t subResource) const
    {
#if RESOURCE_STATE
        if (subResource == 0xffffffff)
            return mCurrentStates.front();
        //CauldronAssert(ASSERT_CRITICAL, subResource < m_CurrentStates.size(), L"Trying to get state of sub-resource out of range!");
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
        else
        {
            //CauldronAssert(ASSERT_CRITICAL, subResource < m_CurrentStates.size(), L"Trying to get state of sub-resource out of range!");
            mCurrentStates.at(subResource) = newState;
        }
#endif
    }
	
    void HwResource::initSubResourceCount(uint32_t subResourceCount)
    {
#if RESOURCE_STATE
        //CauldronAssert(ASSERT_CRITICAL, subResourceCount < 0xffffffff && subResourceCount > 0, L"Wrong number of sub-resources!");
		
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

	void SamplerHandle::init() {
		SamplerInfo point;
		point.magFilter = Filter::Nearest;
		point.minFilter = Filter::Nearest;
		point.mipmapMode = Filter::Nearest;
		NearestRepeat = device().createSampler(point);

		point.addressModeU = SamplerAddressMode::ClampToEdge;
		point.addressModeV = SamplerAddressMode::ClampToEdge;
		point.addressModeW = SamplerAddressMode::ClampToEdge;
		NearestClampToEdge = device().createSampler(point);

		point.addressModeU = SamplerAddressMode::ClampToBorder;
		point.addressModeV = SamplerAddressMode::ClampToBorder;
		point.addressModeW = SamplerAddressMode::ClampToBorder;
		NearestClampToBorder = device().createSampler(point);

		point.compareEnable = true;
		point.compareOp = CompareOp::LessOrEqual;
		Shadow = device().createSampler(point);

		SamplerInfo linear;
		linear.magFilter = Filter::Nearest;
		linear.minFilter = Filter::Nearest;
		linear.mipmapMode = Filter::Nearest;
		LinearRepeat = device().createSampler(linear);

		linear.addressModeU = SamplerAddressMode::ClampToEdge;
		linear.addressModeV = SamplerAddressMode::ClampToEdge;
		linear.addressModeW = SamplerAddressMode::ClampToEdge;
		LinearClampToEdge = device().createSampler(linear);

		linear.addressModeU = SamplerAddressMode::ClampToBorder;
		linear.addressModeV = SamplerAddressMode::ClampToBorder;
		linear.addressModeW = SamplerAddressMode::ClampToBorder;
		LinearClampToBorder = device().createSampler(linear);

	}

	void SamplerHandle::shutdown() {

	}

	HwRenderPrimitive::HwRenderPrimitive(VertexData* geo, const DrawPrimitiveCommand& primitive) : mGeometry(geo), drawArgs(primitive) {
	}

}

