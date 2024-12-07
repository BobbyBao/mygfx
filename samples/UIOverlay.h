#pragma once
#include "utils/RefCounted.h"
#include "utils/SharedPtr.h"

#include <imgui/imgui.h>

struct SDL_Window;
union SDL_Event;

namespace mygfx {

class GraphicsApi;
class Shader;
class Texture;

class UIOverlay : public utils::RefCounted {
public:
    UIOverlay();
    ~UIOverlay();

    void init();
    void update();
    void draw(GraphicsApi& cmd);
    void resize(uint32_t width, uint32_t height);

private:
    void freeResources();
    utils::Ref<Shader> mProgram;
    utils::Ref<Texture> mFontTexture;

    ImFont* font = nullptr;
    ImFont* iconFont = nullptr;
    bool visible = true;
    bool updated = false;
    float scale = 1.0f;
};
}
