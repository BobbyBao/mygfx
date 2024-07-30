#pragma once
#include "utils/SharedPtr.h"
#include "GraphicsDefs.h"
#include <vector>

namespace mygfx {

	class HwObject : public RefCounted {
	public:
		static void gc(bool force = false);
	protected:
        void deleteThis() override;

		friend class GraphicsDevice;
	};

	class HwResource : public HwObject {
	public:
		void initState(ResourceState initialState);
        ResourceState getCurrentResourceState(uint32_t subResource = 0xffffffff) const;
        void setCurrentResourceState(ResourceState newState, uint32_t subResource = 0xffffffff);
		void initSubResourceCount(uint32_t subResourceCount);

		ResourceType type = ResourceType::Unknown;
		std::vector<ResourceState> mCurrentStates;
	};

	class HwBuffer : public HwResource {
	public:
		HwBuffer() { type = ResourceType::Buffer; }

		uint64_t size = 0;
		BufferUsage usage = BufferUsage::None;
		MemoryUsage memoryUsage = MemoryUsage::GpuOnly;
		uint16_t stride = 0;
		uint16_t extra = 0;
		void* mapped = nullptr;
		uint64_t deviceAddress = 0;
		uint64_t bufferOffset = 0;

		uint64_t count() const {
			return stride == 0 ? 0 : size / stride;
		}
	};
		
	inline uint64_t BufferInfo::getDeviceAddress() const {
		return buffer ? buffer->deviceAddress + offset : 0;
	}
	
	class HwTextureView : public HwObject {
	public:		
		
		inline int index() const { return index_; }
		
		int index_ = -1;
	};
	
	class HwTexture : public HwResource {
	public:
		HwTexture() { type = ResourceType::Image; }

		uint16_t width, height;
		uint16_t depth = 1;
		uint16_t layerCount = 1;
        uint16_t faceCount = 1;
		uint16_t mipLevels = 1;
		Format format;
		SamplerType samplerType = SamplerType::SAMPLER_2D;
		
		Ref<HwTextureView> mSRV;
		Ref<HwTextureView> mRTV;
		Ref<HwTextureView> mDSV;

		HwTextureView* getSRV() const { return mSRV.get(); }
		HwTextureView* getRTV() const { return mRTV.get(); }
		HwTextureView* getDSV() const { return mDSV.get(); }

		bool isCubemap() const
		{
			return faceCount == 6;
		}

        static Ref<HwTexture> Black;
        static Ref<HwTexture> White;
        static Ref<HwTexture> Magenta;			
	};

	struct SamplerHandle {
		uint16_t index;

		static SamplerHandle NearestRepeat;
		static SamplerHandle NearestClampToEdge;
		static SamplerHandle NearestClampToBorder;
		static SamplerHandle LinearRepeat;
		static SamplerHandle LinearClampToEdge;
		static SamplerHandle LinearClampToBorder;
		static SamplerHandle Shadow;

		static void init();
		static void shutdown();
	};
		

	class ShaderResourceInfo;

	class HwShaderModule : public HwObject {
	public:
		std::vector<Ref<ShaderResourceInfo>> shaderResourceInfo;
		Ref<ShaderResourceInfo> materialUniforms;
	};
	
	class HwVertexInput : public HwObject {
	public:
	};

	class HwProgram : public HwObject {
	public:
		ShaderResourceInfo* getShaderResource(const String& name);
		HwVertexInput* vertexInput;
	};

	struct RenderTargetDesc {
		uint32_t width, height;
		
		std::vector<Ref<HwTexture>> colorAttachments;
		Ref<HwTexture> depthAttachment = nullptr;
	};

	class HwRenderTarget : public HwObject {
	public:
		uint32_t width, height;
		uint32_t currentIndex = 0;
		bool isSwapchain = false;
	};

	struct RenderPassInfo {
		Viewport viewport {};
		uint32_t passID = 0;
		TargetBufferFlags clearFlags = TargetBufferFlags::NONE;
		TargetBufferFlags loadFlags = TargetBufferFlags::NONE;
		TargetBufferFlags storeFlags = TargetBufferFlags::ALL;
		float clearColor[4] = {0};
		float clearDepth = 0.0;
		uint32_t clearStencil = 0;
	};
	
	struct SwapChainDesc {
		void* windowInstance;
		void* window;
		uint32_t width = 1280;
		uint32_t height = 720;
		Format colorFormat = Format::R8G8B8A8_SNORM;
		Format depthFormat = Format::UNDEFINED;
		bool fullscreen = false;
		bool vsync = false;
	};

	class HwSwapchain : public HwObject {
	public:
		SwapChainDesc desc;
		Ref<HwRenderTarget> renderTarget;
	};
	
	struct DrawPrimitiveCommand {

		union {
			uint32_t    indexCount;
			uint32_t    vertexCount;
		};

		uint32_t    instanceCount = 1;

		union {
			uint32_t    firstIndex = 0;
			uint32_t    firstVertex;
		};

		int32_t     vertexOffset = 0;
		uint32_t    firstInstance = 0;
	};

	class VertexData : public RefCounted {
	public:
		std::vector<Ref<HwBuffer>> vertexBuffers;
		Ref<HwBuffer> indexBuffer;
	};

	class HwRenderPrimitive : public HwObject {
	public:
		HwRenderPrimitive(VertexData* geo, const DrawPrimitiveCommand& primitive);

		DrawPrimitiveCommand drawArgs;	
	protected:
		Ref<VertexData> mGeometry;
	};



	class HwDescriptorSet : public HwObject {
	public:
		virtual void bind(uint32_t dstBinding, const BufferInfo& buffer) = 0;
		virtual void bind(uint32_t dstBinding, HwTextureView* texView) = 0;
		virtual void bind(uint32_t dstBinding, HwBuffer* buffer) = 0;
	};
	
	class Uniforms {
	public:
		static constexpr int MAX_COUNT = 4;
		Uniforms();
				
		Uniforms(uint32_t v0) {
			push_back(v0);
		}
		
		Uniforms(uint32_t v0, uint32_t v1) {
			push_back(v0);
			push_back(v1);
		}
		
		Uniforms(uint32_t v0, uint32_t v1, uint32_t v2) {
			push_back(v0);
			push_back(v1);
			push_back(v2);
		}
		
		Uniforms(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3) {
			push_back(v0);
			push_back(v1);
			push_back(v2);
			push_back(v3);
		}

		Uniforms(std::initializer_list<uint32_t> const& list);

		void set(uint32_t v0) {
			mSize = 0;
			push_back(v0);
		}
		
		void set(uint32_t v0, uint32_t v1) {
			mSize = 0;
			push_back(v0);
			push_back(v1);
		}
		
		void set(uint32_t v0, uint32_t v1, uint32_t v2) {
			mSize = 0;
			push_back(v0);
			push_back(v1);
			push_back(v2);
		}
		
		void set(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3) {
			mSize = 0;
			push_back(v0);
			push_back(v1);
			push_back(v2);
			push_back(v3);
		}

		uint32_t size() const {
			return mSize;
		}
		
		const uint32_t& operator[](uint32_t index) const {
			assert(index < mSize);
			return mValues[index];
		}

		uint32_t& operator[](uint32_t index) {
			assert (index < mSize);
			return mValues[index];
		}

		const uint32_t* data() const { return &mValues[0]; }
		uint32_t* data() { return &mValues[0]; }
		
		void push_back(uint32_t v);
		void pop_back();
		void clear();
	private:
		uint32_t mValues[MAX_COUNT] = {0};
		uint32_t mSize = 0;
	};
	
	inline Uniforms::Uniforms() = default;

	inline Uniforms::Uniforms(std::initializer_list<uint32_t> const& list) {
		for (auto& v : list) {
			push_back(v);
		}
	}

	inline void Uniforms::push_back(uint32_t v) {
		mValues[mSize++] = v;
	}

	inline void Uniforms::pop_back() {
		if (mSize > 0) {
			--mSize;
		}
	}

	inline void Uniforms::clear() {
		mSize = 0;
	}
}

