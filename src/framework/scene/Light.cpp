#include "Light.h"
#include "Scene.h"

namespace mygfx {

Light::Light() = default;

Object* Light::createObject()
{
    return new Light();
}

void Light::cloneProcess(Object* destObj)
{
    Component::cloneProcess(destObj);

    Light* light = (Light*)destObj;
    light->mColor = mColor;
    light->mIntensity = mIntensity;
    light->mType = mType;
    light->mRange = mRange;
    light->mSpotInnerConeAngle = mSpotInnerConeAngle;
    light->mSpotOuterConeAngle = mSpotOuterConeAngle;
}
    
void Light::onAddToScene(Scene* scene) {
    scene->mLights.insert(this);
}
   
void Light::onRemoveFromScene(Scene* scene) {
    scene->mLights.erase(this);
}

}