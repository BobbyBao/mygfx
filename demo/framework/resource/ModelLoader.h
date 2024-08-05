#pragma once
#include "Fwd.h"
#include "core/MathTypes.h"
#include "GraphicsDefs.h"

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

	class ModelLoader {
	public:
		ModelLoader() = default;

		Ref<Node> load(const String& fileName);

		bool hasSkin() const;
	protected:
		void loadScene();
		void loadNode(Node* parent, cgltf_node& node, float globalscale);
		void loadImages();
		void loadMaterials();
		Texture* getTexture(size_t index);
		Material* getMaterial(size_t index, bool skined, const DefineList* marcos);
		Ref<Material> getDefaultMaterial(const DefineList* marcos);

		VertexAttribute mVertexAttribute = VertexAttribute::NONE;
		float mScale = 1.0f;
		bool mEnableSkin = true;
		Ref<Shader> mShader;
		cgltf_data* mGltfModel = nullptr;
		Ref<Node> mRootNode;
		std::vector<Ref<Texture>> mTextures;
		std::vector<Ref<Material>> mMaterials;
		std::vector<Node*> mNodes;
	};

}