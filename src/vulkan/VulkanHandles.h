#pragma once
#include "../GraphicsHandles.h"
#include "../PipelineState.h"
#include "VulkanDefs.h"

#include <vector>

namespace mygfx {

class VulkanRenderPrimitive : public HwRenderPrimitive {
public:
    VulkanRenderPrimitive(VertexData* geo, const DrawPrimitiveCommand& primitive);
    std::vector<VkBuffer> vertexBuffers;
    std::unique_ptr<uint64_t[]> bufferOffsets = nullptr;
    std::unique_ptr<VertexAttribute[]> vertexSemantics = nullptr;
    ;
    VkBuffer indexBuffer = nullptr;
    VkIndexType indexType;
};

class VulkanVertexInput : public HwVertexInput {
public:
    VulkanVertexInput(const FormatList& fmts);
    VulkanVertexInput(const FormatList& fmts, const FormatList& fmts1);

    void append(const FormatList& fmts, bool perInstance);

    inline bool empty() const { return attributeDescriptions.empty(); }

    std::vector<VkVertexInputBindingDescription2EXT> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription2EXT> attributeDescriptions;

private:
};

class VulkanTexture;

class VulkanRenderTarget : public HwRenderTarget {
public:
    VulkanRenderTarget(const RenderTargetDesc& desc);
    VulkanRenderTarget(uint32_t w, uint32_t h, bool isSwapchain = false);

    uint32_t numAttachments() const { return isSwapchain ? 1 : (uint32_t)colorAttachments.size(); }

    std::vector<Ref<VulkanTexture>> colorAttachments;
    Ref<VulkanTexture> depthAttachment = nullptr;
};

}
