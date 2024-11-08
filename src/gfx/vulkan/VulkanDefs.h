#pragma once

#include "volk/volk.h"

#include <array>

#include <vma/vk_mem_alloc.h>

#ifdef VK_EXT_shader_object
#define HAS_SHADER_OBJECT_EXT 0
#else
#define HAS_SHADER_OBJECT_EXT 0
#endif

namespace mygfx {

static constexpr uint32_t DESCRIPTOR_TYPE_COUNT = 16;
static constexpr uint32_t VARIABLE_DESC_COUNT = 4096;

struct DescriptorResourceCounts : public std::array<uint32_t, DESCRIPTOR_TYPE_COUNT> {

    bool operator==(const DescriptorResourceCounts& other)
    {
        return std::memcmp(data(), other.data(), sizeof(uint32_t) * DESCRIPTOR_TYPE_COUNT) == 0;
    }
};

struct AttachmentFormats {
    uint32_t colorAttachmentCount = 0;
    VkFormat attachmentFormats[10] = { VK_FORMAT_UNDEFINED };
    VkFormat depthAttachmentFormat() const { return attachmentFormats[8]; }
    VkFormat stencilAttachmentFormat() const { return attachmentFormats[9]; }
    VkFormat& depthAttachmentFormat() { return attachmentFormats[8]; }
    VkFormat& stencilAttachmentFormat() { return attachmentFormats[9]; }
    size_t getHash() const { return mHash; }

    void reset()
    {
        std::memset(&attachmentFormats[0], 0, sizeof(VkFormat) * 10);
    }

    void calculateHash()
    {
        mHash = std::_Hash_array_representation(attachmentFormats, 10);
    }

private:
    size_t mHash = 0;
};

#if HAS_SHADER_OBJECT_EXT
extern PFN_vkCreateShadersEXT g_vkCreateShadersEXT;
extern PFN_vkDestroyShaderEXT g_vkDestroyShaderEXT;
extern PFN_vkCmdBindShadersEXT g_vkCmdBindShadersEXT;
extern PFN_vkGetShaderBinaryDataEXT g_vkGetShaderBinaryDataEXT;
#endif

// VK_EXT_shader_objects requires render passes to be dynamic
extern PFN_vkCmdBeginRenderingKHR g_vkCmdBeginRenderingKHR;
extern PFN_vkCmdEndRenderingKHR g_vkCmdEndRenderingKHR;

// With VK_EXT_shader_object pipeline state must be set at command buffer creation using these functions
extern PFN_vkCmdSetAlphaToCoverageEnableEXT g_vkCmdSetAlphaToCoverageEnableEXT;
extern PFN_vkCmdSetColorBlendEnableEXT g_vkCmdSetColorBlendEnableEXT;
extern PFN_vkCmdSetColorBlendEquationEXT g_vkCmdSetColorBlendEquationEXT;
extern PFN_vkCmdSetColorWriteMaskEXT g_vkCmdSetColorWriteMaskEXT;
extern PFN_vkCmdSetCullModeEXT g_vkCmdSetCullModeEXT;
extern PFN_vkCmdSetDepthBiasEnableEXT g_vkCmdSetDepthBiasEnableEXT;
extern PFN_vkCmdSetDepthCompareOpEXT g_vkCmdSetDepthCompareOpEXT;
extern PFN_vkCmdSetDepthTestEnableEXT g_vkCmdSetDepthTestEnableEXT;
extern PFN_vkCmdSetDepthWriteEnableEXT g_vkCmdSetDepthWriteEnableEXT;
extern PFN_vkCmdSetFrontFaceEXT g_vkCmdSetFrontFaceEXT;
extern PFN_vkCmdSetPolygonModeEXT g_vkCmdSetPolygonModeEXT;
extern PFN_vkCmdSetPrimitiveRestartEnableEXT g_vkCmdSetPrimitiveRestartEnableEXT;
extern PFN_vkCmdSetPrimitiveTopologyEXT g_vkCmdSetPrimitiveTopologyEXT;
extern PFN_vkCmdSetRasterizationSamplesEXT g_vkCmdSetRasterizationSamplesEXT;
extern PFN_vkCmdSetRasterizerDiscardEnableEXT g_vkCmdSetRasterizerDiscardEnableEXT;
extern PFN_vkCmdSetSampleMaskEXT g_vkCmdSetSampleMaskEXT;
extern PFN_vkCmdSetScissorWithCountEXT g_vkCmdSetScissorWithCountEXT;
extern PFN_vkCmdSetStencilTestEnableEXT g_vkCmdSetStencilTestEnableEXT;
extern PFN_vkCmdSetViewportWithCountEXT g_vkCmdSetViewportWithCountEXT;

// VK_EXT_vertex_input_dynamic_state
extern PFN_vkCmdSetVertexInputEXT g_vkCmdSetVertexInputEXT;
}