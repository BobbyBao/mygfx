#pragma once

#include "core/System.h"

#include <imgui/imgui.h>

struct SDL_Window;
union SDL_Event;

namespace mygfx {

class GraphicsApi;
class Shader;
class Texture;

class UIOverlay : public TSystem<UIOverlay> {
public:
    UIOverlay(SDL_Window* wnd);
    ~UIOverlay();

    void init() override;
    bool handleEvent(const SDL_Event& event);
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

namespace ImGui {
void Texture(mygfx::Texture* tex, const ImVec2& image_size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0));
}