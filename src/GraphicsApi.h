#pragma once

#include "api/CommandStream.h"
#include "api/CommandBufferQueue.h"

namespace mygfx {

	class GraphicsApi : public CommandStream {
	public:
		GraphicsApi(GraphicsDevice& driver);
		~GraphicsApi();

		using CommandStream::CommandStream;

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

		template<typename V>
		void drawUserPrimitives(const Span<V>& vertices, uint32_t firstInstance = 0) {
			
			void* pData;
			BufferInfo bufferInfo;
			if (!allocVertexBuffer(sizeof(V) * vertices.size(), &pData, &bufferInfo)) {
				return;
			}

			std::memcpy(pData, vertices.data(), sizeof(V) * vertices.size());

			bindVertexBuffer(0, bufferInfo.buffer, bufferInfo.offset);
			draw(vertices.size(), 1, 0, firstInstance);
		}

		template<typename V>
		void drawUserPrimitives(const Span<V>& vertices, const Span<uint16_t>& indices, uint32_t firstInstance = 0) {
			
			void* pData;
			BufferInfo bufferInfo;
			if (!allocVertexBuffer((uint32_t)vertices.size(), sizeof(V), &pData, &bufferInfo)) {
				return;
			}

			void* pData1;
			BufferInfo bufferInfo1;
			if (!allocIndexBuffer((uint32_t)indices.size(), sizeof(uint16_t), &pData1, &bufferInfo1)) {
				return;
			}

			std::memcpy(pData, vertices.data(), sizeof(V) * vertices.size());
			std::memcpy(pData1, indices.data(), sizeof(uint16_t) * indices.size());
			
			bindVertexBuffer(0, bufferInfo.buffer, bufferInfo.offset);
			bindIndexBuffer(bufferInfo1.buffer, bufferInfo1.offset, IndexType::UInt16);
			drawIndexed((uint32_t)indices.size(), 1, 0, 0, firstInstance);
		}

		template<typename V>
		void drawUserPrimitives(const Span<V>& vertices, const Span<uint32_t>& indices, uint32_t firstInstance = 0) {
			
			void* pData;
			BufferInfo bufferInfo;
			if (!allocVertexBuffer((uint32_t)vertices.size(), sizeof(V), &pData, &bufferInfo)) {
				return;
			}

			void* pData1;
			BufferInfo bufferInfo1;
			if (!allocIndexBuffer((uint32_t)indices.size(), sizeof(uint32_t), &pData1, &bufferInfo1)) {
				return;
			}

			std::memcpy(pData, vertices.data(), sizeof(V) * vertices.size());
			std::memcpy(pData1, indices.data(), sizeof(uint32_t) * indices.size());
			
			bindVertexBuffer(0, bufferInfo.buffer, bufferInfo.offset);
			bindIndexBuffer(bufferInfo1.buffer, bufferInfo1.offset, IndexType::UInt32);
			drawIndexed((uint32_t)indices.size(), 1, 0, 0, firstInstance);
		}

		void flush();
		void destroy();
	protected:
		void renderLoop();

		CommandBufferQueue mCommandBufferQueue;
		std::unique_ptr<std::thread> mRenderThread;
		bool mRendering = false;
		bool mDestroyed = false;

		inline static GraphicsApi* sGraphicsApi = nullptr;

		friend GraphicsApi& gfxApi() noexcept;
	};

	inline GraphicsApi& gfxApi() noexcept {
		return *GraphicsApi::sGraphicsApi;
	}

}
