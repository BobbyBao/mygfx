#include "VulkanSwapChain.h"
#include "VulkanDevice.h"
#include "VulkanHandles.h"
#include "VulkanImageUtility.h"
#include "VulkanTexture.h"

namespace mygfx {

VulkanSwapChain::VulkanSwapChain() = default;

VulkanSwapChain::VulkanSwapChain(const SwapChainDesc& desc)
{
    this->desc = desc;    
    initSurface();
}

VulkanSwapChain::~VulkanSwapChain()
{
    cleanup();
}

void VulkanSwapChain::initSurface()
{
    if (desc.surface) {
        mSurface = (VkSurfaceKHR)desc.surface;
        selectSurfaceFormat();
        return;
    }

#if defined(VK_USE_PLATFORM_WIN32_KHR)
    initSurface(desc.windowInstance, desc.window);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    initSurface((ANativeWindow*)desc.window);
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    initSurface((IDirectFB*)desc.windowInstance, (IDirectFBSurface*)desc.window);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    initSurface((wl_display*)desc.windowInstance, (wl_surface*)desc.window);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    initSurface((xcb_connection_t*)desc.windowInstance, (xcb_window_t)desc.window);
#elif (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
    initSurface(desc.window);
#endif

    selectSurfaceFormat();
}

void VulkanSwapChain::selectSurfaceFormat()
{
    // Get available queue family properties
    uint32_t queueCount;
    vkGetPhysicalDeviceQueueFamilyProperties(gfx().physicalDevice, &queueCount, NULL);
    assert(queueCount >= 1);

    std::vector<VkQueueFamilyProperties> queueProps(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(gfx().physicalDevice, &queueCount, queueProps.data());

    // Iterate over each queue to learn whether it supports presenting:
    // Find a queue with present support
    // Will be used to present the swap chain images to the windowing system
    std::vector<VkBool32> supportsPresent(queueCount);
    for (uint32_t i = 0; i < queueCount; i++) {
        vkGetPhysicalDeviceSurfaceSupportKHR(gfx().physicalDevice, i, mSurface, &supportsPresent[i]);
    }

    // Search for a graphics and a present queue in the array of queue
    // families, try to find one that supports both
    uint32_t graphicsQueueNodeIndex = UINT32_MAX;
    uint32_t presentQueueNodeIndex = UINT32_MAX;
    for (uint32_t i = 0; i < queueCount; i++) {
        if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
            if (graphicsQueueNodeIndex == UINT32_MAX) {
                graphicsQueueNodeIndex = i;
            }

            if (supportsPresent[i] == VK_TRUE) {
                graphicsQueueNodeIndex = i;
                presentQueueNodeIndex = i;
                break;
            }
        }
    }
    if (presentQueueNodeIndex == UINT32_MAX) {
        // If there's no queue that supports both present and graphics
        // try to find a separate present queue
        for (uint32_t i = 0; i < queueCount; ++i) {
            if (supportsPresent[i] == VK_TRUE) {
                presentQueueNodeIndex = i;
                break;
            }
        }
    }

    // Exit if either a graphics or a presenting queue hasn't been found
    if (graphicsQueueNodeIndex == UINT32_MAX || presentQueueNodeIndex == UINT32_MAX) {
        tools::exitFatal("Could not find a graphics and/or presenting queue!", -1);
    }

    if (graphicsQueueNodeIndex != presentQueueNodeIndex) {
        tools::exitFatal("Separate graphics and presenting queues are not supported yet!", -1);
    }

    queueNodeIndex = graphicsQueueNodeIndex;

    // Get list of supported surface formats
    uint32_t formatCount;
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(gfx().physicalDevice, mSurface, &formatCount, NULL));
    assert(formatCount > 0);

    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(gfx().physicalDevice, mSurface, &formatCount, surfaceFormats.data()));

    // We want to get a format that best suits our needs, so we try to get one from a set of preferred formats
    // Initialize the format to the first one returned by the implementation in case we can't find one of the preffered formats
    VkSurfaceFormatKHR selectedFormat = surfaceFormats[0];
    std::vector<VkFormat> preferredImageFormats = {
        // VK_FORMAT_B8G8R8A8_UNORM,
        VK_FORMAT_B8G8R8A8_SRGB,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_FORMAT_A8B8G8R8_UNORM_PACK32
    };

    for (auto& availableFormat : surfaceFormats) {
        if (std::find(preferredImageFormats.begin(), preferredImageFormats.end(), availableFormat.format) != preferredImageFormats.end()) {
            selectedFormat = availableFormat;
            break;
        }
    }

    colorFormat = selectedFormat.format;
    colorSpace = selectedFormat.colorSpace;
}

void VulkanSwapChain::recreate(const SwapChainDesc* pDesc)
{
    if (pDesc) {
        this->desc = *pDesc;
    }

    create(&this->desc.width, &this->desc.height, this->desc.vsync, this->desc.fullscreen);

    setupDepthStencil();
}

/** @brief Creates the platform specific surface abstraction of the native platform window used for presentation */
#if defined(VK_USE_PLATFORM_WIN32_KHR)
bool VulkanSwapChain::initSurface(void* platformHandle, void* platformWindow)
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
bool VulkanSwapChain::initSurface(ANativeWindow* window)
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
bool VulkanSwapChain::initSurface(IDirectFB* dfb, IDirectFBSurface* window)
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
bool VulkanSwapChain::initSurface(wl_display* display, wl_surface* window)
#elif defined(VK_USE_PLATFORM_XCB_KHR)
bool VulkanSwapChain::initSurface(xcb_connection_t* connection, xcb_window_t window)
#elif (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
bool VulkanSwapChain::initSurface(void* view)
#elif defined(VK_USE_PLATFORM_METAL_EXT)
bool VulkanSwapChain::initSurface(CAMetalLayer* metalLayer)
#elif (defined(_DIRECT2DISPLAY) || defined(VK_USE_PLATFORM_HEADLESS_EXT))
bool VulkanSwapChain::initSurface(uint32_t width, uint32_t height)
#elif defined(VK_USE_PLATFORM_SCREEN_QNX)
bool VulkanSwapChain::initSurface(screen_context_t screen_context, screen_window_t screen_window)
#endif
{
    VkResult err = VK_SUCCESS;
    auto instance = gfx().instance;
    // Create the os-specific surface
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hinstance = (HINSTANCE)platformHandle;
    surfaceCreateInfo.hwnd = (HWND)platformWindow;
    err = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &mSurface);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.window = window;
    err = vkCreateAndroidSurfaceKHR(instance, &surfaceCreateInfo, NULL, &mSurface);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
    VkIOSSurfaceCreateInfoMVK surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK;
    surfaceCreateInfo.pNext = NULL;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.pView = view;
    err = vkCreateIOSSurfaceMVK(instance, &surfaceCreateInfo, nullptr, &mSurface);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
    VkMacOSSurfaceCreateInfoMVK surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
    surfaceCreateInfo.pNext = NULL;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.pView = view;
    err = vkCreateMacOSSurfaceMVK(instance, &surfaceCreateInfo, NULL, &mSurface);
#elif defined(_DIRECT2DISPLAY)
    createDirect2DisplaySurface(width, height);
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    VkDirectFBSurfaceCreateInfoEXT surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_DIRECTFB_SURFACE_CREATE_INFO_EXT;
    surfaceCreateInfo.dfb = dfb;
    surfaceCreateInfo.surface = window;
    err = vkCreateDirectFBSurfaceEXT(instance, &surfaceCreateInfo, nullptr, &mSurface);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.display = display;
    surfaceCreateInfo.surface = window;
    err = vkCreateWaylandSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &mSurface);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.connection = connection;
    surfaceCreateInfo.window = window;
    err = vkCreateXcbSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &mSurface);
#elif defined(VK_USE_PLATFORM_HEADLESS_EXT)
    VkHeadlessSurfaceCreateInfoEXT surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT;
    PFN_vkCreateHeadlessSurfaceEXT fpCreateHeadlessSurfaceEXT = (PFN_vkCreateHeadlessSurfaceEXT)vkGetInstanceProcAddr(instance, "vkCreateHeadlessSurfaceEXT");
    if (!fpCreateHeadlessSurfaceEXT) {
        tools::exitFatal("Could not fetch function pointer for the headless extension!", -1);
    }
    err = fpCreateHeadlessSurfaceEXT(instance, &surfaceCreateInfo, nullptr, &mSurface);
#elif defined(VK_USE_PLATFORM_SCREEN_QNX)
    VkScreenSurfaceCreateInfoQNX surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_SCREEN_SURFACE_CREATE_INFO_QNX;
    surfaceCreateInfo.pNext = NULL;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.context = screen_context;
    surfaceCreateInfo.window = screen_window;
    err = vkCreateScreenSurfaceQNX(instance, &surfaceCreateInfo, NULL, &mSurface);
#endif

    if (err != VK_SUCCESS) {
        tools::exitFatal("Could not create surface!", err);
        return false;
    }
    return true;
}

/**
 * Create the swapchain and get its images with given width and height
 *
 * @param width Pointer to the width of the swapchain (may be adjusted to fit the requirements of the swapchain)
 * @param height Pointer to the height of the swapchain (may be adjusted to fit the requirements of the swapchain)
 * @param vsync (Optional) Can be used to force vsync-ed rendering (by using VK_PRESENT_MODE_FIFO_KHR as presentation mode)
 */
void VulkanSwapChain::create(uint32_t* width, uint32_t* height, bool vsync, bool fullscreen)
{
    // Store the current swap chain handle so we can use it later on to ease up recreation
    VkSwapchainKHR oldSwapchain = swapChain;

    // Get physical device surface properties and formats
    VkSurfaceCapabilitiesKHR surfCaps;
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gfx().physicalDevice, mSurface, &surfCaps));

    // Get available present modes
    uint32_t presentModeCount;
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(gfx().physicalDevice, mSurface, &presentModeCount, NULL));
    assert(presentModeCount > 0);

    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(gfx().physicalDevice, mSurface, &presentModeCount, presentModes.data()));

    VkExtent2D swapchainExtent = {};
    // If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
    if (surfCaps.currentExtent.width == (uint32_t)-1) {
        // If the surface size is undefined, the size is set to
        // the size of the images requested.
        swapchainExtent.width = *width;
        swapchainExtent.height = *height;
    } else {
        // If the surface size is defined, the swap chain size must match
        swapchainExtent = surfCaps.currentExtent;
        *width = surfCaps.currentExtent.width;
        *height = surfCaps.currentExtent.height;
    }

    // Select a present mode for the swapchain

    // The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
    // This mode waits for the vertical blank ("v-sync")
    VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

    // If v-sync is not requested, try to find a mailbox mode
    // It's the lowest latency non-tearing present mode available
    if (!vsync) {
        for (size_t i = 0; i < presentModeCount; i++) {
            if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
                swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }
            if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
        }
    }

    // Determine the number of images
    uint32_t desiredNumberOfSwapchainImages = surfCaps.minImageCount + 1;
#if (defined(VK_USE_PLATFORM_MACOS_MVK) && defined(VK_EXAMPLE_XCODE_GENERATED))
    // SRS - Work around known MoltenVK issue re 2x frame rate when vsync (VK_PRESENT_MODE_FIFO_KHR) enabled
    struct utsname sysInfo;
    uname(&sysInfo);
    // SRS - When vsync is on, use minImageCount when not in fullscreen or when running on Apple Silcon
    // This forces swapchain image acquire frame rate to match display vsync frame rate
    if (vsync && (!fullscreen || strcmp(sysInfo.machine, "arm64") == 0)) {
        desiredNumberOfSwapchainImages = surfCaps.minImageCount;
    }
#endif
    if ((surfCaps.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfCaps.maxImageCount)) {
        desiredNumberOfSwapchainImages = surfCaps.maxImageCount;
    }

    // Find the transformation of the surface
    VkSurfaceTransformFlagsKHR preTransform;
    if (surfCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        // We prefer a non-rotated transform
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        preTransform = surfCaps.currentTransform;
    }

    // Find a supported composite alpha format (not all devices support alpha opaque)
    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    // Simply select the first composite alpha format available
    std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };
    for (auto& compositeAlphaFlag : compositeAlphaFlags) {
        if (surfCaps.supportedCompositeAlpha & compositeAlphaFlag) {
            compositeAlpha = compositeAlphaFlag;
            break;
        };
    }

    VkSwapchainCreateInfoKHR swapchainCI = {};
    swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCI.surface = mSurface;
    swapchainCI.minImageCount = desiredNumberOfSwapchainImages;
    swapchainCI.imageFormat = colorFormat;
    swapchainCI.imageColorSpace = colorSpace;
    swapchainCI.imageExtent = { swapchainExtent.width, swapchainExtent.height };
    swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCI.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
    swapchainCI.imageArrayLayers = 1;
    swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCI.queueFamilyIndexCount = 0;
    swapchainCI.presentMode = swapchainPresentMode;
    // Setting oldSwapChain to the saved handle of the previous swapchain aids in resource reuse and makes sure that we can still present already acquired images
    swapchainCI.oldSwapchain = oldSwapchain;
    // Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
    swapchainCI.clipped = VK_TRUE;
    swapchainCI.compositeAlpha = compositeAlpha;

    // Enable transfer source on swap chain images if supported
    if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
        swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    // Enable transfer destination on swap chain images if supported
    if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    VK_CHECK_RESULT(vkCreateSwapchainKHR(gfx().device, &swapchainCI, nullptr, &swapChain));

    // If an existing swap chain is re-created, destroy the old swap chain
    // This also cleans up all the presentable images
    if (oldSwapchain != VK_NULL_HANDLE) {
        // todo:
        vkDestroySwapchainKHR(gfx().device, oldSwapchain, nullptr);
    }
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(gfx().device, swapChain, &imageCount, NULL));

    // Get the swap chain images
    images.resize(imageCount);
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(gfx().device, swapChain, &imageCount, images.data()));

    if (renderTarget == nullptr) {
        VulkanRenderTarget* rt = new VulkanRenderTarget(*width, *height, true);
        renderTarget = rt;
    }

    auto rt = staticCast<VulkanRenderTarget>(renderTarget);
    rt->width = *width;
    rt->height = *height;
    rt->colorAttachments.clear();

    colorTextures.clear();

    for (uint32_t i = 0; i < imageCount; i++) {
        auto& t = colorTextures.emplace_back(new VulkanTexture(images[i], colorFormat));
        rt->colorAttachments.emplace_back(t->rtv());
    }
}

void VulkanSwapChain::setupDepthStencil()
{
    VkFormat vkDepthFormat = imgutil::toVk(depthFormat);

    // Find a suitable depth and/or stencil format
    VkBool32 validFormat { false };
    // Samples that make use of stencil will require a depth + stencil format, so we select from a different list
    if (requiresStencil) {
        validFormat = tools::getSupportedDepthStencilFormat(gfx().physicalDevice, &vkDepthFormat);
    } else {
        validFormat = tools::getSupportedDepthFormat(gfx().physicalDevice, &vkDepthFormat);
    }
    assert(validFormat);

    depthFormat = imgutil::fromVk(vkDepthFormat);

    auto rt = staticCast<VulkanRenderTarget>(renderTarget);
    if (rt) {

        TextureData textureData = TextureData::texture2D(desc.width, desc.height, depthFormat);
        textureData.usage = TextureUsage::DEPTH_STENCIL_ATTACHMENT;
        depthTexture = new VulkanTexture(textureData, SamplerInfo::create(Filter::NEAREST, SamplerAddressMode::CLAMP_TO_EDGE));
        rt->depthAttachment = depthTexture->dsv();
    }
}

/**
 * Acquires the next image in the swap chain
 *
 * @param presentCompleteSemaphore (Optional) Semaphore that is signaled when the image is ready for use
 * @param imageIndex Pointer to the image index that will be increased if the next image could be acquired
 *
 * @note The function will always wait until the next image has been acquired by setting timeout to UINT64_MAX
 *
 * @return VkResult of the image acquisition
 */
VkResult VulkanSwapChain::acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex, VkFence fence)
{
    // By setting timeout to UINT64_MAX we will always wait until the next image has been acquired or an actual error is thrown
    // With that we don't have to handle VK_NOT_READY
    return vkAcquireNextImageKHR(gfx().device, swapChain, UINT64_MAX, presentCompleteSemaphore, fence, imageIndex);
}

/**
 * Queue an image for presentation
 *
 * @param queue Presentation queue for presenting the image
 * @param imageIndex Index of the swapchain image to queue for presentation
 * @param waitSemaphore (Optional) Semaphore that is waited on before the image is presented (only used if != VK_NULL_HANDLE)
 *
 * @return VkResult of the queue presentation
 */
VkResult VulkanSwapChain::queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore)
{
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = NULL;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapChain;
    presentInfo.pImageIndices = &imageIndex;
    // Check if a wait semaphore has been specified to wait for before presenting the image
    if (waitSemaphore != VK_NULL_HANDLE) {
        presentInfo.pWaitSemaphores = &waitSemaphore;
        presentInfo.waitSemaphoreCount = 1;
    }
    return vkQueuePresentKHR(queue, &presentInfo);
}

/**
 * Destroy and free Vulkan resources used for the swapchain
 */
void VulkanSwapChain::cleanup()
{
    if (swapChain != VK_NULL_HANDLE) {
        renderTarget.reset();
    }

    if (mSurface != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(gfx().device, swapChain, nullptr);
        vkDestroySurfaceKHR(gfx().instance, mSurface, nullptr);
    }

    mSurface = VK_NULL_HANDLE;
    swapChain = VK_NULL_HANDLE;
}

}