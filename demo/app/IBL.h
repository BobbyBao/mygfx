#pragma once
#include "Fwd.h"

namespace mygfx {
	
	class Texture;

	class IBL {
	public:
		void init(Texture* cubeMap);
		void filter();
	protected:
		Ref<Texture> mHdr;
		Ref<Texture> mCubeMap;
		Ref<Texture> mIrrMap;
		Ref<Texture> mLUT;
	};

}