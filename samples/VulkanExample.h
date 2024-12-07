#pragma once

#include "VulkanExampleBase.h"
#include "ShaderLibs.h"
#include "Maths.h"

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
    
    void setDemo(int index);
    void setDemo(Demo* demo);

    //static DemoApp* get() { return (DemoApp*)msInstance; }

protected:
    void onStart();
    void onDestroy();
    void render();
    void onGUI();
    void keyDown(uint32_t key);
    void onUpdate(double delta);
    void onPreDraw(GraphicsApi& cmd);
    void onDraw(GraphicsApi& cmd);

    int mActiveDemoIndex = -1;
    Ref<Demo> mActiveDemo;
};

}