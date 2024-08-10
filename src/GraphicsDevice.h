#pragma once

#include "GraphicsHandles.h"
#include "DynamicBufferPool.h"
#include "RenderQueue.h"
#include "SyncContext.h"
#include "PipelineState.h"
#include "Uniforms.h"

namespace mygfx {

	struct Settings {
		const char* name;
		bool validation = false;
	};

	struct PipelineState;
	struct RenderCommand;
	class RenderQueue;

	template<typename T>
	class ConcreteDispatcher;
	class Dispatcher;
	class CommandStream;
	
	using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
	using Clock = std::chrono::high_resolution_clock;

	class GraphicsDevice : public SyncContext {
	public:
		GraphicsDevice();
		virtual ~GraphicsDevice();
		
		virtual bool create(const Settings& settings) = 0;
		virtual const char* getDeviceName() const = 0;

		// Returns the dispatcher. This is only called once during initialization of the CommandStream,
		// so it doesn't matter that it's virtual.
		virtual Dispatcher getDispatcher() const noexcept;

		// called from CommandStream::execute on the render-thread
		// the fn function will execute a batch of driver commands
		// this gives the driver a chance to wrap their execution in a meaningful manner
		// the default implementation simply calls fn
		virtual void execute(std::function<void(void)> const& fn) noexcept;

		// This is called on debug build, or when enabled manually on the backend thread side.
		virtual void debugCommandBegin(CommandStream* cmds,
			bool synchronous, const char* methodName) noexcept {}

		virtual void debugCommandEnd(CommandStream* cmds,
			bool synchronous, const char* methodName) noexcept {}

		template<typename T>
		uint32_t allocConstant(const T& data)
		{
			void* pData;
			BufferInfo bufferInfo;
			if (!allocConstantBuffer(sizeof(T), &pData, &bufferInfo)) {
				return 0;
			}

			std::memcpy(pData, &data, sizeof(T));
			return (uint32_t)bufferInfo.offset;
		}

		template<typename T>
		uint32_t allocVertex(const std::span<T>& data)
		{
			void* pData;
			BufferInfo bufferInfo;
			if (!allocVertexBuffer(sizeof(T)*data.size(), &pData, &bufferInfo)) {
				return 0;
			}

			std::memcpy(pData, &data, sizeof(T) * data.size());
			return bufferInfo.offset;
		}

		const Ref<HwSwapchain>& getSwapChain() const { return mSwapChain; }

        template<typename T>
        Ref<HwBuffer> createBuffer1(BufferUsage usage, MemoryUsage memoryUsage, uint32_t count, const T* data = nullptr) {
            return createBuffer(usage, memoryUsage, count * sizeof(T), sizeof(T), (void*)data);
        }

		template<typename T>
		Ref<HwBuffer> createBuffer1(BufferUsage usage, MemoryUsage memoryUsage, const std::span<T>& data) {
			return createBuffer(usage, memoryUsage, data.size() * sizeof(T), sizeof(T), (void*)data.data());
		}

#define DECL_DRIVER_API(methodName, paramsDecl, params) \
    void methodName(paramsDecl) {}

#define DECL_DRIVER_API_SYNCHRONOUS(RetType, methodName, paramsDecl, params) \
    virtual RetType methodName(paramsDecl) = 0;

#define DECL_DRIVER_API_RETURN(RetType, methodName, paramsDecl, params) \
    virtual RetType methodName##S() noexcept = 0; \
    void methodName##R(RetType, paramsDecl) {}

#include "api/GraphicsAPI.inc"
		
	public:
		void swapContext() override;

		//render thread:
		void beginRender();
		void endRender();

		void post(const std::function<void()>& fn, int delay = 2);
	protected:
		void executeAll();
		
		Ref<HwSwapchain> mSwapChain;
		TimePoint mLastRenderTime;
		std::vector<std::tuple<std::function<void()>, int>> postCall_;
	};

	struct RenderCommand {
		HwRenderPrimitive* renderPrimitive;
		PipelineState pipelineState;
		Uniforms uniforms;
	};

	class RenderQueue : public HwObject {
	public:
		std::vector<RenderCommand>& getWriteCommands();
		const std::vector<RenderCommand>& getReadCommands() const;
		void clear();
	private:
		std::vector<RenderCommand> mRenderables[2];
	};

	struct Stats {
		static uint32_t getDrawCall();
		static auto& drawCall();
		
		static uint32_t getTriCount();
		static auto& triCount();
		
		static double getRenderTime();
		static double& renderTime();
		
		static void clear();
	private:
	
		inline static std::atomic<uint32_t> sDrawCall[2];
		inline static std::atomic<uint32_t> sTriCount[2];	
		inline static double sRenderTime[2];

	};
	
	extern GraphicsDevice* gInstance;

	inline GraphicsDevice& device() {
		
		return *gInstance;
	}

	inline auto& Stats::drawCall() {
		return sDrawCall[gInstance->renderFrame()];
	}
	
	inline auto& Stats::triCount() {
		return sTriCount[gInstance->renderFrame()];
	}

	inline double& Stats::renderTime() {
		return sRenderTime[gInstance->renderFrame()];
	}
	
}
