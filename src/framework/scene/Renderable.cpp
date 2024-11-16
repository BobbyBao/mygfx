#include "Renderable.h"
#include "GraphicsApi.h"
#include "Scene.h"

namespace mygfx {

Renderable::Renderable() = default;

void Renderable::cloneProcess(Object* destObj)
{
    Component::cloneProcess(destObj);

    Renderable* dest = static_cast<Renderable*>(destObj);

    dest->mRenderer = mRenderer;
    dest->mRenderableType = mRenderableType;
    dest->mBoundingBox = mBoundingBox;
    dest->mSkinning = mSkinning;
    dest->mMorphing = mMorphing;
}

void Renderable::onAddToScene(Scene* scene)
{
    scene->mRenderables.insert(this);
}

void Renderable::onRemoveFromScene(Scene* scene)
{
    scene->mRenderables.erase(this);
}

CustomRenderable::CustomRenderable()
{
    mRenderer = this;
}

}