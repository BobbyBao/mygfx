#include "Renderable.h"
#include "GraphicsApi.h"
#include "Scene.h"

namespace mygfx {

Renderable::Renderable() = default;

void Renderable::onAddToScene(Scene* scene)
{
    scene->mRenderables.insert(this);
}

void Renderable::onRemoveFromScene(Scene* scene)
{
    scene->mRenderables.erase(this);
}


}