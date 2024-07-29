#pragma once

#include "utils/SharedPtr.h"

#include <imgui/imgui.h>

struct SDL_Window;
union SDL_Event;

namespace mygfx {

	class GraphicsApi;
	class Program;
	class Texture;

	class UIOverlay : public utils::RefCounted
	{
	public:
		UIOverlay(SDL_Window* wnd);
		~UIOverlay();

		void init();
		bool handleEvent(const SDL_Event& event);
		void update();
		void draw(GraphicsApi& cmd);
		void resize(uint32_t width, uint32_t height);
		
        ImFont* font = nullptr;
        ImFont* iconFont = nullptr;
	private:
		void freeResources();
		utils::Ref<Program> mProgram;
		utils::Ref<Texture> mFontTexture;

		bool visible = true;
		bool updated = false;
		float scale = 1.0f;
	};
}
