#include "DemoApp.h"

namespace mygfx::demo {
	
	class TriangleDemo : public Demo {
	public:
		Ref<HwBuffer> mVB;
		Ref<Program> mShader;

		struct VertexColored {
			float3 pos;
			uint32_t color;
		};

		void start() override {

			mShader = ShaderLibs::getColorShader();

			VertexColored pos[] = {
				{{0.0f, -1.0f, 0.0f}, 0xff0000ff },
				{{-1.0f, 1.0f, 0.0f}, 0xff00ff00 },
				{{1.0f, 1.0f, 0.0f}, 0xffff0000 }
			};
			
			mVB = device().createBuffer1(BufferUsage::Vertex, MemoryUsage::GpuOnly, 3, pos);
		}

		void draw(GraphicsApi& cmd) override {
			float L = -1;
			float R = 1;
			float B = 1;
			float T = -1.0f;
			float mvp[4][4] =
			{
				{ 2.0f / (R - L),   0.0f,           0.0f,       0.0f },
				{ 0.0f,         2.0f / (T - B),     0.0f,       0.0f },
				{ 0.0f,         0.0f,           0.5f,       0.0f },
				{ (R + L) / (L - R),  (T + B) / (B - T),    0.5f,       1.0f },
			};

			uint32_t perView = device().allocConstant(mvp);

			cmd.bindPipelineState(mShader->pipelineState);
			cmd.bindUniforms({ perView });
			cmd.bindVertexBuffer(0, mVB, 0);
			cmd.draw(3, 1, 0, 0);
		}
	};

	DEF_DEMO(TriangleDemo, "Triangle demo");	
}