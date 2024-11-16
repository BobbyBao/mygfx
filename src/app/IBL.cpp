#include "IBL.h"
#include "Framework.h"

namespace mygfx {


void IBL::filter(Texture* hdr)
{
    mHdr = hdr;

    const uint32_t IMAGE_SIZE = 1024;

    GraphicsApi& cmd = gfxApi();

    RenderPassInfo renderInfo {
        .clearFlags = TargetBufferFlags::ALL,
        .clearColor = { 0.0f, 0.0f, 0.0f, 1.0f }
    };

    renderInfo.viewport = { .left = 0, .top = 0, .width = IMAGE_SIZE, .height = IMAGE_SIZE };

    {
        auto textureData = TextureData::textureCube(IMAGE_SIZE, IMAGE_SIZE, 1, Format::R16G16B16A16_SFLOAT);
        textureData.usage = TextureUsage::TRANSFER_DST | TextureUsage::SAMPLED;
        textureData.mipMapCount = Texture::maxLevelCount(IMAGE_SIZE);

        mCubeMap = Texture::createFromData(textureData);

        auto panorama_to_cubemap = Shader::fromFile("shaders/fullscreen.vert", "shaders/tools/panorama_to_cubemap.frag");
        auto renderTexture = Texture::createRenderTexture(IMAGE_SIZE, IMAGE_SIZE, Format::R16G16B16A16_SFLOAT, TextureUsage::SAMPLED | TextureUsage::TRANSFER_SRC);

        struct {
            int u_currentFace;
            int u_panorama;
        } pushConst;

        pushConst.u_panorama = mHdr->index();

        RenderTargetDesc desc { .width = IMAGE_SIZE, .height = IMAGE_SIZE };

        desc.colorAttachments.emplace_back(renderTexture->getSRV());
        auto renderTarget = gfxApi().createRenderTarget(desc);

        for (int i = 0; i < 6; i++) {
            cmd.beginRendering(renderTarget, renderInfo);

            cmd.bindPipelineState(panorama_to_cubemap->pipelineState);

            pushConst.u_currentFace = i;

            cmd.pushConstant(0, &pushConst, sizeof(pushConst));

            cmd.draw(3, 1, 0, 0);

            cmd.endRendering(renderTarget);
            
            //todo: copy image
        }

    }

    {		
        enum class Distribution : unsigned int 
	    {
		    Lambertian = 0,
		    GGX = 1,
		    Charlie = 2
	    };

        struct PushConstant
	    {
		    float roughness = 0.f;
		    uint32_t sampleCount = 1u;
		    uint32_t mipLevel = 1u;
		    uint32_t width = 1024u;
		    float lodBias = 0.f;
		    Distribution distribution = Distribution::Lambertian;
	    }pushConst;

        auto textureData = TextureData::textureCube(64, 64, 1, Format::R16G16B16A16_SFLOAT);
        textureData.usage = TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLED;
        mIrrMap = Texture::createFromData(textureData);
        mLUT = Texture::createRenderTexture(IMAGE_SIZE, IMAGE_SIZE, Format::R16G16B16A16_SFLOAT, TextureUsage::SAMPLED);
        
        RenderTargetDesc desc { .width = IMAGE_SIZE, .height = IMAGE_SIZE };

        for (int i = 0; i < 6; i++) {
            auto rtv = gfxApi().createRTV(mIrrMap->getHwTexture(), i, "");
            desc.colorAttachments.push_back(rtv);
        }

        desc.colorAttachments.emplace_back(mLUT->getSRV());

        auto renderTarget = gfxApi().createRenderTarget(desc);

        auto shader = Shader::fromFile("shaders/fullscreen.vert", "shaders/tools/filter.frag");
    
        cmd.beginRendering(renderTarget, renderInfo);

        cmd.bindPipelineState(shader->pipelineState);

        cmd.pushConstant(0, &pushConst, sizeof(pushConst));

        cmd.draw(3, 1, 0, 0);

        cmd.endRendering(renderTarget);
    }


}

} // namespace mygfx