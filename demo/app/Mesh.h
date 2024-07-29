#pragma once
#include "GraphicsHandles.h"
#include "MathTypes.h"

namespace mygfx {

	struct SubMesh {

		SubMesh() = default;
		SubMesh(VertexData* vertexData);

		String name;
		Ref<VertexData> vertexData;
		DrawPrimitiveCommand drawArgs;
	};

	class Mesh : public RefCounted {
	public:
		Mesh();
		~Mesh();

		auto& getName() const { return mName; }
		void setName(const String& name) { mName = name; }

		uint32_t getSubMeshCount() const { return (uint32_t)mSubMeshes.size(); }
		Vector<SubMesh>& getSubMeshes() { return mSubMeshes; }
		const Vector<SubMesh>& getSubMeshes() const { return mSubMeshes; }
		const SubMesh& getSubMeshAt(uint32_t index) const { return mSubMeshes.at(index); }

		auto& getBoundingBox() const { return mBoundingBox; }
		void setBoundingBox(const Aabb& aabb) { mBoundingBox = aabb; }

		static Mesh* createPlane(float size = 1.0f);
		static Mesh* createCube(float size = 1.0f);
	private:
		String mName;
		Vector<SubMesh> mSubMeshes;
		Aabb mBoundingBox;
	};

}
