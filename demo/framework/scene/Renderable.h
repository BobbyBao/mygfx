#pragma once
#include "Component.h"


namespace mygfx {
	
	class Mesh;
	class HwRenderPrimitive;
	class Material;
	
	struct Primitive {
		HwRenderPrimitive* renderPrimitive = nullptr;
		Material* material = nullptr;
		uint32_t primitiveUniforms = 0;
	};

	class Renderable : public Component {
	public:
		Renderable();

		void setMesh(Mesh* m);

		Vector<Primitive> primitives;

		static Ref<Node> createCube(float size);

	protected:
		Object* createObject() override;
		void cloneProcess(Object* destNode) override;
		void onAddToScene(Scene* scene) override;
		void onRemoveFromScene(Scene* scene) override;	
		
		mutable bool mSkinning : 1 = false;
		mutable bool mMorphing : 1 = false;
		Ref<Mesh> mMesh;
	};

}