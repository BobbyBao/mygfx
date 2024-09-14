#include "VulkanImageUtility.h"

#include "VkFormatHelper.h"
#include "VulkanTexture.h"

#include <tuple>


namespace mygfx::imgutil {

VkImageAspectFlags getAspectFlags(VkFormat fmt)
{
    if (isDepthOnlyFormat(fmt)) {
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    } else if (isDepthStencilFormat(fmt)) {
        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    } else {
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}

VkFormat toVk(Format fmt)
{
    return (VkFormat)TinyImageFormat_ToVkFormat((TinyImageFormat)fmt);
}

Format fromVk(VkFormat fmt)
{
    return (Format)TinyImageFormat_FromVkFormat((TinyImageFormat_VkFormat)fmt);
}

namespace {

    inline std::tuple<VkAccessFlags, VkAccessFlags, VkPipelineStageFlags, VkPipelineStageFlags,
        VkImageLayout, VkImageLayout>
    getVkTransition(const VulkanLayoutTransition& transition)
    {
        VkAccessFlags srcAccessMask, dstAccessMask;
        VkPipelineStageFlags srcStage, dstStage;

        switch (transition.oldLayout) {
        case VulkanLayout::UNDEFINED:
            srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
            break;
        case VulkanLayout::COLOR_ATTACHMENT:
            srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            break;
        case VulkanLayout::READ_WRITE:
            srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case VulkanLayout::READ_ONLY:
            srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case VulkanLayout::TRANSFER_SRC:
            srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VulkanLayout::TRANSFER_DST:
            srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VulkanLayout::DEPTH_ATTACHMENT:
            srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            break;
        case VulkanLayout::DEPTH_SAMPLER:
            srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case VulkanLayout::COLOR_ATTACHMENT_RESOLVE:
            srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VulkanLayout::PRESENT:
            srcAccessMask = VK_ACCESS_NONE;
            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        }

        switch (transition.newLayout) {
        case VulkanLayout::COLOR_ATTACHMENT:
            dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT
                | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
                | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dstStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
                | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            break;
        case VulkanLayout::READ_WRITE:
            dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case VulkanLayout::READ_ONLY:
            dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
            break;
        case VulkanLayout::TRANSFER_SRC:
            dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VulkanLayout::TRANSFER_DST:
            dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VulkanLayout::DEPTH_ATTACHMENT:
            dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
                | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            break;
        case VulkanLayout::DEPTH_SAMPLER:
            dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            break;
        case VulkanLayout::PRESENT:
        case VulkanLayout::COLOR_ATTACHMENT_RESOLVE:
        case VulkanLayout::UNDEFINED:
            dstAccessMask = 0;
            dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            break;
        }

        return std::make_tuple(srcAccessMask, dstAccessMask, srcStage, dstStage,
            getVkLayout(transition.oldLayout), getVkLayout(transition.newLayout));
    }

} // anonymous namespace

void transitionLayout(VkCommandBuffer cmdbuffer,
    VulkanLayoutTransition transition)
{
    if (transition.oldLayout == transition.newLayout) {
        return;
    }
    auto [srcAccessMask, dstAccessMask, srcStage, dstStage, oldLayout, newLayout]
        = getVkTransition(transition);

    assert(transition.image != VK_NULL_HANDLE && "No image for transition");
    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = srcAccessMask,
        .dstAccessMask = dstAccessMask,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = transition.image,
        .subresourceRange = transition.subresources,
    };
    vkCmdPipelineBarrier(cmdbuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

}

bool operator<(const VkImageSubresourceRange& a, const VkImageSubresourceRange& b)
{
    if (a.aspectMask < b.aspectMask)
        return true;
    if (a.aspectMask > b.aspectMask)
        return false;
    if (a.baseMipLevel < b.baseMipLevel)
        return true;
    if (a.baseMipLevel > b.baseMipLevel)
        return false;
    if (a.levelCount < b.levelCount)
        return true;
    if (a.levelCount > b.levelCount)
        return false;
    if (a.baseArrayLayer < b.baseArrayLayer)
        return true;
    if (a.baseArrayLayer > b.baseArrayLayer)
        return false;
    if (a.layerCount < b.layerCount)
        return true;
    if (a.layerCount > b.layerCount)
        return false;
    return false;
}
