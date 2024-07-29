#include "UIOverlay.h"
#include "GraphicsApi.h"
#include "utils/FileUtils.h"

#include <SDL.h>

#include <SDL3/SDL_iostream.h>

#include <imgui/backends/imgui_impl_sdl3.h>

namespace mygfx
{
	static ImFont* addFont(const char* filePath, float size_pixels, const ImFontConfig* font_cfg = nullptr, const ImWchar* glyph_ranges = nullptr)
	{
		auto& io = ImGui::GetIO();
		auto file = SDL_IOFromFile(filePath, "rb");
		auto fileSize = SDL_GetIOSize(file);
		auto bytes = ImGui::MemAlloc(fileSize);
		SDL_ReadIO(file, bytes, fileSize);
		SDL_CloseIO(file);
		return io.Fonts->AddFontFromMemoryTTF(bytes, (int)fileSize, size_pixels, font_cfg, glyph_ranges);
	}

	UIOverlay::UIOverlay(SDL_Window* wnd)
	{
#if defined(__ANDROID__)		
		if (vks::android::screenDensity >= ACONFIGURATION_DENSITY_XXHIGH) {
			scale = 3.5f;
		}
		else if (vks::android::screenDensity >= ACONFIGURATION_DENSITY_XHIGH) {
			scale = 2.5f;
		}
		else if (vks::android::screenDensity >= ACONFIGURATION_DENSITY_HIGH) {
			scale = 2.0f;
		};
#endif

		// Init ImGui
		ImGui::CreateContext();

		// Dimensions
		ImGuiIO& io = ImGui::GetIO();
		io.FontGlobalScale = scale;
		
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;
		
		auto& style = ImGui::GetStyle();

		ImGui::StyleColorsDark();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		ImGui_ImplSDL3_InitForVulkan(wnd);
	}

	UIOverlay::~UIOverlay()	{
		
		freeResources();

		ImGui_ImplSDL3_Shutdown();

		if (ImGui::GetCurrentContext()) {
			ImGui::DestroyContext();
		}

	}

	void UIOverlay::init()
	{
		ImGuiIO& io = ImGui::GetIO();

		//mShader = Shader::load("shaders/UI.shader");
		//mProgram = mShader->getMainPass();
				
		float size = 18.0f;

		font = addFont("fonts/arial.ttf", size);

		// Create font texture
		unsigned char* fontData;
		int texWidth, texHeight;
		io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);

		size_t dataSize = texWidth * texHeight * 4 * sizeof(char);

		//SRS - Set ImGui style scale factor to handle retina and other HiDPI displays (same as font scaling above)
		ImGuiStyle& style = ImGui::GetStyle();
		style.ScaleAllSizes(scale);

		mFontTexture = Texture::create2D(texWidth, texHeight, Format::R8G8B8A8_UNORM, MemoryBlock(fontData, dataSize));
		io.Fonts->TexID = (void*)mFontTexture->index();

	}
	
	bool UIOverlay::handleEvent(const SDL_Event& event)
	{
		if (ImGui::GetCurrentContext() == nullptr) {
			return false;
		}

		return ImGui_ImplSDL3_ProcessEvent(&event);
	}

	void UIOverlay::update()
	{
		ImGui_ImplSDL3_NewFrame();

		ImGui::NewFrame();
	}
	
	void UIOverlay::draw(GraphicsApi& cmd)
	{
		ImGui::Render();
		
		ImDrawData* imDrawData = ImGui::GetDrawData();
		int32_t vertexOffset = 0;
		int32_t indexOffset = 0;

		if ((!imDrawData) || (imDrawData->CmdListsCount == 0)) {
			return;
		}

		ImGuiIO& io = ImGui::GetIO();

		char* pVertices = NULL;
		BufferInfo VerticesView;
		device().allocVertexBuffer(imDrawData->TotalVtxCount, sizeof(ImDrawVert), (void**)&pVertices, &VerticesView);

		char* pIndices = NULL;
		BufferInfo indicesView;
		device().allocIndexBuffer(imDrawData->TotalIdxCount, sizeof(ImDrawIdx), (void**)&pIndices, &indicesView);

		// Upload data

		ImDrawVert* vtxDst = (ImDrawVert*)pVertices;
		ImDrawIdx* idxDst = (ImDrawIdx*)pIndices;

		for (int n = 0; n < imDrawData->CmdListsCount; n++) {
			const ImDrawList* cmd_list = imDrawData->CmdLists[n];
			memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
			vtxDst += cmd_list->VtxBuffer.Size;
			idxDst += cmd_list->IdxBuffer.Size;
		}

		cmd.setViewport(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y, 0, 1);
		
		cmd.bindVertexBuffer(0, VerticesView.buffer, VerticesView.offset);
		cmd.bindIndexBuffer(indicesView.buffer, indicesView.offset, IndexType::UInt16);
				
		float L = 0.0f;
		float R = ImGui::GetIO().DisplaySize.x;
		float B = ImGui::GetIO().DisplaySize.y;
		float T = 0.0f;
		float mvp[4][4] =
		{
			{ 2.0f / (R - L),   0.0f,           0.0f,       0.0f },
			{ 0.0f,         2.0f / (T - B),     0.0f,       0.0f },
			{ 0.0f,         0.0f,           0.5f,       0.0f },
			{ (R + L) / (L - R),  (T + B) / (B - T),    0.5f,       1.0f },
		};

		uint32_t perView = device().allocConstant(mvp);
		
		cmd.bindPipelineState(mProgram->pipelineState);
		cmd.bindUniforms({perView});
				
		uint32_t topX, topY, width, height;

		for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
		{
			const ImDrawList* cmd_list = imDrawData->CmdLists[i];
			for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
			{
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
				topX = std::max((int32_t)(pcmd->ClipRect.x), 0);
				topY = std::max((int32_t)(pcmd->ClipRect.y), 0);
				width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
				height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
				cmd.setScissor(topX, topY, width, height);
				cmd.drawIndexed(pcmd->ElemCount, 1, indexOffset, vertexOffset, (uint32_t)(uint64_t)pcmd->TextureId);
				indexOffset += pcmd->ElemCount;
			}
			vertexOffset += cmd_list->VtxBuffer.Size;
		}

		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void UIOverlay::resize(uint32_t width, uint32_t height)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)(width), (float)(height));
	}

	void UIOverlay::freeResources()
	{
		mProgram.reset();
		//mFontTexture.reset();
	}

}
