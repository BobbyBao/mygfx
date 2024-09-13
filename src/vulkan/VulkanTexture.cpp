#include "VulkanTexture.h"
#include "VkFormatHelper.h"
#include "VulkanDevice.h"
#include "VulkanImageUtility.h"
#include "VulkanTextureView.h"

namespace mygfx {
VulkanTexture::VulkanTexture(const TextureData& textureData, SamplerInfo samplerInfo)
{
    setSampler(samplerInfo);
    create(textureData);
}

VulkanTexture::VulkanTexture(VkImage image, VkFormat format)
{
    mImage = image;
    vkFormat = format;
    mRTV = createRTV(0);
    isSwapchain = true;

    initState(ResourceState::PRESENT);
}

VulkanTexture::~VulkanTexture()
{
    destroy();
}

void VulkanTexture::destroy()
{
    if (!isSwapchain && mImage != VK_NULL_HANDLE) {
        auto img = mImage;
        auto mem = mImageAlloc;
        vmaDestroyImage(gfx().getVmaAllocator(), img, mem);

        mImage = VK_NULL_HANDLE;
        mImageAlloc = nullptr;
    }

    mSRV.reset();
    mRTV.reset();
    mDSV.reset();
}

bool VulkanTexture::create(const TextureData& textureData)
{
    width = std::max((uint16_t)1, textureData.width);
    height = std::max((uint16_t)1, textureData.height);
    depth = std::max((uint16_t)1, textureData.depth);

    layerCount = textureData.layerCount;
    faceCount = textureData.faceCount;
    mipLevels = textureData.mipMapCount;
    format = textureData.format;
    samplerType = textureData.samplerType;

    if (samplerType == SamplerType::COUNT) {
        if (depth > 1) {
            samplerType = SamplerType::SAMPLER_3D;
        } else if (layerCount > 1) {
            if (faceCount > 1) {
                samplerType = SamplerType::SAMPLER_CUBE_ARRAY;
            } else {
                samplerType = (height == 1 ? SamplerType::SAMPLER_1D_ARRAY : SamplerType::SAMPLER_2D_ARRAY);
            }

        } else {

            if (faceCount > 1) {
                samplerType = SamplerType::SAMPLER_CUBE;
            } else {
                samplerType = (height == 1 ? SamplerType::SAMPLER_1D : SamplerType::SAMPLER_2D);
            }
        }
    }

    vkFormat = imgutil::toVk(textureData.format);
    mSamples = (VkSampleCountFlagBits)textureData.sampleCount;
    usage = (VkImageUsageFlags)textureData.usage;

    if (any(textureData.usage & TextureUsage::DEPTH_STENCIL_ATTACHMENT)) {

        initDepthStencil(textureData.name.c_str());

        if (any(textureData.usage & TextureUsage::SAMPLED))
            initState(ResourceState::SHADER_RESOURCE);
        else
            initState(ResourceState::COMMON_RESOURCE);

    } else if (any(textureData.usage & TextureUsage::COLOR_ATTACHMENT)) {

        initRenderTarget(textureData.name.c_str());

        if (any(textureData.usage & TextureUsage::SAMPLED))
            initState(ResourceState::SHADER_RESOURCE);
        else
            initState(ResourceState::COMMON_RESOURCE);

    } else {

        initState(ResourceState::COPY_DEST);

        initFromData(textureData);

        initSubResourceCount(mipLevels * layerCount * faceCount);
    }

    return true;
}

bool VulkanTexture::initFromData(const TextureData& textureData)
{
    assert(!mImage);

    createImage(textureData.name.c_str(), isSrgb(vkFormat));

    if (textureData.data() == nullptr) {
        createSRV();
        return true;
    }

    VkFlags aspect = imgutil::getAspectFlags(vkFormat);

    // Upload Image
    {
        VkImageMemoryBarrier copy_barrier = {};
        copy_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        copy_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        copy_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        copy_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        copy_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        copy_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        copy_barrier.image = mImage;
        copy_barrier.subresourceRange.aspectMask = aspect;
        copy_barrier.subresourceRange.baseMipLevel = 0;
        copy_barrier.subresourceRange.levelCount = mipLevels;
        copy_barrier.subresourceRange.layerCount = layerCount;

        gfx().getUploadHeap().AddPreBarrier(copy_barrier);
    }

    // compute pixel size
    //
    uint32_t bytePP = getBitsPerPixel(format) / 8;
    uint8_t* pixels = NULL;
    uint64_t UplHeapSize = width * height * bytePP;
    pixels = gfx().getUploadHeap().Suballocate(UplHeapSize, 512);
    assert(pixels != NULL);

    std::memcpy(pixels, textureData.data(), width * height * bytePP);

    uint32_t offset = uint32_t(pixels - gfx().getUploadHeap().BasePtr());
    VkBufferImageCopy region = {};
    region.bufferOffset = offset;
    region.imageSubresource.aspectMask = aspect;
    region.imageSubresource.layerCount = 1;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.mipLevel = 0;
    region.imageExtent.width = width;
    region.imageExtent.height = height;
    region.imageExtent.depth = 1;

    gfx().getUploadHeap().AddCopy(mImage, region);

    // prepare to shader read
    //
    {
        VkImageMemoryBarrier use_barrier = {};
        use_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        use_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        use_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        use_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        use_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        use_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        use_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        use_barrier.image = mImage;
        use_barrier.subresourceRange.aspectMask = aspect;
        use_barrier.subresourceRange.levelCount = mipLevels;
        use_barrier.subresourceRange.layerCount = layerCount;

        gfx().getUploadHeap().AddPostBarrier(use_barrier);
    }

    gfx().getUploadHeap().FlushAndFinish();

    if (mSRV == nullptr) {
        createSRV();
    }

    setCurrentResourceState(ResourceState::SHADER_RESOURCE);

    return true;
}

void VulkanTexture::createImage(const char* pName, bool useSRGB)
{
    VkImageCreateInfo info = {};

    VkImageFormatListCreateInfo formatListInfo = {};
    if (useSRGB && ((usage & VK_IMAGE_USAGE_STORAGE_BIT) != 0)) {
        // the storage bit is not supported for srgb formats
        // we can still use the srgb format on an image view if the access is read-only
        // for write access, we need to use an image view with unorm format
        // this is ok as srgb and unorm formats are compatible with each other

        formatListInfo.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO;
        formatListInfo.viewFormatCount = 2;
        VkFormat list[2];
        list[0] = vkFormat;
        list[1] = setFormatGamma(vkFormat, useSRGB);
        formatListInfo.pViewFormats = list;

        info.pNext = &formatListInfo;
    } else {
        // assert(!isSrgb(vkFormat));
        vkFormat = setFormatGamma(vkFormat, useSRGB);
    }

    // Create the Image:
    {
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.imageType = imgutil::getImageType(samplerType);
        info.format = vkFormat;
        info.extent.width = width;
        info.extent.height = height;
        info.extent.depth = depth;
        info.mipLevels = mipLevels;
        info.arrayLayers = layerCount * faceCount;
        if (faceCount == 6)
            info.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | usage;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        createImage(&info, pName);
    }
}

void VulkanTexture::initRenderTarget(const char* name, VkImageCreateFlags flags)
{
    VkImageCreateInfo image_info = {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.pNext = NULL;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = vkFormat;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = mipLevels;
    image_info.arrayLayers = layerCount * faceCount;
    image_info.samples = mSamples;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.queueFamilyIndexCount = 0;
    image_info.pQueueFamilyIndices = NULL;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.usage = usage;
    image_info.flags = flags;
    if (faceCount == 6)
        image_info.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL; // VK_IMAGE_TILING_LINEAR should never be used and will never be faster

    createImage(&image_info, name);

    mRTV = createRTV();

    if (usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
        createSRV();
    }

    setImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

void VulkanTexture::initDepthStencil(const char* name)
{
    VkImageCreateInfo image_info = {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.pNext = NULL;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = imgutil::toVk(format);
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = mipLevels;
    image_info.arrayLayers = layerCount * faceCount;
    image_info.samples = mSamples;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.queueFamilyIndexCount = 0;
    image_info.pQueueFamilyIndices = NULL;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | usage;
    image_info.flags = 0;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL; // VK_IMAGE_TILING_LINEAR should never be used and will never be faster

    createImage(&image_info, name);

    mDSV = createRTV();

    if (usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
        createSRV();
    }
}

void VulkanTexture::createImage(VkImageCreateInfo* pCreateInfo, const char* name)
{
    mSamples = pCreateInfo->samples;

    VmaAllocationCreateInfo imageAllocCreateInfo = {};
    imageAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    imageAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
    imageAllocCreateInfo.pUserData = (void*)name;
    VmaAllocationInfo gpuImageAllocInfo = {};
    VkResult res = vmaCreateImage(gfx().getVmaAllocator(), pCreateInfo, &imageAllocCreateInfo, &mImage, &mImageAlloc, &gpuImageAllocInfo);
    assert(res == VK_SUCCESS);
    if (name)
        gfx().setResourceName(VK_OBJECT_TYPE_IMAGE, (uint64_t)mImage, name);
}

Ref<VulkanTextureView> VulkanTexture::createSRV(int mipLevel, const char* name)
{
    VkImageViewCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.image = mImage;
    info.viewType = imgutil::getViewType(samplerType);
    if (faceCount > 1) {
        info.subresourceRange.layerCount = layerCount * faceCount;
    } else {
        info.subresourceRange.layerCount = layerCount;
    }

    info.format = vkFormat;
    info.subresourceRange.aspectMask = imgutil::getAspectFlags(vkFormat);

    if (mipLevel == -1) {
        info.subresourceRange.baseMipLevel = 0;
        info.subresourceRange.levelCount = mipLevels;
    } else {
        info.subresourceRange.baseMipLevel = mipLevel;
        info.subresourceRange.levelCount = 1;
    }

    info.subresourceRange.baseArrayLayer = 0;
    auto srv = makeShared<VulkanTextureView>(info, mSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, name);
    mSRVs.push_back(srv);
    return srv;
}

Ref<VulkanTextureView> VulkanTexture::createRTV(int mipLevel, const char* name)
{
    VkImageViewCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.image = mImage;
    info.viewType = VK_IMAGE_VIEW_TYPE_2D;

    info.format = vkFormat;
    info.components = {
        VK_COMPONENT_SWIZZLE_R,
        VK_COMPONENT_SWIZZLE_G,
        VK_COMPONENT_SWIZZLE_B,
        VK_COMPONENT_SWIZZLE_A
    };

    info.subresourceRange.aspectMask = imgutil::getAspectFlags(info.format);

    std::string ResourceName = name ? name : "";

    info.subresourceRange.baseArrayLayer = 0;
    if (layerCount > 1 && faceCount > 1) {
        info.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
    } else if (layerCount > 1) {
        info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    } else if (faceCount > 1) {
        info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    }

    if (mipLevel == -1) {
        info.subresourceRange.baseMipLevel = 0;
        info.subresourceRange.levelCount = mipLevels;
        info.subresourceRange.layerCount = layerCount * faceCount;
    } else {
        info.subresourceRange.baseMipLevel = mipLevel;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.layerCount = 1;
        ResourceName += std::to_string(mipLevel);
    }

    auto rtv = makeShared<VulkanTextureView>(info, ResourceName.c_str());
    mRTVs.push_back(rtv);
    return rtv;
}

Ref<VulkanTextureView> VulkanTexture::createDSV(const char* name)
{
    VkImageViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.pNext = NULL;
    view_info.image = mImage;
    view_info.format = vkFormat;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;

    if (view_info.format == VK_FORMAT_D16_UNORM_S8_UINT || view_info.format == VK_FORMAT_D24_UNORM_S8_UINT || view_info.format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
        view_info.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    return makeShared<VulkanTextureView>(view_info, name);
}

void VulkanTexture::createSRV()
{
    mSRV = createSRV(-1);
}

int VulkanTexture::index() const
{
    return mSRV->index();
}

void VulkanTexture::setSampler(SamplerInfo samplerInfo)
{
    mSampler = gfx().createSampler(samplerInfo);
}

void VulkanTexture::setImageLayout(VkImageLayout newImageLayout)
{
    VkImageSubresourceRange subresourceRange;
    subresourceRange.aspectMask = imgutil::getAspectFlags(vkFormat);
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = mipLevels;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = layerCount;
    setImageLayout(mImageLayout, newImageLayout, subresourceRange);
}

void VulkanTexture::setImageLayout(VkImageLayout oldImageLayout, VkImageLayout newImageLayout)
{
    VkImageSubresourceRange subresourceRange;
    subresourceRange.aspectMask = imgutil::getAspectFlags(vkFormat);
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = mipLevels;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = layerCount;
    setImageLayout(oldImageLayout, newImageLayout, subresourceRange);
}

void VulkanTexture::setImageLayout(VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange)
{
    gfx().executeCommand(CommandQueueType::Copy, [&](const auto& commandBuffer) {
        commandBuffer.setImageLayout(mImage, oldImageLayout, newImageLayout, subresourceRange);
    });

    mImageLayout = newImageLayout;
}

bool VulkanTexture::copyData(TextureDataProvider* dataProvider)
{
    // Upload Image
    {
        VkImageMemoryBarrier copy_barrier = {};
        copy_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        copy_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        copy_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        copy_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        copy_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        copy_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        copy_barrier.image = mImage;
        copy_barrier.subresourceRange.aspectMask = imgutil::getAspectFlags(vkFormat);
        copy_barrier.subresourceRange.baseMipLevel = 0;
        copy_barrier.subresourceRange.levelCount = mipLevels;
        copy_barrier.subresourceRange.layerCount = layerCount * faceCount;
        gfx().getUploadHeap().AddPreBarrier(copy_barrier);
    }

    // compute pixel size
    //

    uint32_t bitsPerPixel = getBitsPerPixel(format);

    for (uint32_t a = 0; a < layerCount; a++) {
        for (uint32_t face = 0; face < faceCount; face++) {
            // copy all the mip slices into the offsets specified by the footprint structure
            //
            for (uint32_t mip = 0; mip < mipLevels; mip++) {
                uint32_t dwWidth = std::max<uint32_t>(width >> mip, 1);
                uint32_t dwHeight = std::max<uint32_t>(height >> mip, 1);
                uint32_t dwDepth = std::max<uint32_t>(depth >> mip, 1);

                uint64_t UplHeapSize = (dwWidth * dwHeight * bitsPerPixel) / 8;
                uint8_t* pixels = gfx().getUploadHeap().BeginSuballocate(UplHeapSize, 512);

                if (pixels == NULL) {
                    // oh! We ran out of mem in the upload heap, flush it and try allocating mem from it again
                    gfx().getUploadHeap().FlushAndFinish();
                    pixels = gfx().getUploadHeap().Suballocate(UplHeapSize, 512);
                    assert(pixels != NULL);
                }

                dataProvider->copyPixels(pixels, (uint32_t)UplHeapSize, dwWidth, dwHeight, a, face, mip);

                gfx().getUploadHeap().EndSuballocate();

                uint32_t offset = uint32_t(pixels - gfx().getUploadHeap().BasePtr());
                {
                    VkBufferImageCopy region = {};
                    region.bufferOffset = offset;
                    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    region.imageSubresource.layerCount = 1;
                    region.imageSubresource.baseArrayLayer = a * faceCount + face;
                    region.imageSubresource.mipLevel = mip;
                    region.imageExtent.width = dwWidth;
                    region.imageExtent.height = dwHeight;
                    region.imageExtent.depth = 1;
                    gfx().getUploadHeap().AddCopy(mImage, region);
                }
            }
        }
    }

    // prepare to shader read
    //
    {
        VkImageMemoryBarrier use_barrier = {};
        use_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        use_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        use_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        use_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        use_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        use_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        use_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        use_barrier.image = mImage;
        use_barrier.subresourceRange.aspectMask = imgutil::getAspectFlags(vkFormat);
        use_barrier.subresourceRange.levelCount = mipLevels;
        use_barrier.subresourceRange.layerCount = layerCount * faceCount;
        gfx().getUploadHeap().AddPostBarrier(use_barrier);
    }

    gfx().getUploadHeap().finish();

    if (mSRV == nullptr) {
        createSRV();
    }
    return true;
}

void VulkanTexture::copyTo(VulkanTexture* destTex) {

}

void VulkanTexture::setData(uint32_t level, int x, int y, uint32_t w, uint32_t h, const std::span<uint8_t>& data)
{
    auto pixels = gfx().getUploadHeap().Suballocate(data.size(), 64);
    std::memcpy(pixels, data.data(), data.size());

    uint32_t offset = uint32_t(pixels - gfx().getUploadHeap().BasePtr());
    VkBufferImageCopy bufferCopyRegion {
        .bufferOffset = offset,
        .bufferImageHeight = 0,
        .imageSubresource = VkImageSubresourceLayers {
            .aspectMask = imgutil::getAspectFlags(vkFormat),
            .mipLevel = level,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },

        .imageOffset = VkOffset3D { x, y, 0 },
        .imageExtent = VkExtent3D { w, h, 1 },
    };

    gfx().getUploadHeap().AddCopy(mImage, bufferCopyRegion);

    // The sub resource range describes the regions of the image we will be transition
    VkImageSubresourceRange subresourceRange {
        imgutil::getAspectFlags(vkFormat), 0, 1, 0, 1
    };

    VkImageMemoryBarrier copy_barrier {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        nullptr,
        VK_ACCESS_NONE,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        0,
        0,
        mImage,
        subresourceRange,
    };

    gfx().getUploadHeap().AddPreBarrier(copy_barrier);

    VkImageMemoryBarrier use_barrier {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        nullptr,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_SHADER_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        0,
        0,
        mImage,
        subresourceRange,
    };

    gfx().getUploadHeap().AddPostBarrier(use_barrier);
    gfx().getUploadHeap().finish();
}

void VulkanTexture::setData(uint32_t level, int x, int y, int z, uint32_t w, uint32_t h, uint32_t depth, const void* data, size_t size)
{
    auto pixels = gfx().getUploadHeap().Suballocate(size, 64);

    std::memcpy(pixels, data, size);

    uint32_t offset = uint32_t(pixels - gfx().getUploadHeap().BasePtr());
    VkBufferImageCopy bufferCopyRegion {
        .bufferOffset = offset,
        .bufferImageHeight = 0,
        .imageSubresource = VkImageSubresourceLayers {
            .aspectMask = imgutil::getAspectFlags(vkFormat),
            .mipLevel = level,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },

        .imageOffset = VkOffset3D { x, y, z },
        .imageExtent = VkExtent3D { w, h, depth },
    };

    // The sub resource range describes the regions of the image we will be transition
    VkImageSubresourceRange transitionRange {
        imgutil::getAspectFlags(vkFormat), 0, 1, 0, 1
    };

    if (samplerType == SamplerType::SAMPLER_2D_ARRAY || samplerType == SamplerType::SAMPLER_CUBE || samplerType == SamplerType::SAMPLER_CUBE_ARRAY) {
        bufferCopyRegion.imageOffset.z = 0;
        bufferCopyRegion.imageExtent.depth = 1;
        bufferCopyRegion.imageSubresource.baseArrayLayer = z;
        bufferCopyRegion.imageSubresource.layerCount = depth;
        transitionRange.baseArrayLayer = z;
        transitionRange.layerCount = depth;
    }

    gfx().getUploadHeap().AddCopy(mImage, bufferCopyRegion);

    VkImageMemoryBarrier copy_barrier {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        nullptr,
        VK_ACCESS_NONE,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        0,
        0,
        mImage,
        transitionRange,
    };

    gfx().getUploadHeap().AddPreBarrier(copy_barrier);

    VkImageMemoryBarrier use_barrier {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        nullptr,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_SHADER_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        0,
        0,
        mImage,
        transitionRange,
    };

    gfx().getUploadHeap().AddPostBarrier(use_barrier);
    gfx().getUploadHeap().finish();
}

void VulkanTexture::setData(uint32_t level, int x, int y, uint32_t w, uint32_t h, const void* data, uint32_t size)
{
    setData(level, x, y, w, h, std::span<uint8_t>((uint8_t*)data, (int)size));
}

}
