#pragma once

#include "Node.h"

namespace mygfx {

class Light;
class Renderable;
class Skybox;

class Scene : public Node {
public:
    Scene();

    Ref<Node> instantiate(Node* model, Node* parent = nullptr);
    Ref<Node> instantiate(Node* model, Node* parent, const vec3& pos, const Quaternion& rot = identity<Quaternion>(), const vec3& scale = vec3(1.0f));
    Ref<Node> instantiate(Node* model, const vec3& pos, const Quaternion& rot = identity<Quaternion>(), const vec3& scale = vec3(1.0f));
    
    Skybox* getSkybox() const { return mSkybox; }
    auto& getRenderables() const { return mRenderables; }
    auto& getLights() const { return mLights; }

protected:
    Skybox* mSkybox = nullptr;
    HashSet<Renderable*> mRenderables;
    HashSet<Light*> mLights;

    friend class Skybox;
    friend class Renderable;
    friend class Light;

};

}