#include "UIOverlay.h"
#include "GraphicsApi.h"
#include "utils/FileUtils.h"
#include "resource/Shader.h"
#include "resource/Texture.h"
#include "MathTypes.h"

#include <SDL.h>
#include <SDL3/SDL_iostream.h>

#include <imgui/backends/imgui_impl_sdl3.h>

namespace mygfx
{
	const char* vsCode = R"(
			#version 450

			#extension GL_ARB_separate_shader_objects : enable
			#extension GL_ARB_shading_language_420pack : enable

			layout (location = 0) in vec4 inPos;
			layout (location = 1) in vec2 inUV;
			layout (location = 2) in vec4 inColor;

			layout (binding = 0) uniform ProjConstants {
				mat4 proj;
			} ubo;

			layout (location = 0) out vec2 outUV;
			layout (location = 1) out vec4 outColor;
			layout (location = 2) flat out int texIndex;

			out gl_PerVertex 
			{
				vec4 gl_Position;
			};

			void main() 
			{
				outUV = inUV;
				outColor = inColor;
				gl_Position = ubo.proj * inPos;// vec4(inPos, 0.0, 1.0);
				texIndex = gl_InstanceIndex;
			}
)";

	const char* fsCode = R"(
			#version 450

			#extension GL_ARB_separate_shader_objects : enable
			#extension GL_ARB_shading_language_420pack : enable
			#extension GL_EXT_nonuniform_qualifier : require


			layout (location = 0) in vec2 inUV;
			layout (location = 1) in vec4 inColor;

			layout(location = 2) flat in int texIndex;

			layout (location = 0) out vec4 outColor;

			layout(set = 1, binding = 0) uniform sampler2D textures[];

			void main() 
			{
				outColor = inColor;
				
				if (texIndex < 0) {
					outColor.a *= textureLod(textures[nonuniformEXT(-texIndex)], inUV, 0).r;
				} else {
					outColor *= textureLod(textures[nonuniformEXT(texIndex)], inUV, 0);
				}
			}
)";
	static ImFont* addFont(const char* filePath, float size_pixels, const ImFontConfig* font_cfg = nullptr, const ImWchar* glyph_ranges = nullptr)
	{
		auto& io = ImGui::GetIO();
		auto file = SDL_IOFromFile(filePath, "rb");
		if (!file) {
			return nullptr;
		}

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

		mProgram = new Shader(vsCode, fsCode);
		mProgram->setVertexInput({Format::R32G32_SFLOAT, Format::R32G32_SFLOAT, Format::R8G8B8A8_UNORM });
		mProgram->setBlendMode(BlendMode::ALPHA);
		mProgram->pipelineState.rasterState.cullMode = CullMode::NONE;

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
		io.Fonts->TexID = (void*)(int64_t)mFontTexture->index();

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
		gfxApi().allocVertexBuffer(imDrawData->TotalVtxCount, sizeof(ImDrawVert), (void**)&pVertices, &VerticesView);

		char* pIndices = NULL;
		BufferInfo indicesView;
		gfxApi().allocIndexBuffer(imDrawData->TotalIdxCount, sizeof(ImDrawIdx), (void**)&pIndices, &indicesView);

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
		cmd.bindIndexBuffer(indicesView.buffer, indicesView.offset, IndexType::UINT16);
				
		float L = 0.0f;
		float R = ImGui::GetIO().DisplaySize.x;
		float B = ImGui::GetIO().DisplaySize.y;
		float T = 0.0f;

		auto m = glm::ortho(L, R, B, T, -1.0f, 1.0f);

		uint32_t perView = gfxApi().allocConstant(m);
		
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
		mFontTexture.reset();
	}

}

namespace ImGui {

	void Texture(mygfx::Texture* tex, const ImVec2& image_size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col) {
		if (tex->getSRV()) {
			ImGui::Image((ImTextureID)(int64_t)tex->getSRV()->index(), image_size, uv0, uv1, tint_col, border_col);
		}
		else {
			ImGui::Image((ImTextureID)(int64_t)mygfx::Texture::Magenta->getSRV()->index(), image_size);
		}
	}
}

