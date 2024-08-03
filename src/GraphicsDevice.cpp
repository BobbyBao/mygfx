#include "GraphicsDevice.h"
#include "utils/Log.h"
#include "api/Dispatcher.h"
#include "FrameListener.h"

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
	
	Dispatcher GraphicsDevice::getDispatcher() const noexcept {
		return {};
	}

	void GraphicsDevice::execute(std::function<void(void)> const& fn) noexcept {
		fn();
	}
	
	void GraphicsDevice::swapContext() {
		
		SyncContext::swapContext();

		FrameChangeListener::callFrameChange();

		Stats::clear();
	}

	void GraphicsDevice::beginRender() {
		mainSemWait();
		mLastRenderTime = Clock::now();
	}

	void GraphicsDevice::endRender() {

		for (auto it = postCall_.begin(); it != postCall_.end();) {
			auto& timer = std::get<1>(*it);
			if (--timer == 0) {
				auto& fn = std::get<0>(*it);
				fn();
				it = postCall_.erase(it);
			}
			else {
				++it;
			}
		}

		Stats::renderTime() = std::chrono::duration<double, std::milli>(Clock::now() - mLastRenderTime).count();
		renderSemPost();
	}

	void GraphicsDevice::post(const std::function<void()>& fn, int delay)
	{
		postCall_.push_back(std::make_tuple(fn, delay));
	}

	void GraphicsDevice::executeAll()
	{
		for (auto& t : postCall_) {
			auto& fn = std::get<0>(t);
			fn();
		}

		postCall_.clear();
	}

	std::vector<RenderCommand>& RenderQueue::getWriteCommands() {
		return mRenderables[gInstance->workContext()];
	}

	const std::vector<RenderCommand>& RenderQueue::getReadCommands() const {
		return mRenderables[gInstance->renderFrame()];
	}

	void RenderQueue::clear() {
		mRenderables[gInstance->workContext()].clear();
	}
	
	uint32_t Stats::getDrawCall() {
		return sDrawCall[gInstance->workContext()];
	}
	
	uint32_t Stats::getTriCount() {		
		return sTriCount[gInstance->workContext()];
	}

	double Stats::getRenderTime() {
		return sRenderTime[gInstance->workContext()];
	}


	void Stats::clear() {
		drawCall() = 0;
		triCount() = 0;
		renderTime() = 0;
	}
}