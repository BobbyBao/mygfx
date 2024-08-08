#pragma once
#include "Node.h"


namespace mygfx {
	
	class Mesh;
	class HwRenderPrimitive;
	class Material;
	
	struct Primitive {
		HwRenderPrimitive* renderPrimitive = nullptr;
		Material* material = nullptr;
		uint32_t primitiveUniforms = 0;
	};

	class Renderable : public Node {
	public:
		Renderable();

		void setMesh(Mesh* m);

		Vector<Primitive> primitives;

		static Ref<Renderable> createCube(float size);

	protected:
		Node* createNode() override;
		void cloneProcess(Node* destNode) override;
		void addToScene() override;
		void removeFromScene() override;
		Ref<Mesh> mMesh;
	};

}