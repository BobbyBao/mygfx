
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include "DemoApp.h"

using namespace mygfx::demo;

int SDL_AppInit(void** appstate, int argc, char** argv)
{
    auto app = new mygfx::demo::DemoApp(argc, argv);
    if (!app->init()) {
        delete app;
        return -1;
    }
    *appstate = app;
    return 0;
}

int SDL_AppIterate(void* appstate)
{
    auto app = (DemoApp*)appstate;
    app->updateFrame();
    return 0;
}

int SDL_AppEvent(void* appstate, const SDL_Event* event)
{
    auto app = (DemoApp*)appstate;

    if (event->type == SDL_EVENT_QUIT) {
        app->quit();
        return 1;
    }

    app->handleEvent(*event);
    return 0;
}

void SDL_AppQuit(void* appstate)
{
    auto app = (DemoApp*)appstate;
    app->destroy();
    delete app;
}
