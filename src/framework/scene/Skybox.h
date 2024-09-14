#pragma once

#include "Renderable.h"

namespace mygfx {

class Texture;
class Shader;

class Skybox : public Renderable {
public:
    Skybox();

    Texture* getCubeMap() const { return mCubeMap; }
    void setCubeMap(Texture* tex);

    Texture* getIrrMap() const { return mIrrMap; }
    void setIrrMap(Texture* tex);

    Texture* getGGXLUT() const { return mGGXLUT; }

protected:
    Object* createObject() override;
    void cloneProcess(Object* destNode) override;
    void onAddToScene(Scene* scene) override;
    void onRemoveFromScene(Scene* scene) override;

    Ref<Texture> mCubeMap;
    Ref<Texture> mIrrMap;
    Ref<Texture> mGGXLUT;
    inline static Ref<Shader> msDefaultShader;
};

}