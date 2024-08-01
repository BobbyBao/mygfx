#if (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
// SRS - Enable beta extensions and make VK_KHR_portability_subset visible
#define VK_ENABLE_BETA_EXTENSIONS
#endif
#include "VulkanDevice.h"
#include <unordered_set>
#include "VulkanBuffer.h"
#include "vulkan/VulkanTexture.h"
#include "utils/Log.h"
#include "VulkanHandles.h"
#include "VulkanSwapchain.h"
#include "VulkanTextureView.h"
#include "VulkanStagePool.h"
#include "api/CommandStreamDispatcher.h"

#define VOLK_IMPLEMENTATION
#include "Volk/volk.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include "vulkan/ShaderCompiler.h"
#include "VkFormatHelper.h"

namespace mygfx
{	
	VulkanDevice& gfx() {
		return static_cast<VulkanDevice&>(device());
	}

	VulkanDevice::VulkanDevice()
	{
	}

	VulkanDevice::~VulkanDevice()
	{
	}
	
	bool VulkanDevice::init(const Settings& settings)
	{
		mSwapchainDesc.width = settings.width;
		mSwapchainDesc.height = settings.height;
		mSwapchainDesc.fullscreen = settings.fullscreen;
		mSwapchainDesc.vsync = settings.vsync;
		
		volkInitialize();

		// Vulkan instance
		VkResult err = createInstance(settings.name, settings.validation);
		if (err) {
			tools::exitFatal("Could not create Vulkan instance : \n" + tools::errorString(err), err);
			return false;
		}

		volkLoadInstance(instance);

		if (!selectPhysicalDevice()) {
			return false;
		}

		getEnabledFeatures();
		getEnabledExtensions();
		
		VkResult res = createLogicalDevice(enabledFeatures, enabledDeviceExtensions);
		if (res != VK_SUCCESS) {
			tools::exitFatal("Could not create Vulkan device: \n" + tools::errorString(res), res);
			return false;
		}

		// Get a graphics queue from the device
		vkGetDeviceQueue(device, queueFamilyIndices.graphics, 0, &queue);
		vkGetDeviceQueue(device, queueFamilyIndices.compute, 0, &computeQueue);
		vkGetDeviceQueue(device, queueFamilyIndices.transfer, 0, &transferQueue);

		// Create synchronization objects
		VkSemaphoreCreateInfo semaphoreCreateInfo = initializers::semaphoreCreateInfo();
		// Create a semaphore used to synchronize image presentation
		// Ensures that the image is displayed before we start submitting new commands to the queue
		VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.presentComplete));
		// Create a semaphore used to synchronize command submission
		// Ensures that the image is not presented until all commands have been submitted and executed
		VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.renderComplete));

		// Set up submit info structure
		// Semaphores will stay the same during application lifetime
		// Command buffer submission info is set by each example
		submitInfo = initializers::submitInfo();
		submitInfo.pWaitDstStageMask = &submitPipelineStages;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &semaphores.presentComplete;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &semaphores.renderComplete;


		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = physicalDevice;
		allocatorInfo.device = device;
		allocatorInfo.instance = instance;
		allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
		vmaCreateAllocator(&allocatorInfo, &vmaAllocator_);
		
		mDescriptorPoolManager.init();

		mStagePool = new VulkanStagePool(vmaAllocator_);

		mTextureSet = new DescriptorTable(DescriptorType::CombinedImageSampler | DescriptorType::StorageImage);
		//mImageSet = new DescriptorTable(DescriptorType::StorageImage);
		//mBufferSet = new DescriptorTable(DescriptorType::StorageBuffer);	

		mCommandQueues[0].Init(CommandQueueType::Graphics, queueFamilyIndices.graphics, 0, 3, "");
		mCommandQueues[1].Init(CommandQueueType::Compute, queueFamilyIndices.compute, 0, 3, "");
		mCommandQueues[2].Init(CommandQueueType::Copy, queueFamilyIndices.transfer, 1, 3, "");
		return true;
	}

	void VulkanDevice::create(void* windowInstance, void* window)
	{
		createCommandPool();
		createCommandBuffers();
		createSynchronizationPrimitives();

		SamplerHandle::init();

		// Create a 'dynamic' constant buffer
		const uint32_t constantBuffersMemSize = 32 * 1024 * 1024;
		mConstantBufferRing.create(BufferUsage::Uniform | BufferUsage::Storage | BufferUsage::ShaderDeviceAddress,
			2560 * 1024, MAX_BACKBUFFER_COUNT, constantBuffersMemSize, "Uniforms");
#if LARGE_DYNAMIC_INDEX
		const uint32_t vertexBuffersMemSize = 64 * 1024 * 1024;
#else
		const uint32_t vertexBuffersMemSize = 16 * 1024 * 1024;
#endif
		mVertexBufferRing.create(BufferUsage::Vertex | BufferUsage::Index, MAX_BACKBUFFER_COUNT, vertexBuffersMemSize, "VertexBuffers|IndexBuffers");
						
		const uint32_t uploadHeapMemSize = 64 * 1024 * 1024;
		mUploadHeap.create(uploadHeapMemSize);    // initialize an upload heap (uses suballocation for faster results)

		mSwapchainDesc.windowInstance = windowInstance;
		mSwapchainDesc.window = window;		
		swapChain = makeShared<VulkanSwapChain>(mSwapchainDesc);
		swapChain->recreate(mSwapchainDesc);
		mSwapChain = swapChain;
	}

	const char* VulkanDevice::getDeviceName() const {
		return properties.deviceName;
	}

	void VulkanDevice::updateDynamicDescriptorSet(int index, uint32_t size, VkDescriptorSet descriptorSet)
	{
		VulkanBuffer* vkBuffer = (VulkanBuffer*)mConstantBufferRing.getBuffer();
		VkDescriptorBufferInfo out = {};
		out.buffer = vkBuffer->buffer;
		out.offset = 0;
		out.range = size;

		VkWriteDescriptorSet write;
		write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = NULL;
		write.dstSet = descriptorSet;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		write.pBufferInfo = &out;
		write.dstArrayElement = 0;
		write.dstBinding = index;

		vkUpdateDescriptorSets(gfx().device, 1, &write, 0, NULL);
	}

	DescriptorSet* VulkanDevice::getDynamicUniformSet(DescriptorSetLayout* layout) {
		uint32_t bindingNum = layout->numBindings();
		if (bindingNum == 0) {
			return nullptr;
		}

		std::lock_guard<std::mutex> locker(mUniformSetLock);
		Ref<ResourceSet>& rs = mUniformSet[bindingNum - 1];
		if (rs == nullptr) {
			rs = makeShared<ResourceSet>();
		}

		auto ds = rs->getDescriptorSet(layout);
	
		for(uint32_t j = 0; j < bindingNum; j++) {
			ds->dynamicBufferSize[j] = 256;
			updateDynamicDescriptorSet(j, 256, *ds);
		}
		return ds;
	}
	
	void VulkanDevice::createCommandPool()
	{
		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = queueFamilyIndices.graphics;
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &cmdPool));
	}

	void VulkanDevice::createCommandBuffers()
	{
		cmdBuffers.resize(MAX_BACKBUFFER_COUNT + 1);

		VkCommandBufferAllocateInfo cmdBufAllocateInfo =
			initializers::commandBufferAllocateInfo(
				cmdPool,
				VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				static_cast<uint32_t>(cmdBuffers.size()));

		VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, cmdBuffers.data()));

		drawCmdBuffers.reserve(cmdBuffers.size());
		drawCmdBuffers.clear();
		freeCmdBuffers.clear();

		for (int i = 0; i < cmdBuffers.size(); i++)
		{
			drawCmdBuffers.emplace_back(cmdBuffers[i]);
			freeCmdBuffers.push_back(&drawCmdBuffers.back());
		}
	}
	
	void VulkanDevice::createSynchronizationPrimitives()
	{
		// Wait fences to sync command buffer access
		VkFenceCreateInfo fenceCreateInfo = initializers::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
		waitFences.resize(drawCmdBuffers.size());
		for (auto& fence : waitFences) {
			VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, nullptr, &fence));
		}
	}

	void VulkanDevice::destroyCommandBuffers()
	{
		vkFreeCommandBuffers(device, cmdPool, static_cast<uint32_t>(drawCmdBuffers.size()), cmdBuffers.data());
	}
		
	CommandBuffer* VulkanDevice::getCommandBuffer(CommandQueueType queueType, uint32_t count) {
		return mCommandQueues[(int)queueType].getCommandBuffer(count);
	}

	void VulkanDevice::freeCommandBuffer(CommandBuffer* cmd) {
		mCommandQueues[(int)cmd->getCommandQueueType()].freeCommandBuffer(cmd);
	}
	
	void VulkanDevice::executeCommand(CommandQueueType queueType, const std::function<void(const CommandBuffer&)>& fn) {
		auto cmd = getCommandBuffer(queueType);
        cmd->begin();
		fn(*cmd);
		cmd->end();
		uint64_t waitValue = mCommandQueues[(int)queueType].Submit(&cmd->cmd, 1, VK_NULL_HANDLE, VK_NULL_HANDLE, false);
		mCommandQueues[(int)queueType].Wait(device, waitValue);
		freeCommandBuffer(cmd);
	}
	
	void VulkanDevice::reload(uint32_t destWidth, uint32_t destHeight) {

		// Ensure all operations on the device have been finished before destroying resources
		vkDeviceWaitIdle(device);

		// Recreate swap chain
		mSwapchainDesc.width = destWidth;
		mSwapchainDesc.height = destHeight;		
		swapChain->recreate(mSwapchainDesc);
		
		// Command buffers need to be recreated as they may store
		// references to the recreated frame buffer
		destroyCommandBuffers();
		createCommandBuffers();

		// SRS - Recreate fences in case number of swapchain images has changed on resize
		for (auto& fence : waitFences) {
			vkDestroyFence(device, fence, nullptr);
		}

		createSynchronizationPrimitives();

		vkDeviceWaitIdle(device);

	}

	void VulkanDevice::destroy()
	{
		// Flush device to make sure all resources can be freed
		if (device != VK_NULL_HANDLE) {
			vkDeviceWaitIdle(device);
		}

		HwObject::gc(true);

		// Clean up Vulkan resources
		swapChain.reset();

		if (commandPool)
		{
			vkDestroyCommandPool(device, commandPool, nullptr);
		}

		destroyCommandBuffers();

		vkDestroyCommandPool(device, cmdPool, nullptr);
		vkDestroySemaphore(device, semaphores.presentComplete, nullptr);
		vkDestroySemaphore(device, semaphores.renderComplete, nullptr);

		for (auto& fence : waitFences) {
			vkDestroyFence(device, fence, nullptr);
		}
		
		mTextureSet.reset();

		mConstantBufferRing.destroy();
		mVertexBufferRing.destroy();

		mDescriptorPoolManager.destroyAll();

		SamplerHandle::shutdown();

		for (auto& s : mSamplers) {
			vkDestroySampler(device, s.second, nullptr);
		}

		mSamplers.clear();
		mStagePool->terminate();
		delete mStagePool;

		vmaDestroyAllocator(vmaAllocator_);
		vmaAllocator_ = NULL;

		mCommandQueues[0].Release(device);
		mCommandQueues[1].Release(device);
		mCommandQueues[2].Release(device);

		if (device)
		{
			vkDestroyDevice(device, nullptr);
		}

		debug::freeDebugCallback(instance);


		vkDestroyInstance(instance, nullptr);
	}

	bool VulkanDevice::allocConstantBuffer(uint32_t size, void** pData, BufferInfo* pOut)
	{
		return mConstantBufferRing.allocBuffer(size, pData, pOut);
	}

	bool VulkanDevice::allocVertexBuffer(uint32_t numbeOfVertices, uint32_t strideInBytes, void** pData, BufferInfo* pOut)
	{
		return mVertexBufferRing.allocVertexBuffer(numbeOfVertices, strideInBytes, pData, pOut);
	}

	bool VulkanDevice::allocVertexBuffer(uint32_t sizeInBytes, void** pData, BufferInfo* pOut)
	{
		return mVertexBufferRing.allocBuffer(sizeInBytes, pData, pOut);
	}

	bool VulkanDevice::allocIndexBuffer(uint32_t numbeOfIndices, uint32_t strideInBytes, void** pData, BufferInfo* pOut)
	{
		return mVertexBufferRing.allocIndexBuffer(numbeOfIndices, strideInBytes, pData, pOut);
	}

	bool VulkanDevice::allocIndexBuffer(uint32_t sizeInBytes, void** pData, BufferInfo* pOut)
	{
		return mVertexBufferRing.allocBuffer(sizeInBytes, pData, pOut);
	}

	SharedPtr<HwBuffer> VulkanDevice::createBuffer(BufferUsage usage, MemoryUsage memoryUsage, uint64_t size, uint16_t stride, const void* data) {
		return makeShared<VulkanBuffer>(usage, memoryUsage, size, stride, data);
	}

	SharedPtr<HwTexture> VulkanDevice::createTexture(const TextureData& textureData, SamplerInfo sampler) {
		return makeShared<VulkanTexture>(textureData, sampler);
	}
	
	bool VulkanDevice::copyData(HwTexture* tex, TextureDataProvider* dataProvider) {
		VulkanTexture* vkTex = static_cast<VulkanTexture*>(tex);
		return vkTex->copyData(dataProvider);
	}


	VkSampler createVkSampler(const SamplerInfo& info) {

		VkSamplerCreateInfo samplerCI = {};
		samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCI.magFilter = (VkFilter)info.magFilter;
		samplerCI.minFilter = (VkFilter)info.minFilter;
		samplerCI.mipmapMode = (VkSamplerMipmapMode)info.mipmapMode;
		samplerCI.addressModeU = (VkSamplerAddressMode)info.addressModeU;
		samplerCI.addressModeV = (VkSamplerAddressMode)info.addressModeV;
		samplerCI.addressModeW = (VkSamplerAddressMode)info.addressModeW;

		samplerCI.mipLodBias = 0.0f;
		samplerCI.anisotropyEnable = info.anisotropyEnable;
		samplerCI.maxAnisotropy = 1.0f;
		samplerCI.compareEnable = info.compareEnable;
		samplerCI.compareOp = (VkCompareOp)info.compareOp;
		samplerCI.minLod = -1000;
		samplerCI.maxLod = 1000;

		samplerCI.borderColor = (VkBorderColor)info.borderColor;
		samplerCI.unnormalizedCoordinates = info.unnormalizedCoordinates;

		VkSampler vkSampler;
		vkCreateSampler(gfx().device, &samplerCI, nullptr, &vkSampler);
		return vkSampler;
	}

	SamplerHandle VulkanDevice::createSampler(const SamplerInfo& info) {	
		std::lock_guard<std::mutex> lock(mSamplerLock);
		for (int i = 0; i < mSamplers.size(); i++) {
			auto& s = mSamplers[i];
			if (s.first == info) {
				return SamplerHandle{ .index = (uint16_t)i };
			}
		}
		auto s = createVkSampler(info);
		auto index = mSamplers.size();
		mSamplers.emplace_back(info, s);
		return SamplerHandle{ .index = (uint16_t)index };
	}

	Ref<HwShaderModule> VulkanDevice::createShaderModule(ShaderStage stage, const ByteArray& shaderCode, ShaderCodeType shaderCodeType){
		return makeShared<VulkanShaderModule>(stage, shaderCode, shaderCodeType);
	}

	Ref<HwShaderModule> VulkanDevice::compileShaderModule(ShaderSourceType sourceType, const ShaderStage shader_type, const String& pShaderCode, const char* pShaderEntryPoint, const char* shaderCompilerParams, const DefineList* pDefines){
		return ShaderCompiler::compileFromString(sourceType, ToVkShaderStage(shader_type), pShaderCode, pShaderEntryPoint, shaderCompilerParams, pDefines);
	}

	SharedPtr<HwProgram> VulkanDevice::createProgram(Ref<HwShaderModule>* shaderModules, uint32_t count) {
		SharedPtr<VulkanProgram> handle = makeShared<VulkanProgram>(shaderModules, count);
		return handle;
	}

	SharedPtr<HwRenderPrimitive> VulkanDevice::createRenderPrimitive(VertexData* geo, const DrawPrimitiveCommand& primitive) {
		return makeShared<VulkanRenderPrimitive>(geo, primitive);
	}

	SharedPtr<HwRenderTarget> VulkanDevice::createRenderTarget(const RenderTargetDesc& desc) {
		return makeShared<VulkanRenderTarget>(desc);
	}

	SharedPtr<HwSwapchain> VulkanDevice::createSwapchain(const SwapChainDesc& desc) {
		return makeShared<VulkanSwapChain>(desc);
	}
	
	SharedPtr<HwVertexInput> VulkanDevice::createVertexInput(const FormatList& fmts, const FormatList& fmts1)
	{
		return makeShared<VulkanVertexInput>(fmts, fmts1);
	}
	
	SharedPtr<HwDescriptorSet> VulkanDevice::createDescriptorSet(const Span<DescriptorSetLayoutBinding>& bindings)
	{
		return makeShared<DescriptorSet>(bindings);
	}

	void VulkanDevice::updateTexture(HwTexture* texture,
		uint32_t level,
		uint32_t xoffset,
		uint32_t yoffset,
		uint32_t zoffset,
		uint32_t width,
		uint32_t height,
		uint32_t depth,
		const void* data,
		size_t size)
	{
		VulkanTexture* vkTexture = static_cast<VulkanTexture*>(texture);
		vkTexture->setData(level, xoffset, yoffset, zoffset, width, height, depth, data, size);
	}

	void VulkanDevice::updateBuffer(HwBuffer* buffer, const void* data, size_t size, size_t offset)
	{
		VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
		vkBuffer->setData(data, size, offset);
	}
	
	void VulkanDevice::updateDescriptorSet1(HwDescriptorSet* descriptorSet,uint32_t dstBinding, HwTextureView* texView) {
		static_cast<DescriptorSet*>(descriptorSet)->bind(dstBinding, texView);
	}

	void VulkanDevice::updateDescriptorSet2(HwDescriptorSet* descriptorSet,uint32_t dstBinding, HwBuffer* buffer) {
		static_cast<DescriptorSet*>(descriptorSet)->bind(dstBinding, buffer);
	}

	void VulkanDevice::updateDescriptorSet3(HwDescriptorSet* descriptorSet,uint32_t dstBinding, const BufferInfo& buffer) {
		static_cast<DescriptorSet*>(descriptorSet)->bind(dstBinding, buffer);
	}

	void VulkanDevice::beginFrame(int) {
		currentCmd = getFree();
		currentCmd->begin();
	}

	void VulkanDevice::beginRendering(HwRenderTarget* pRT, const RenderPassInfo& renderInfo)
	{
		currentCmd->beginRendering(pRT, renderInfo);
		mRenderPassInfo = renderInfo;
		mRenderTarget = (VulkanRenderTarget*)pRT;
		colorAttachmentCount = mRenderTarget->numAttachments();
		for (uint32_t i = 0; i < colorAttachmentCount; i++) {
			colorAttachmentFormats[i] = mRenderTarget->colorAttachments[i]->vkFormat;
		}

		if (mRenderTarget->depthAttachment) {
			depthAttachmentFormat = mRenderTarget->depthAttachment->vkFormat;
			if (isDepthStencilFormat(depthAttachmentFormat)) {
				stencilAttachmentFormat = depthAttachmentFormat;
			} else {
				stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
			}
			
		} else {
			depthAttachmentFormat = VK_FORMAT_UNDEFINED;
			stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
		}
	}

	void VulkanDevice::endRendering(HwRenderTarget* pRT)
	{
		currentCmd->endRendering(pRT);
		mRenderTarget = nullptr;
	}

	void VulkanDevice::prepareFrame(int v)
	{
		// Acquire the next image from the swap chain
		VkResult result = swapChain->acquireNextImage(semaphores.presentComplete, &currentBuffer);
		swapChain->renderTarget->currentIndex = currentBuffer;

		for (auto c : submitCmdBuffers[currentBuffer]) {
			freeCmdBuffers.push_back(c);
		}
		submitCmdBuffers[currentBuffer].clear();

		// Recreate the swapchain if it's no longer compatible with the surface (OUT_OF_DATE)
		// SRS - If no longer optimal (VK_SUBOPTIMAL_KHR), wait until submitFrame() in case number of swapchain images will change on resize
		if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) {
			if (result == VK_ERROR_OUT_OF_DATE_KHR) {
				LOG_ERROR("VK_ERROR_OUT_OF_DATE_KHR");
				//windowResize();
			}
			return;
		}
		else {
			VK_CHECK_RESULT(result);
		}
	}

	void VulkanDevice::resetState(int)
	{
		currentCmd->resetState();
	}
	
	void VulkanDevice::setViewport(float topX, float topY, float width, float height, float minDepth, float maxDepth)
	{
		VkViewport viewport
		{
			.x = topX,
			.y = topY + height,
			.width = width,
			.height = -height,
			.minDepth = minDepth,
			.maxDepth = maxDepth
		};

		currentCmd->setViewport(1, &viewport);
	}
	
	void VulkanDevice::setScissor(uint32_t topX, uint32_t topY, uint32_t width, uint32_t height)
	{
		VkRect2D scissor
		{
			.offset = {(int32_t)topX, (int32_t)topY },
			.extent = {(uint32_t)(width), (uint32_t)(height)},
		};

		currentCmd->setScissor(1, &scissor);
	}

	void VulkanDevice::setViewportAndScissor(uint32_t topX, uint32_t topY, uint32_t width, uint32_t height)
	{
		currentCmd->setViewportAndScissor(topX, topY, width, height);
	}

	void VulkanDevice::setVertexInput(HwVertexInput* vertexInput) {
		currentCmd->setVertexInput((VulkanVertexInput*)vertexInput);
	}
	
	void VulkanDevice::setPrimitiveTopology(PrimitiveTopology primitiveTopology) {
		currentCmd->setPrimitiveTopology(primitiveTopology);
	}
	
	void VulkanDevice::setPrimitiveRestartEnable(bool restartEnable) {
		currentCmd->setPrimitiveRestartEnable(restartEnable);
	}

	void VulkanDevice::bindShaderProgram(HwProgram* program) {
		currentCmd->bindShaderProgram(program);
	}
	
	void VulkanDevice::bindRasterState(RasterState* rasterState) {
		currentCmd->bindRasterState(rasterState);
	}
	
	void VulkanDevice::bindColorBlendState(ColorBlendState* colorBlendState) {
		currentCmd->bindColorBlendState(colorBlendState);
	}
	
	void VulkanDevice::bindDepthState(DepthState* depthState) {
		currentCmd->bindDepthState(depthState);
	}
	
	void VulkanDevice::bindStencilState(StencilState* stencilState) {
		currentCmd->bindStencilState(stencilState);
	}

	void VulkanDevice::bindPipelineState(const PipelineState& pipelineState) {
		currentCmd->bindPipelineState(&pipelineState);
	}

	void VulkanDevice::bindUniforms(const Uniforms& uniforms) {
		currentCmd->bindUniformBuffer(uniforms.size(), uniforms.data());
	}
	
	void VulkanDevice::bindIndexBuffer(HwBuffer* buffer, uint64_t offset, IndexType indexType)
	{		
		currentCmd->bindIndexBuffer(buffer, offset, indexType);
	}

	void VulkanDevice::bindVertexBuffer(uint32_t firstBinding, HwBuffer* buffer, uint64_t offset)
	{
		currentCmd->bindVertexBuffer(firstBinding, buffer, offset);
	}

	void VulkanDevice::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
		currentCmd->draw(vertexCount, instanceCount, firstVertex, firstInstance);
		Stats::drawCall()++;
	}

	void VulkanDevice::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
		currentCmd->drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
		Stats::drawCall()++;
	}

    void VulkanDevice::drawIndirect(HwBuffer* buffer, uint64_t offset, uint32_t drawCount, uint32_t stride) {
		currentCmd->drawIndirect(buffer, offset, drawCount, stride);
	}

    void VulkanDevice::drawIndexedIndirect(HwBuffer* buffer, uint64_t offset, uint32_t drawCount, uint32_t stride) {
		currentCmd->drawIndexedIndirect(buffer, offset, drawCount, stride);
	}

	void VulkanDevice::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
		currentCmd->dispatch(groupCountX, groupCountY, groupCountZ);
	}

    void VulkanDevice::dispatchIndirect(HwBuffer* buffer, uint64_t offset) {
		currentCmd->dispatchIndirect(buffer, offset);
	}
		
	void VulkanDevice::drawPrimitive(HwRenderPrimitive* primitive) {
		VulkanRenderPrimitive* rp = static_cast<VulkanRenderPrimitive*>(primitive);
		vkCmdBindVertexBuffers(currentCmd->cmd, 0, (uint32_t)rp->vertexBuffers.size(), rp->vertexBuffers.data(), rp->bufferOffsets.get());

		if (rp->indexBuffer != nullptr) {
			vkCmdBindIndexBuffer(currentCmd->cmd, rp->indexBuffer, 0, rp->indexType);
			currentCmd->drawIndexed(rp->drawArgs.indexCount, 1, rp->drawArgs.firstIndex, 0, 0);
		} else {
			currentCmd->draw(rp->drawArgs.vertexCount, 1, rp->drawArgs.firstVertex, 0);
		}

		Stats::drawCall()++;
	}

	static void drawBatch1(const CommandBuffer& cmd, const RenderCommand* start, uint32_t count)
	{
		for (uint32_t i = 0; i < count; i++) {
			auto& primitive = start[i];
			cmd.bindPipelineState(&primitive.pipelineState);
			cmd.bindUniforms(primitive.uniforms);
			cmd.drawPrimitive(primitive.renderPrimitive);
		}
	}

	void VulkanDevice::drawBatch(RenderQueue* renderQueue)
	{
		const auto& primitives = renderQueue->getReadCommands();
		if (primitives.size() > 200) {
			drawMultiThreaded(primitives, *currentCmd);
		} else {
			drawBatch1(*currentCmd, primitives.data(), (uint32_t)primitives.size());
		}

		Stats::drawCall() += (uint32_t)primitives.size();
	}

    void drawWork(VulkanDevice* device, const RenderCommand* start, uint32_t count, uint32_t index)
	{
		CommandBuffer& cb = *device->mCmdList[index];
		
		VkCommandBufferInheritanceRenderingInfoKHR info1 =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO_KHR,
			.pNext = nullptr,
			.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT,
			.viewMask = 0,
			.colorAttachmentCount = device->colorAttachmentCount,
			.pColorAttachmentFormats = device->colorAttachmentFormats,
			.depthAttachmentFormat = device->depthAttachmentFormat,
			.stencilAttachmentFormat = device->stencilAttachmentFormat,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		};

		VkCommandBufferInheritanceInfo info =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
			.pNext = &info1,
		};

		cb.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, &info);
		auto& vp = device->mRenderPassInfo.viewport;
		cb.setViewportAndScissor(vp.left, vp.top, vp.width, vp.height);
		cb.resetState();
		drawBatch1(cb, start, count);

		cb.end();
	}

	void VulkanDevice::drawMultiThreaded(const std::vector<RenderCommand>& items, const CommandBuffer& cmd)
	{
		uint32_t itemsPerThread = 200;
		uint32_t threadNum = std::thread::hardware_concurrency(); //16;
		itemsPerThread = (uint32_t)(items.size() + threadNum - 1) / threadNum;
		itemsPerThread = std::max(40u, itemsPerThread);

		mFutures.clear();
		uint32_t start = 0, end = 0;
		
		mSecondCmdBuffers.reserve(threadNum);
		
		for (unsigned i = 0; i < threadNum && start < items.size(); ++i) {
			auto cmdList = mCommandQueues[(int)CommandQueueType::Graphics].getCommandBuffer(1, true);
			mCmdList.push_back(cmdList);
			mSecondCmdBuffers.push_back(cmdList->cmd);
			uint32_t itemCount = std::min(itemsPerThread, (uint32_t)items.size() - start);
			mFutures.emplace_back(std::async(std::launch::async,
				drawWork, this, (const RenderCommand*)&items[start], (uint32_t)itemCount, (uint32_t)i));

			start += itemCount;
		}

		for (auto& f : mFutures) {
			f.get();
		}

		vkCmdExecuteCommands(cmd.cmd, (uint32_t)mSecondCmdBuffers.size(), mSecondCmdBuffers.data());
		mSecondCmdBuffers.clear();

		for (auto cmdList : mCmdList) {
			auto c = cmdList;
			post([=]() {
				mCommandQueues[(int)CommandQueueType::Graphics].ReleaseCommandPool(c->commandPool);
				c->free();
				}, 4);
		}

		mCmdList.clear();

	}

	void VulkanDevice::resourceBarrier(uint32_t barrierCount, const Barrier* pBarriers)
    {
		currentCmd->resourceBarrier(barrierCount, pBarriers);
	}

	void VulkanDevice::submitFrame(int v)
	{
		currentCmd->end();

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &currentCmd->cmd;
		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
		submitCmdBuffers[currentBuffer].push_back(currentCmd);

		VkResult result = swapChain->queuePresent(queue, currentBuffer, semaphores.renderComplete);
		// Recreate the swapchain if it's no longer compatible with the surface (OUT_OF_DATE) or no longer optimal for presentation (SUBOPTIMAL)
		if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) {

			//LOG_ERROR("VK_ERROR_OUT_OF_DATE_KHR");			

			if (result == VK_ERROR_OUT_OF_DATE_KHR) {
				return;
			}
		}
		else {
			VK_CHECK_RESULT(result);
		}
		VK_CHECK_RESULT(vkQueueWaitIdle(queue));
	}

	void VulkanDevice::endFrame(int)
	{
		currentCmd = nullptr;

		HwObject::gc();
		mStagePool->gc();
	}
	
	Dispatcher VulkanDevice::getDispatcher() const noexcept 
	{
		return ConcreteDispatcher<VulkanDevice>::make();
	}

	// explicit instantiation of the Dispatcher
	template class ConcreteDispatcher<VulkanDevice>;
}
