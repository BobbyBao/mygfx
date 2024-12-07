#pragma once
#include "GraphicsApi.h"

#include "VulkanExampleBase.h"
#include "ShaderLibs.h"
#include "Maths.h"
#include "UIOverlay.h"

namespace mygfx::samples {

class VulkanExample;

class Demo : public utils::RefCounted {
public:
    virtual void start() { }
    virtual void gui() { }
    virtual void update(double delta) { }
    virtual void preDraw(GraphicsApi& cmd) { }
    virtual void draw(GraphicsApi& cmd) { }
    virtual void stop() { }

protected:
    const char* mName = "";
    String mDesc;
    VulkanExample* mApp = nullptr;

    friend class VulkanExample;
};

struct DemoDesc {
    const char* name;
    std::function<Demo*()> creator;
    DemoDesc(const char* name, const std::function<Demo*()>& creator);
};

#define DEF_DEMO(TYPE, NAME) \
    inline static DemoDesc s_##TYPE(NAME, []() { return new TYPE(); });

class VulkanExample : public VulkanExampleBase
{
public:
    VulkanExample(int argc = 0, char** argv = nullptr);
    
	bool initVulkan() override;
    void prepare() override;

    void setDemo(int index);
    void setDemo(Demo* demo);
protected:
    void onDestroy();
    void render();
    void onGUI();
    void windowResized() override;
    void keyPressed(uint32_t key) override;
    void onUpdate(double delta);
    void onPreDraw(GraphicsApi& cmd);
    void onDraw(GraphicsApi& cmd);
    
    std::unique_ptr<GraphicsApi> mGraphicsApi;
    Ref<HwSwapchain> mSwapchain;
    Ref<UIOverlay> mUI;
    int mActiveDemoIndex = -1;
    Ref<Demo> mActiveDemo;
};

}