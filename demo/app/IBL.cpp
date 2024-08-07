#include "IBL.h"
#include "Framework.h"

namespace mygfx {
    
	void IBL::init(Texture* cubeMap) {

		mCubeMap = cubeMap;
	}
		
	void IBL::filter() {

		
		mLUT = Texture::createRenderTarget(1024, 1024, Format::R8G8B8A8_UNORM, TextureUsage::SAMPLED);
		auto renderTarget = gfxApi().createRenderTarget(
			{ .width = 1024,
			.height = 1024, 
			.colorAttachments = {mLUT->getHwTexture()} });


	}

}