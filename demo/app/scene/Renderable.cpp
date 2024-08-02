#include "Renderable.h"
#include "resource/Mesh.h"
#include "GraphicsApi.h"

namespace mygfx {

	void Renderable::setMesh(Mesh* m) {

		mesh = m;

		for (auto& rp : mesh->renderPrimitives) {
			auto& primitive = primitives.emplace_back();
			primitive.renderPrimitive = rp;

		}
	}
}