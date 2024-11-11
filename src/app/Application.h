#pragma once
#include "Framework.h"
#include "SDL.h"
#include "UIOverlay.h"

struct SDL_Window;
union SDL_Event;

namespace mygfx {

class Engine;
class CameraController;

class Application : public Framework {
public:
    Application(int argc = 0, char** argv = nullptr);
    virtual ~Application();

    void handleEvent(const SDL_Event& e);
protected:
    bool createWindow(void** window, void** windowInstance) override;
    void onInit() override;
    void onStart() override;
    void onDestroy() override;
    void processEvent() override;
    void onPreUpdate(double delta) override;
    void onPostDraw(GraphicsApi& cmd) override;

    virtual void onGUI();
    virtual void onResized(int w, int h);
    virtual void keyDown(uint32_t key);
    virtual void keyUp(uint32_t key);
    virtual void mouseDown(int button, float x, float y);
    virtual void mouseUp(int button, float x, float y);
    virtual void mouseWheel(float wheelDelta);
    virtual void mouseMove(float x, float y);

protected:
    void setupConsole(std::string title);
    void setupDPIAwareness();

    SDL_Window* mSdlWindow;
    Ref<UIOverlay> mUI;
    bool mShowProfiler = false;
    bool mShowGUI = true;
};

}
