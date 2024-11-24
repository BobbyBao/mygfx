#pragma once

#include "Application.h"
#include "ShaderLibs.h"
#include "Maths.h"

namespace mygfx::samples {

class DemoApp;

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
    DemoApp* mApp = nullptr;

    friend class DemoApp;
};

struct DemoDesc {
    const char* name;
    std::function<Demo*()> creator;
    DemoDesc(const char* name, const std::function<Demo*()>& creator);
};

#define DEF_DEMO(TYPE, NAME) \
    inline static DemoDesc s_##TYPE(NAME, []() { return new TYPE(); });

class DemoApp : public Application
{
public:
    DemoApp(int argc = 0, char** argv = nullptr);

    void setDemo(int index);
    void setDemo(Demo* demo);

    static DemoApp* get() { return (DemoApp*)msInstance; }

protected:
    void onStart() override;
    void onDestroy() override;
    void onGUI() override;
    void keyDown(uint32_t key) override;
    void onUpdate(double delta) override;
    void onPreDraw(GraphicsApi& cmd) override;
    void onDraw(GraphicsApi& cmd) override;

    int mActiveDemoIndex = -1;
    Ref<Demo> mActiveDemo;
};

}