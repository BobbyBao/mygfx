#include "IBL.h"
#include "Framework.h"

namespace mygfx {

void IBL::init(Texture* cubeMap)
{
    mCubeMap = cubeMap;
}

void IBL::filter()
{
    auto textureData = TextureData::TextureCube(1024, 1024, 1, Format::R16G16B16A16_SFLOAT);
		textureData.usage = TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLED;
    
    mCubeMap = Texture::createFromData(textureData);

    mLUT = Texture::createRenderTarget(1024, 1024, Format::R16G16B16A16_SFLOAT,
        TextureUsage::SAMPLED);
        
    RenderTargetDesc desc {.width = 1024,
        .height = 1024
    };

    desc.colorAttachments.emplace_back(mLUT->getSRV());

    auto renderTarget = gfxApi().createRenderTarget(desc);

    auto shader = Shader::fromFile("shaders/fullscreen.vert", "shaders/tools/filter.frag");
}

} // namespace mygfx