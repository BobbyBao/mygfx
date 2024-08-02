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

		const Vector<SubMesh>& getSubMeshes() const { return mSubMeshes; }
		const SubMesh& getSubMeshAt(uint32_t index) const { return mSubMeshes.at(index); }
		void addSubMesh(VertexData* vertexData);
		void addSubMesh(VertexData* vertexData, const DrawPrimitiveCommand& drawArgs);

		auto& getBoundingBox() const { return mBoundingBox; }
		void setBoundingBox(const Aabb& aabb) { mBoundingBox = aabb; }

		static Mesh* createPlane(float size = 1.0f);
		static Mesh* createCube(float size = 1.0f);

		Vector<Ref<HwRenderPrimitive>> renderPrimitives;
	private:
		String mName;
		Vector<SubMesh> mSubMeshes;
		Aabb mBoundingBox;
	};

}
