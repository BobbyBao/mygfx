#include "Renderable.h"
#include "resource/Mesh.h"
#include "GraphicsApi.h"

namespace mygfx {
	
	Renderable::Renderable() = default;

	void Renderable::setMesh(Mesh* m) {

		mesh = m;
		primitives.clear();
		
		for (int i = 0; i < mesh->getSubMeshCount(); i++) {
			auto& subMesh = mesh->getSubMeshAt(i);
			auto& primitive = primitives.emplace_back();
			primitive.renderPrimitive = mesh->renderPrimitives[i];
			primitive.material = subMesh.material;
		}
	}
}