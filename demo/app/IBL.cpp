#include "IBL.h"
#include "Framework.h"

namespace mygfx {


void IBL::filter(Texture* hdr)
{
    mHdr = hdr;



    auto textureData = TextureData::TextureCube(1024, 1024, 1, Format::R16G16B16A16_SFLOAT);
		textureData.usage = TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLED;
    
    mCubeMap = Texture::createFromData(textureData);
    mLUT = Texture::createRenderTarget(1024, 1024, Format::R16G16B16A16_SFLOAT, TextureUsage::SAMPLED);
        
    RenderTargetDesc desc {.width = 1024, .height = 1024};

    for (int i = 0; i < 6; i++) {
        auto rtv = gfxApi().createRTV(mCubeMap->getHwTexture(), i, "");
        desc.colorAttachments.push_back(rtv);
    }

    desc.colorAttachments.emplace_back(mLUT->getSRV());

    auto renderTarget = gfxApi().createRenderTarget(desc);

    auto shader = Shader::fromFile("shaders/fullscreen.vert", "shaders/tools/filter.frag");
    
    GraphicsApi& cmd = gfxApi();

    RenderPassInfo renderInfo {
        .clearFlags = TargetBufferFlags::ALL,
        .clearColor = { 0.0f, 0.0f, 0.0f, 1.0f }
    };

    renderInfo.viewport = { .left = 0, .top = 0, .width = 1024, .height = 1024 };

    cmd.beginRendering(renderTarget, renderInfo);

    cmd.bindPipelineState(shader->pipelineState);

    //cmd.pushConstant(0);

    cmd.draw(3, 1, 0, 0);

    cmd.endRendering(renderTarget);

}

} // namespace mygfx