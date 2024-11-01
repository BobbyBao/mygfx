#include "DemoApp.h"
#include "resource/Mesh.h"
#include "resource/Texture.h"

namespace mygfx::demo {

class DescriptorSetDemo : public Demo {
public:
    Ref<Mesh> mMesh;
    Ref<Shader> mShader;
    Ref<HwDescriptorSet> mDescriptorSet;
    void start() override
    {
        mMesh = Mesh::createCube();

        mShader = new Shader(vsCode, fsCode);
        mShader->setVertexInput({ Format::R32G32B32_SFLOAT, Format::END,
            Format::R32G32_SFLOAT, Format::END,
            Format::R32G32B32_SFLOAT });

        
        std::array<DescriptorSetLayoutBinding, 3> bindings = {
            DescriptorSetLayoutBinding {
                .binding = 0,
                .descriptorType = DescriptorType::UNIFORM_BUFFER_DYNAMIC,
                .descriptorCount = 1,
                .stageFlags = ShaderStage::VERTEX },

            DescriptorSetLayoutBinding {
                .binding = 1,
                .descriptorType = DescriptorType::UNIFORM_BUFFER_DYNAMIC,
                .descriptorCount = 1,
                .stageFlags = ShaderStage::VERTEX },

            DescriptorSetLayoutBinding {
                .binding = 2,
                .descriptorType = DescriptorType::COMBINED_IMAGE_SAMPLER,
                .descriptorCount = 1,
                .stageFlags = ShaderStage::FRAGMENT },
        };

        mDescriptorSet = gfxApi().createDescriptorSet(Span<DescriptorSetLayoutBinding>(bindings));
        
        gfxApi().updateDescriptorSet(mDescriptorSet, 0, sizeof(mat4))
            .updateDescriptorSet(mDescriptorSet, 1, sizeof(mat4))
            .updateDescriptorSet(mDescriptorSet, 2, Texture::Green->getSRV());
    }

    void draw(GraphicsApi& cmd) override
    {
        auto w = mApp->getWidth();
        auto h = mApp->getHeight();
        float aspect = w / (float)h;
        auto vp = glm::ortho(-aspect, aspect, 1.0f, -1.0f, -1.0f, 1.0f);

        uint32_t perView = gfxApi().allocConstant(vp);

        auto world = identity<mat4>();

        uint32_t perObject = gfxApi().allocConstant(world);
        
        auto ds = cmd.allocate<HwDescriptorSet*>(1);
        ds[0] = mDescriptorSet;
        cmd.bindPipelineState(mShader->pipelineState);        
        cmd.bindDescriptorSets(ds, Uniforms{ perView, perObject });

        for (auto& prim : mMesh->renderPrimitives) {
            cmd.drawPrimitive(prim, 1, 0);
        }
    }

    const char* vsCode = R"(
			#version 450
			
			layout (binding = 0) uniform PerView {
				mat4 viewProj;
			} frameUniforms;

			layout (binding = 1) uniform PerRenderable {
				mat4 world;
			} objectUniforms;

			layout(location = 0) in vec3 inPos;
			layout(location = 1) in vec2 inUV;
			layout(location = 2) in vec3 inNorm;

			layout(location = 0) out vec2 outUV;
			layout(location = 1) out vec3 outNorm;

			void main()
			{
				outUV = inUV;
				outNorm = inNorm;
				gl_Position = frameUniforms.viewProj * objectUniforms.world * vec4(inPos.xyz, 1.0);
			}
		)";

    const char* fsCode = R"(
			#version 450

			#extension GL_EXT_nonuniform_qualifier : require

			layout(binding = 2) uniform sampler2D baseColorMap;

			layout(location = 0) in vec2 inUV;
			layout(location = 1) in vec3 inNorm;

			layout(location = 0) out vec4 outFragColor;

			void main()
			{
				outFragColor = texture(baseColorMap, inUV);
			}
		)";
};

DEF_DEMO(DescriptorSetDemo, "DescriptorSet Demo");
}