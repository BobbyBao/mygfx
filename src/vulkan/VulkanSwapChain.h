#pragma once

#include <stdlib.h>
#include <string>
#include <assert.h>
#include <stdio.h>
#include <vector>
#include "VulkanDefs.h"
#include "VulkanTools.h"

#ifdef __APPLE__
#include <sys/utsname.h>
#endif

#include "../GraphicsHandles.h"
#include "VulkanTexture.h"

namespace mygfx {

class VulkanSwapChain : public HwSwapchain
{
public:
	VulkanSwapChain();
	VulkanSwapChain(const SwapChainDesc& desc);
	~VulkanSwapChain();

	void initSurface();
	void recreate(const SwapChainDesc* pDesc = nullptr);

#if defined(VK_USE_PLATFORM_WIN32_KHR)
	bool initSurface(void* platformHandle, void* platformWindow);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
	bool initSurface(ANativeWindow* window);
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
	bool initSurface(IDirectFB* dfb, IDirectFBSurface* window);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	bool initSurface(wl_display* display, wl_surface* window);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	bool initSurface(xcb_connection_t* connection, xcb_window_t window);
#elif (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
	bool initSurface(void* view);
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    bool initSurface(CAMetalLayer* metalLayer);
#elif (defined(_DIRECT2DISPLAY) || defined(VK_USE_PLATFORM_HEADLESS_EXT))
	bool initSurface(uint32_t width, uint32_t height);
#if defined(_DIRECT2DISPLAY)
	bool createDirect2DisplaySurface(uint32_t width, uint32_t height);
#endif
#elif defined(VK_USE_PLATFORM_SCREEN_QNX)
	bool initSurface(screen_context_t screen_context, screen_window_t screen_window);
#endif
	void create(uint32_t* width, uint32_t* height, bool vsync = false, bool fullscreen = false);

	VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex, VkFence fence);
	VkResult queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);
	void cleanup();
	
	VkFormat colorFormat;
	VkColorSpaceKHR colorSpace;
	VkSwapchainKHR swapChain = VK_NULL_HANDLE;	
	uint32_t imageCount;
	std::vector<VkImage> images;
	uint32_t queueNodeIndex = UINT32_MAX;	
	bool requiresStencil{ false };
	Format depthFormat;
    Vector<Ref<VulkanTexture>> colorTextures;
	Ref<VulkanTexture> depthTexture;
private:
    void selectSurfaceFormat();
	void setupDepthStencil();

	VkSurfaceKHR mSurface = VK_NULL_HANDLE;
};

}