#include "DemoApp.h"
#include "Mesh.h"
#include "Texture.h"
#include <random>

namespace mygfx::demo {
	
	class ComputeShaderDemo : public Demo {
	public:
		const static uint32_t PARTICLE_COUNT = 256 * 1024;

		Ref<HwBuffer> mSBO;
		Ref<Program> mShader;
		Ref<Program> mComputeShader;

		float timer = 0.0f;
		float animStart = 20.0f;
		bool attachToCursor = false;

		struct Particle {
			vec2 pos;
			vec3 vel;
		};

		struct UniformData {						// Compute shader uniform block object
			float deltaT;							//		Frame delta time
			float destX;							//		x position of the attractor
			float destY;							//		y position of the attractor
			uint32_t particleCount = PARTICLE_COUNT;
		} uniformData;
		
		void start() override {

			std::default_random_engine rndEngine((unsigned)time(nullptr));
			std::uniform_real_distribution<float> rndDist(-1.0f, 1.0f);

			// Initial particle positions
			std::vector<Particle> particleBuffer(PARTICLE_COUNT);
			for (auto& particle : particleBuffer) {
				particle.pos = glm::vec2(rndDist(rndEngine), rndDist(rndEngine));
				particle.vel = glm::vec3(0.0f, 0.0f, particle.pos.x / 2.0f);			
			}

			mSBO = device().createBuffer1(BufferUsage::Storage | BufferUsage::Vertex, MemoryUsage::GpuOnly, std::span{ particleBuffer });

			mComputeShader = new Program(csCode);

			auto ds = mComputeShader->getProgram()->getDescriptorSet(0);
			getGraphicsApi().updateDescriptorSet2(ds, 0, mSBO);

			mShader = new Program(vsCode, fsCode);
			mShader->setVertexInput({
				Format::R32G32_SFLOAT, 
				Format::R32G32B32_SFLOAT });
			mShader->setBlendMode(BlendMode::Add);
			mShader->pipelineState.primitiveState.primitiveTopology = PrimitiveTopology::PointList;

			auto ds1 = mShader->getProgram()->getDescriptorSet(0);

			auto tex = Texture::createFromFile("../../media/textures/particle_rgba.ktx");
			auto tex1 = Texture::createFromFile("../../media/textures/particle_gradient_rgba.ktx");
			getGraphicsApi().updateDescriptorSet1(ds1, 0, tex->getSRV());
			getGraphicsApi().updateDescriptorSet1(ds1, 1, tex1->getSRV());
		}

		void preDraw(GraphicsApi& cmd) override {

			auto width = mApp->getWidth();
			auto height = mApp->getHeight();
			float frameTimer = (float)mApp->getDeltaTime();

			if (!attachToCursor)
			{
				if (animStart > 0.0f)
				{
					animStart -= frameTimer * 5.0f;
				}
				else if (animStart <= 0.0f)
				{
					timer += frameTimer * 0.04f;
					if (timer > 1.f)
						timer = 0.f;
				}
			}

			uniformData.deltaT = frameTimer;
			if (!attachToCursor)
			{
				uniformData.destX = sin(glm::radians(timer * 360.0f)) * 0.75f;
				uniformData.destY = 0.0f;
			}
			else
			{
				float x, y;
				SDL_GetMouseState(&x, &y);
				float normalizedMx = (x - static_cast<float>(width / 2)) / static_cast<float>(width / 2);
				float normalizedMy = (y - static_cast<float>(height / 2)) / static_cast<float>(height / 2);
				uniformData.destX = normalizedMx;
				uniformData.destY = normalizedMy;
			}

			cmd.bindShaderProgram(mComputeShader->getProgram());
			auto ubo = cmd.allocConstant(uniformData);
			cmd.bindUniforms({ ubo });
			cmd.dispatch(PARTICLE_COUNT / 256, 1, 1);
		}

		void draw(GraphicsApi& cmd) override {

			cmd.bindPipelineState(mShader->pipelineState);
			cmd.bindVertexBuffer(0, mSBO, 0);
			cmd.bindUniforms({});
			cmd.draw(PARTICLE_COUNT, 1, 0, 0);

		}

		const char* csCode = R"(
		#version 450

		struct Particle
		{
			vec2 pos;
			vec3 vel;
		};

		// Binding 0 : Position storage buffer
		layout(std140, binding = 0) buffer Pos
		{
			Particle particles[];
		};

		layout(local_size_x = 256) in;

		layout(binding = 1) uniform UBO
		{
			float deltaT;
			float destX;
			float destY;
			int particleCount;
		} ubo;

		vec2 attraction(vec2 pos, vec2 attractPos)
		{
			vec2 delta = attractPos - pos;
			const float damp = 0.5;
			float dDampedDot = dot(delta, delta) + damp;
			float invDist = 1.0f / sqrt(dDampedDot);
			float invDistCubed = invDist * invDist * invDist;
			return delta * invDistCubed * 0.0035;
		}

		vec2 repulsion(vec2 pos, vec2 attractPos)
		{
			vec2 delta = attractPos - pos;
			float targetDistance = sqrt(dot(delta, delta));
			return delta * (1.0 / (targetDistance * targetDistance * targetDistance)) * -0.000035;
		}

		void main()
		{
			// Current SSBO index
			uint index = gl_GlobalInvocationID.x;
			// Don't try to write beyond particle count
			if (index >= ubo.particleCount)
				return;

			// Read position and velocity
			vec2 vVel = particles[index].vel.xy;
			vec2 vPos = particles[index].pos.xy;

			vec2 destPos = vec2(ubo.destX, ubo.destY);

			vec2 delta = destPos - vPos;
			float targetDistance = sqrt(dot(delta, delta));
			vVel += repulsion(vPos, destPos.xy) * 0.05;

			// Move by velocity
			vPos += vVel * ubo.deltaT;

			// collide with boundary
			if ((vPos.x < -1.0) || (vPos.x > 1.0) || (vPos.y < -1.0) || (vPos.y > 1.0))
				vVel = (-vVel * 0.1) + attraction(vPos, destPos) * 12;
			else
				particles[index].pos.xy = vPos;

			// Write back
			particles[index].vel.xy = vVel;
			particles[index].vel.z += 0.02 * ubo.deltaT;
			if (particles[index].vel.z > 1.0)
				particles[index].vel.z -= 1.0;
		}
	)";



		const char* vsCode = R"(
			#version 450

			layout (location = 0) in vec2 inPos;
			layout (location = 1) in vec3 inVel;

			layout (location = 0) out vec4 outColor;
			layout (location = 1) out float outGradientPos;

			out gl_PerVertex
			{
				vec4 gl_Position;
				float gl_PointSize;
			};

			void main () 
			{
			  gl_PointSize = 8.0;
			  outColor = vec4(0.035);
			  outGradientPos = inVel.z;
			  gl_Position = vec4(inPos.xy, 1.0, 1.0);
			}
		)";

		const char* fsCode = R"(
		#version 450

		layout (binding = 0) uniform sampler2D samplerColorMap;
		layout (binding = 1) uniform sampler1D samplerGradientRamp;

		layout (location = 0) in vec4 inColor;
		layout (location = 1) in float inGradientPos;

		layout (location = 0) out vec4 outFragColor;

		void main () 
		{
			vec3 color = texture(samplerGradientRamp, inGradientPos).rgb;
			outFragColor.rgb = texture(samplerColorMap, gl_PointCoord).rgb * color;
		}
		)";


	};

	DEF_DEMO(ComputeShaderDemo, "Compute Shader Demo");
}