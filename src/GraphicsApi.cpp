#include "GraphicsApi.h"

namespace mygfx {

#ifndef MIN_COMMAND_BUFFERS_SIZE_IN_MB
#    define MIN_COMMAND_BUFFERS_SIZE_IN_MB 2
#endif

#ifndef COMMAND_BUFFER_SIZE_IN_MB
#    define COMMAND_BUFFER_SIZE_IN_MB (MIN_COMMAND_BUFFERS_SIZE_IN_MB * 3)
#endif

	GraphicsApi::GraphicsApi(GraphicsDevice& driver) 
		: CommandStream(driver, nullptr), mCommandBufferQueue(MIN_COMMAND_BUFFERS_SIZE_IN_MB * 1024 * 1024, COMMAND_BUFFER_SIZE_IN_MB * 1024 * 1024){
		sGraphicsApi = this;
		mCurrentBuffer = &mCommandBufferQueue.getCircularBuffer();
	}

	GraphicsApi::~GraphicsApi() {
		destroy();
	}

	void GraphicsApi::flush() {

		auto& gfx = mDriver;
		if (gfx.singleLoop()) {
			gfx.swapContext();
		}
		else {

			gfx.waitRender();

			mCommandBufferQueue.flush();

			gfx.swapContext();

			if (mRenderThread == nullptr) {
				mRendering = true;
				mRenderThread = std::make_unique<std::thread>(&GraphicsApi::renderLoop, this);
			}

			gfx.mainSemPost();
		}

	}


	void GraphicsApi::renderLoop()
	{
		GraphicsDevice::renderThreadID = std::this_thread::get_id();

		while (mRendering) {
			auto buffers = mCommandBufferQueue.waitForCommands();
			if (UTILS_UNLIKELY(buffers.empty())) {
				continue;
			}

			mDriver.beginRender();
			// execute all command buffers
			auto& api = *this;
			for (auto& item : buffers) {
				if (UTILS_LIKELY(item.begin)) {
					api.execute(item.begin);
					mCommandBufferQueue.releaseBuffer(item);
				}
			}
			mDriver.endRender();
		}
	}

	void GraphicsApi::destroy() {

		if (mDestroyed) {
			return;
		}

		mRendering = false;

		device().mainSemPost();

		device().waitRender();


		// now wait for all pending commands to be executed and the thread to exit
		mCommandBufferQueue.requestExit();

		if (mRenderThread != nullptr)
			mRenderThread->join();

		delete& mDriver;

		sGraphicsApi = nullptr;
		mDestroyed = true;
	}
}
