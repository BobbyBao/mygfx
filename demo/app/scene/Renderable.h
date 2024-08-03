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
		Ref<Mesh> mesh;
	protected:
		void addToScene() override;
		void removeFromScene() override;
	};

}