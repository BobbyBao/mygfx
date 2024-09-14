#pragma once
#include "core/Fwd.h"

namespace mygfx {

class Texture;

class IBL {
public:
    void filter(Texture* hdr);

protected:
    Ref<Texture> mHdr;
    Ref<Texture> mCubeMap;
    Ref<Texture> mIrrMap;
    Ref<Texture> mLUT;
};

}