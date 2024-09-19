#pragma once
#include "GraphicsDefs.h"
#include "core/Fwd.h"
#include "core/Maths.h"

struct cgltf_data;
struct cgltf_node;
struct cgltf_skin;
struct cgltf_primitive;
struct cgltf_camera;
struct cgltf_light;

namespace mygfx {

class Shader;
class Texture;
class Material;
class Node;
class Camera;
class Light;

class ModelLoader {
public:
    ModelLoader() = default;

    Ref<Node> load(const String& fileName);
    bool hasSkin() const;

    auto& getBoundingBox() const
    {
        return mBoundingBox;
    }

protected:
    void loadScene();
    void loadMaterials();
    void loadImages();
    void loadNode(Node* parent, cgltf_node& node, float globalscale);
    void createCamera(Camera* camera, cgltf_camera* c);
    void createLight(Light* light, cgltf_light* l);
    Texture* getTexture(size_t index);
    Material* getMaterial(size_t index, bool skined, const DefineList* marcos);
    Ref<Material> getDefaultMaterial(const DefineList* marcos);

    VertexAttribute mVertexAttribute = VertexAttribute::NONE;
    float mScale = 1.0f;
    bool mEnableSkin = true;
    Ref<Shader> mShader;
    Path mFilePath;
    cgltf_data* mGltfModel = nullptr;
    Ref<Node> mRootNode;
    Vector<bool> mSrgb;
    Vector<Ref<Texture>> mTextures;
    Vector<Ref<Material>> mMaterials;
    Vector<Node*> mNodes;
    Aabb mBoundingBox;
};

}