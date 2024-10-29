#include "Renderable.h"
#include "GraphicsApi.h"
#include "Scene.h"

namespace mygfx {

Renderable::Renderable() = default;

void Renderable::cloneProcess(Object* destObj)
{
    Component::cloneProcess(destObj);

    Renderable* dest = static_cast<Renderable*>(destObj);

    dest->mCustomRenderer = mCustomRenderer;
    dest->mRenderQueue = mRenderQueue;
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


}