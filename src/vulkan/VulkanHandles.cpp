#include "VulkanHandles.h"
#include "VulkanShader.h"
#include "vulkan/VkFormatHelper.h"
#include "vulkan/VulkanDevice.h"
#include "vulkan/VulkanTools.h"

namespace mygfx {

VulkanRenderPrimitive::VulkanRenderPrimitive(VertexData* geo, const DrawPrimitiveCommand& primitive)
    : HwRenderPrimitive(geo, primitive)
{
    bufferOffsets.reset(new uint64_t[geo->vertexBuffers.size()]);
    vertexSemantics.reset(new VertexAttribute[geo->vertexBuffers.size()]);

    for (int i = 0; i < geo->vertexBuffers.size(); i++) {
        VulkanBuffer* vb = (VulkanBuffer*)geo->vertexBuffers[i].get();
        vertexBuffers.push_back(vb->buffer);
        bufferOffsets[i] = vb->bufferOffset;
        vertexSemantics[i] = (VertexAttribute)vb->extra;
    }

    if (geo->indexBuffer) {
        VulkanBuffer* ib = (VulkanBuffer*)geo->indexBuffer.get();
        indexBuffer = ib->buffer;
        indexType = ib->stride == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
    }
}

VulkanVertexInput::VulkanVertexInput(const FormatList& fmts)
{
    append(fmts, false);
}

VulkanVertexInput::VulkanVertexInput(const FormatList& fmts, const FormatList& fmts1)
{
    append(fmts, false);
    append(fmts, true);
}

void VulkanVertexInput::append(const FormatList& fmts, bool perInstance)
{
    uint32_t binding = (uint32_t)bindingDescriptions.size();
    uint32_t loc = (uint32_t)attributeDescriptions.size();
    uint32_t stride = 0;

    for (auto fmt : fmts) {
        if (fmt == Format::UNDEFINED || fmt == Format::END) {

            if (stride > 0) {
                VkVertexInputBindingDescription2EXT vertexInputBinding {};
                vertexInputBinding.sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT;
                vertexInputBinding.binding = binding;
                vertexInputBinding.inputRate = perInstance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
                vertexInputBinding.stride = stride;
                vertexInputBinding.divisor = 1;
                bindingDescriptions.push_back(vertexInputBinding);
                stride = 0;
                binding++;
            }
            continue;
        }

        auto vkFmt = (VkFormat)TinyImageFormat_ToVkFormat((TinyImageFormat)fmt);
        attributeDescriptions.push_back(
            VkVertexInputAttributeDescription2EXT { VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT, nullptr,
                loc++, binding, vkFmt, stride });

        uint32_t size = getBytesPerPixel(vkFmt);
        stride += size;
    }

    if (stride > 0) {
        VkVertexInputBindingDescription2EXT vertexInputBinding {};
        vertexInputBinding.sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT;
        vertexInputBinding.binding = binding;
        vertexInputBinding.inputRate = perInstance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
        vertexInputBinding.stride = stride;
        vertexInputBinding.divisor = 1;
        bindingDescriptions.push_back(vertexInputBinding);
    }
}

VulkanRenderTarget::VulkanRenderTarget(const RenderTargetDesc& desc)
{
    width = desc.width;
    height = desc.height;

    for (auto& rt : desc.colorAttachments) {
        colorAttachments.emplace_back((VulkanTexture*)rt.get());
    }

    depthAttachment = (VulkanTexture*)desc.depthAttachment.get();
}

VulkanRenderTarget::VulkanRenderTarget(uint32_t w, uint32_t h, bool isSwapchain)
{
    width = w;
    height = h;
    this->isSwapchain = isSwapchain;
}
}