#pragma once

#include "Application.h"

#ifdef _WIN32
#pragma comment(linker, "/subsystem:windows")
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <ShellScalingAPI.h>
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
#include <android/native_activity.h>
#include <android/asset_manager.h>
#include <android_native_app_glue.h>
#include <sys/system_properties.h>
#include "VulkanAndroid.h"
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
#include <directfb.h>
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#elif defined(_DIRECT2DISPLAY)
//
#elif defined(VK_USE_PLATFORM_XCB_KHR)
#include <xcb/xcb.h>
#endif

namespace mygfx::samples {

class PlatformApp : public Application
{
public:
    PlatformApp();

protected:
	
	// OS specific
#if defined(_WIN32)
	HWND window;
	HINSTANCE windowInstance;
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
	// true if application has focused, false if moved to background
	bool focused = false;
	struct TouchPos {
		int32_t x;
		int32_t y;
	} touchPos;
	bool touchDown = false;
	double touchTimer = 0.0;
	int64_t lastTapTime = 0;
#elif (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
	void* view;
#if defined(VK_EXAMPLE_XCODE_GENERATED)
	bool quit = false;
#endif
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
	bool quit = false;
	IDirectFB *dfb = nullptr;
	IDirectFBDisplayLayer *layer = nullptr;
	IDirectFBWindow *window = nullptr;
	IDirectFBSurface *surface = nullptr;
	IDirectFBEventBuffer *event_buffer = nullptr;
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	wl_display *display = nullptr;
	wl_registry *registry = nullptr;
	wl_compositor *compositor = nullptr;
	struct xdg_wm_base *shell = nullptr;
	wl_seat *seat = nullptr;
	wl_pointer *pointer = nullptr;
	wl_keyboard *keyboard = nullptr;
	wl_surface *surface = nullptr;
	struct xdg_surface *xdg_surface;
	struct xdg_toplevel *xdg_toplevel;
	bool quit = false;
	bool configured = false;

#elif defined(_DIRECT2DISPLAY)
	bool quit = false;
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	bool quit = false;
	xcb_connection_t *connection;
	xcb_screen_t *screen;
	xcb_window_t window;
	xcb_intern_atom_reply_t *atom_wm_delete_window;
#elif defined(VK_USE_PLATFORM_HEADLESS_EXT)
	bool quit = false;
#elif defined(VK_USE_PLATFORM_SCREEN_QNX)
	screen_context_t screen_context = nullptr;
	screen_window_t screen_window = nullptr;
	screen_event_t screen_event = nullptr;
	bool quit = false;
#endif


#if defined(_WIN32)
	void setupConsole(std::string title);
	void setupDPIAwareness();
	HWND setupWindow(HINSTANCE hinstance, WNDPROC wndproc);
	void handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
	static int32_t handleAppInput(struct android_app* app, AInputEvent* event);
	static void handleAppCommand(android_app* app, int32_t cmd);
#elif (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
	void* setupWindow(void* view);
	void displayLinkOutputCb();
	void mouseDragged(float x, float y);
	void windowWillResize(float x, float y);
	void windowDidResize();
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
	IDirectFBSurface *setupWindow();
	void handleEvent(const DFBWindowEvent *event);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	struct xdg_surface *setupWindow();
	void initWaylandConnection();
	void setSize(int width, int height);
	static void registryGlobalCb(void *data, struct wl_registry *registry,
			uint32_t name, const char *interface, uint32_t version);
	void registryGlobal(struct wl_registry *registry, uint32_t name,
			const char *interface, uint32_t version);
	static void registryGlobalRemoveCb(void *data, struct wl_registry *registry,
			uint32_t name);
	static void seatCapabilitiesCb(void *data, wl_seat *seat, uint32_t caps);
	void seatCapabilities(wl_seat *seat, uint32_t caps);
	static void pointerEnterCb(void *data, struct wl_pointer *pointer,
			uint32_t serial, struct wl_surface *surface, wl_fixed_t sx,
			wl_fixed_t sy);
	static void pointerLeaveCb(void *data, struct wl_pointer *pointer,
			uint32_t serial, struct wl_surface *surface);
	static void pointerMotionCb(void *data, struct wl_pointer *pointer,
			uint32_t time, wl_fixed_t sx, wl_fixed_t sy);
	void pointerMotion(struct wl_pointer *pointer,
			uint32_t time, wl_fixed_t sx, wl_fixed_t sy);
	static void pointerButtonCb(void *data, struct wl_pointer *wl_pointer,
			uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
	void pointerButton(struct wl_pointer *wl_pointer,
			uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
	static void pointerAxisCb(void *data, struct wl_pointer *wl_pointer,
			uint32_t time, uint32_t axis, wl_fixed_t value);
	void pointerAxis(struct wl_pointer *wl_pointer,
			uint32_t time, uint32_t axis, wl_fixed_t value);
	static void keyboardKeymapCb(void *data, struct wl_keyboard *keyboard,
			uint32_t format, int fd, uint32_t size);
	static void keyboardEnterCb(void *data, struct wl_keyboard *keyboard,
			uint32_t serial, struct wl_surface *surface, struct wl_array *keys);
	static void keyboardLeaveCb(void *data, struct wl_keyboard *keyboard,
			uint32_t serial, struct wl_surface *surface);
	static void keyboardKeyCb(void *data, struct wl_keyboard *keyboard,
			uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
	void keyboardKey(struct wl_keyboard *keyboard,
			uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
	static void keyboardModifiersCb(void *data, struct wl_keyboard *keyboard,
			uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched,
			uint32_t mods_locked, uint32_t group);

#elif defined(_DIRECT2DISPLAY)
//
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	xcb_window_t setupWindow();
	void initxcbConnection();
	void handleEvent(const xcb_generic_event_t *event);
#elif defined(VK_USE_PLATFORM_SCREEN_QNX)
	void setupWindow();
	void handleEvent();
#else
	void setupWindow();
#endif


};








// OS specific macros for the example main entry points
#if defined(_WIN32)
// Windows entry point
#define MYGFX_SAMPLE_MAIN(SAMPLE_APP)																		\
Application *app;																		\
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)						\
{																									\
	if (app != NULL)																		\
	{																								\
		app->handleMessages(hWnd, uMsg, wParam, lParam);									\
	}																								\
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));												\
}																									\
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)									\
{																									\
	for (int32_t i = 0; i < __argc; i++) { Application::args.push_back(__argv[i]); };  			\
	app = new SAMPLE_APP();															\
	app->initVulkan();																	\
	app->setupWindow(hInstance, WndProc);													\
	app->prepare();																		\
	app->renderLoop();																	\
	delete(app);																			\
	return 0;																						\
}
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
// Android entry point
#define MYGFX_SAMPLE_MAIN(SAMPLE_APP)																		\
Application *app;																		\
void android_main(android_app* state)																\
{																									\
	app = new SAMPLE_APP();															\
	state->userData = app;																\
	state->onAppCmd = Application::handleAppCommand;												\
	state->onInputEvent = Application::handleAppInput;											\
	androidApp = state;																				\
	vks::android::getDeviceConfig();																\
	app->renderLoop();																	\
	delete(app);																			\
}
#elif defined(_DIRECT2DISPLAY)
// Linux entry point with direct to display wsi
#define MYGFX_SAMPLE_MAIN(SAMPLE_APP)																		\
Application *app;																		\
static void handleEvent()                                											\
{																									\
}																									\
int main(const int argc, const char *argv[])													    \
{																									\
	for (size_t i = 0; i < argc; i++) { Application::args.push_back(argv[i]); };  				\
	app = new SAMPLE_APP();															\
	app->initVulkan();																	\
	app->prepare();																		\
	app->renderLoop();																	\
	delete(app);																			\
	return 0;																						\
}
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
#define MYGFX_SAMPLE_MAIN(SAMPLE_APP)																		\
Application *app;																		\
static void handleEvent(const DFBWindowEvent *event)												\
{																									\
	if (app != NULL)																		\
	{																								\
		app->handleEvent(event);															\
	}																								\
}																									\
int main(const int argc, const char *argv[])													    \
{																									\
	for (size_t i = 0; i < argc; i++) { Application::args.push_back(argv[i]); };  				\
	app = new SAMPLE_APP();															\
	app->initVulkan();																	\
	app->setupWindow();					 												\
	app->prepare();																		\
	app->renderLoop();																	\
	delete(app);																			\
	return 0;																						\
}
#elif (defined(VK_USE_PLATFORM_WAYLAND_KHR) || defined(VK_USE_PLATFORM_HEADLESS_EXT))
#define MYGFX_SAMPLE_MAIN(SAMPLE_APP)																		\
Application *app;																		\
int main(const int argc, const char *argv[])													    \
{																									\
	for (size_t i = 0; i < argc; i++) { Application::args.push_back(argv[i]); };  				\
	app = new SAMPLE_APP();															\
	app->initVulkan();																	\
	app->setupWindow();					 												\
	app->prepare();																		\
	app->renderLoop();																	\
	delete(app);																			\
	return 0;																						\
}
#elif defined(VK_USE_PLATFORM_XCB_KHR)
#define MYGFX_SAMPLE_MAIN(SAMPLE_APP)																		\
Application *app;																		\
static void handleEvent(const xcb_generic_event_t *event)											\
{																									\
	if (app != NULL)																		\
	{																								\
		app->handleEvent(event);															\
	}																								\
}																									\
int main(const int argc, const char *argv[])													    \
{																									\
	for (size_t i = 0; i < argc; i++) { Application::args.push_back(argv[i]); };  				\
	app = new SAMPLE_APP();															\
	app->initVulkan();																	\
	app->setupWindow();					 												\
	app->prepare();																		\
	app->renderLoop();																	\
	delete(app);																			\
	return 0;																						\
}
#elif (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
#if defined(VK_EXAMPLE_XCODE_GENERATED)
#define MYGFX_SAMPLE_MAIN(SAMPLE_APP)																		\
Application *app;																		\
int main(const int argc, const char *argv[])														\
{																									\
	@autoreleasepool																				\
	{																								\
		for (size_t i = 0; i < argc; i++) { Application::args.push_back(argv[i]); };				\
		app = new SAMPLE_APP();														\
		app->initVulkan();																\
		app->setupWindow(nullptr);														\
		app->prepare();																	\
		app->renderLoop();																\
		delete(app);																		\
	}																								\
	return 0;																						\
}
#else
#define MYGFX_SAMPLE_MAIN(SAMPLE_APP)
#endif

#elif defined(VK_USE_PLATFORM_SCREEN_QNX)
#define MYGFX_SAMPLE_MAIN(SAMPLE_APP)												\
Application *app;												\
int main(const int argc, const char *argv[])										\
{															\
	for (int i = 0; i < argc; i++) { Application::args.push_back(argv[i]); };					\
	app = new SAMPLE_APP();										\
	app->initVulkan();											\
	app->setupWindow();											\
	app->prepare();											\
	app->renderLoop();											\
	delete(app);												\
	return 0;													\
}
#endif










}