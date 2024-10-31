#pragma once

#include "Application.h"
#include "ShaderLibs.h"
#include "core/Maths.h"
#include <concurrencpp/concurrencpp.h>

namespace mygfx::demo {

using namespace math;

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

using namespace concurrencpp;

template <typename T>
using Result = concurrencpp::result<T>;

class DemoApp : public Application, public concurrencpp::runtime {
public:
    DemoApp(int argc = 0, char** argv = nullptr);

    void setDemo(int index);
    void setDemo(Demo* demo);

    const std::shared_ptr<concurrencpp::manual_executor>& getMainExecutor() const { return mMainExecutor; }

    static DemoApp* get() { return (DemoApp*)msInstance; }
protected:
    void onStart() override;
    void onDestroy() override;
    void onGUI() override;
    void onUpdate(double delta) override;
    void onPreDraw(GraphicsApi& cmd) override;
    void onDraw(GraphicsApi& cmd) override;

    std::shared_ptr<concurrencpp::manual_executor> mMainExecutor;
    int mActiveDemoIndex = -1;
    Ref<Demo> mActiveDemo;
};

}