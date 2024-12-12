#include "VulkanExample.h"
#include "utils/Log.h"
#include "vulkan/VulkanDevice.h"
#include "resource/Texture.h"

using namespace mygfx;
using namespace mygfx::samples;

static std::vector<DemoDesc*> sDemos;

DemoDesc::DemoDesc(const char* name, const std::function<Demo*()>& creator)
{
    this->name = name;
    this->creator = creator;

    sDemos.push_back(this);
}

VulkanExample::VulkanExample(int argc, char** argv)
{
}

bool VulkanExample::initVulkan() {
    
    mygfx::Settings s;
    s.name = title.c_str();
    s.validation = settings.validation;
    auto device = new VulkanDevice();
    if (!device->create(s)) {
        return false;
    }

    mGraphicsApi = std::make_unique<GraphicsApi>(*device);

    Texture::staticInit();
    return true;
}

void VulkanExample::prepare()
{
    SwapChainDesc desc {
        .width = width,
        .height = height,
    };
    
#if defined(_WIN32)
	desc.windowInstance = windowInstance;
    desc.window = window;
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
	desc.window = androidApp->window;
#elif (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
	desc.window = view;
#elif defined(VK_USE_PLATFORM_METAL_EXT)
	desc.window = metalLayer;
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
	//swapChain.initSurface(dfb, surface);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	//swapChain.initSurface(display, surface);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	//swapChain.initSurface(connection, window);
#elif (defined(_DIRECT2DISPLAY) || defined(VK_USE_PLATFORM_HEADLESS_EXT))
	//swapChain.initSurface(width, height);
#elif defined(VK_USE_PLATFORM_SCREEN_QNX)
	//swapChain.initSurface(screen_context, screen_window);
#endif

    mSwapchain = mGraphicsApi->createSwapchain(desc);

    if (sDemos.size() > 0) {
        setDemo(0);
    }

    mUI = makeShared<UIOverlay>();
    mUI->init();

    prepared = true;
}

void VulkanExample::setDemo(int index)
{
    if (index == mActiveDemoIndex) {
        return;
    }

    mActiveDemoIndex = index;
    auto demo = sDemos[mActiveDemoIndex]->creator();
    demo->mName = sDemos[mActiveDemoIndex]->name;
    demo->mApp = this;
    setDemo(demo);
}

void VulkanExample::setDemo(Demo* demo)
{
    if (demo == mActiveDemo) {
        return;
    }

    if (mActiveDemo) {
        mActiveDemo->stop();
    }

    mActiveDemo = demo;

    if (mActiveDemo) {
        mActiveDemo->start();
    }
}

void VulkanExample::render()
{
    auto& cmd = gfxApi();

    cmd.beginFrame();

    onPreDraw(cmd);

    cmd.makeCurrent(mSwapchain);

    RenderPassInfo renderInfo {
        .clearFlags = TargetBufferFlags::ALL,
        .clearColor = { 0.15f, 0.15f, 0.15f, 1.0f }
    };

    renderInfo.viewport = { .left = 0, .top = 0, .width = width, .height = height };

    cmd.beginRendering(mSwapchain->renderTarget, renderInfo);

    onDraw(cmd);

    mUI->draw(cmd);

    cmd.endRendering(mSwapchain->renderTarget);

    cmd.commit(mSwapchain);

    cmd.endFrame();

    cmd.flush();

}

void VulkanExample::onGUI()
{ 
    ImGui::SetNextWindowPos({ 10.0f, 10.0f });
    ImGui::SetNextWindowSize({ -1.0f, -1.0f });
    ImGui::SetNextWindowBgAlpha(0.75f);

    if (ImGui::Begin("Demos", nullptr, ImGuiWindowFlags_NoDecoration)) {
        //ImGui::Text("CPU:	%s", mCPUName.c_str());
        ImGui::Text("GPU:	%s", gfxApi().getDeviceName());
        ImGui::Text("FPS:	%d", lastFPS);
        ImGui::Text("DrawCall:%d", Stats::getDrawCall());

        const char* preview_value = mActiveDemo ? mActiveDemo->mName : "";

        if (ImGui::BeginCombo("Active Demo", preview_value)) {
            for (int i = 0; i < sDemos.size(); i++) {
                auto desc = sDemos[i];
                if (ImGui::Selectable(desc->name)) {
                    setDemo(i);
                }
            }

            ImGui::EndCombo();
        }

        if (mActiveDemo) {
            mActiveDemo->gui();
        }

        ImGui::End();
    }

}

void VulkanExample::windowResized()
{
    gfxApi().resize(mSwapchain, width, height);
}

void VulkanExample::keyPressed(uint32_t key)
{
}

void VulkanExample::update(float delta)
{
    if (mActiveDemo) {
        mActiveDemo->update(delta);
    }
}

void VulkanExample::onPreDraw(GraphicsApi& cmd)
{
    if (mActiveDemo) {
        mActiveDemo->preDraw(cmd);
    }
}

void VulkanExample::onDraw(GraphicsApi& cmd)
{
    if (mActiveDemo) {
        mActiveDemo->draw(cmd);
    }
}

void VulkanExample::onDestroy()
{
    if (mActiveDemo) {
        mActiveDemo->stop();
    }

    ShaderLibs::clean();

}

#include "Entrypoints.h"

VULKAN_EXAMPLE_MAIN()
