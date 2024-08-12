#pragma once

#include "../GraphicsHandles.h"
#include "../TextureData.h"
#include "VulkanBuffer.h"
#include "VulkanDefs.h"
#include "VulkanTools.h"
#include <fstream>
#include <stdlib.h>
#include <string>
#include <vector>

namespace mygfx {
class VulkanTextureView;

class VulkanTexture : public HwTexture {
public:
    VulkanTexture() = default;
    VulkanTexture(const TextureData& textureData, SamplerInfo samplerInfo);
    VulkanTexture(VkImage image, VkFormat format);
    ~VulkanTexture();

    VkImage image() const { return mImage; }

    VkSampleCountFlagBits samples() const { return mSamples; }

    VulkanTextureView* srv() const { return (VulkanTextureView*)mSRV.get(); }
    VulkanTextureView* rtv() const { return (VulkanTextureView*)mRTV.get(); }
    VulkanTextureView* dsv() const { return (VulkanTextureView*)mDSV.get(); }

    int index() const;
    bool create(const TextureData& textureData);
    void createSRV();

    Ref<VulkanTextureView> createSRV(int mipLevel, const char* name = nullptr);
    Ref<VulkanTextureView> createRTV(int mipLevel = -1, const char* name = nullptr);
    Ref<VulkanTextureView> createDSV(const char* name = nullptr);

    void setSampler(SamplerInfo samplerInfo);
    void setImageLayout(VkImageLayout newImageLayout);
    void setImageLayout(VkImageLayout oldImageLayout, VkImageLayout newImageLayout);
    void setImageLayout(VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange);
    bool copyData(TextureDataProvider* dataProvider);
    void copyTo(VulkanTexture* destTex);

    template <typename T>
    void setData(uint32_t level, const std::span<T>& data)
    {
        setData(level, 0, 0, std::max(1, width >> level), std::max(1, height >> level), data.data(), (uint32_t)(sizeof(T) * data.size()));
    }

    void setData(uint32_t level, int x, int y, uint32_t w, uint32_t h, const std::span<uint8_t>& data);
    void setData(uint32_t level, int x, int y, uint32_t w, uint32_t h, const void* data, uint32_t size);
    void setData(uint32_t level, int x, int y, int z, uint32_t w, uint32_t h, uint32_t depth, const void* data, size_t size);
    void destroy();

    bool isSwapchain = false;
    VkFormat vkFormat;
    VkImageUsageFlags usage;
private:
    bool initFromData(const TextureData& textureData);
    void initRenderTarget(const char* name = nullptr, VkImageCreateFlags flags = (VkImageCreateFlags)0);
    void initDepthStencil(const char* name = nullptr);
    void createImage(VkImageCreateInfo* pCreateInfo, const char* name = nullptr);
    void createImage(const char* pName, bool useSRGB = false);

    VmaAllocation mImageAlloc = VK_NULL_HANDLE;
    VkImage mImage = VK_NULL_HANDLE;
    SamplerHandle mSampler;
    VkImageLayout mImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkSampleCountFlagBits mSamples = VK_SAMPLE_COUNT_1_BIT;
    Vector<Ref<VulkanTextureView>> mSRVs;
    Vector<Ref<VulkanTextureView>> mRTVs;
};

} // namespace mygfx
