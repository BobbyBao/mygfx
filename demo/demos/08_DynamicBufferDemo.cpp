#include "DemoApp.h"
#include <array>
#include "Texture.h"

namespace mygfx::demo {
	
	class DynamicBufferDemo : public Demo {
	public:
		struct Vertex2D {
			float3 pos;
			uint32_t color;
			float2 uv;
			uint32_t texIndex;
		};

		Vector<Vertex2D> mVertices;
		Vector<uint16_t> mIndices;
		Ref<Shader> mShader;

		void start() override {

			mShader = ShaderLibs::getUnlitShader();

			const int SPRITE_COUNT = 10000;

			mSprites.clear();
			
			float width = (float)mApp->getWidth();
			float height = (float)mApp->getHeight();

			for (int i = 0; i < SPRITE_COUNT; i++) {

				float2 center = { linearRand<float>(0, width), linearRand<float>(0.0f, height) };
				float halfSize = linearRand<float>(5.0f, 10.0f);
				int index = linearRand<int>(0, 8);
				mSprites.emplace_back(
					Rect{ {center.x - halfSize, center.y - halfSize}, {center.x + halfSize, center.y + halfSize} },
					0.0f, 0xffffffff, Rect{ {0, 0}, {1, 1} }, index);

			}

			for (auto& spr : mSprites) {
				createQuad(spr.v);
			}
		}

		void createQuad(const std::span<Vertex2D, 4>& v) {
			uint16_t start = (uint16_t)mVertices.size();
			mVertices.insert(mVertices.end(), v.begin(), v.end());

			std::array<uint16_t, 6> indices{};
			indices[0] = start;
			indices[1] = start + 1;
			indices[2] = start + 2;
			indices[3] = start + 3;
			indices[4] = start;
			indices[5] = start + 2;
		
			mIndices.insert(mIndices.end(), indices.begin(), indices.end());
		}


		void draw(GraphicsApi& cmd) override {

			float width = (float)mApp->getWidth();
			float height = (float)mApp->getHeight();
			mat4 proj = ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);

			uint32_t ubo = cmd.allocConstant(proj);

			cmd.bindPipelineState(mShader->pipelineState);
			cmd.bindUniforms({ ubo });

			if (mIndices.size() > 0) {
				cmd.drawUserPrimitives(std::span{ mVertices }, std::span{ mIndices });
			}
		}


		struct Sprite2D {
			Vertex2D v[4]{};

			Sprite2D(const Rect& r, float depth, uint32_t color, const Rect& uv, Texture* tex)
				: Sprite2D(r, depth, color, uv, tex->index()) {
			}

			Sprite2D(const Rect& r, float depth, uint32_t color, const Rect& uv, uint32_t index) {

				v[0].pos = { r.min, depth };
				v[0].color = color;
				v[0].uv = uv.min;
				v[0].texIndex = index;

				v[1].pos = { r.max.x, r.min.y, depth };
				v[1].color = color;
				v[1].uv = { uv.max.x, uv.min.y };
				v[1].texIndex = index;

				v[2].pos = { r.max, depth };
				v[2].color = color;
				v[2].uv = uv.max;
				v[2].texIndex = index;

				v[3].pos = { r.min.x, r.max.y, depth };
				v[3].color = color;
				v[3].uv = { uv.min.x, uv.max.y };
				v[3].texIndex = index;

			}
		};

		Vector<Sprite2D> mSprites;
	};

	DEF_DEMO(DynamicBufferDemo, "DynamicBuffer Demo");	
}