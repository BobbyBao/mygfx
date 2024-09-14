#pragma once
#include "GraphicsHandles.h"
#include "core/Maths.h"
#include "core/Resource.h"

namespace mygfx {

class Material;

struct SubMesh {

    SubMesh() = default;
    SubMesh(VertexData* vertexData);

    String name;
    Ref<VertexData> vertexData;
    DrawPrimitiveCommand drawArgs;
    Ref<Material> material;
    Aabb boundingBox;
};

class Mesh : public Resource {
public:
    Mesh();
    ~Mesh();

    uint32_t getSubMeshCount() const { return (uint32_t)mSubMeshes.size(); }

    const Vector<SubMesh>& getSubMeshes() const { return mSubMeshes; }
    const SubMesh& getSubMeshAt(uint32_t index) const { return mSubMeshes.at(index); }
    SubMesh& addSubMesh(VertexData* vertexData, Material* mat = nullptr);
    SubMesh& addSubMesh(VertexData* vertexData, const DrawPrimitiveCommand& drawArgs, Material* mat = nullptr);
    void setMaterial(uint32_t index, Material* mat);
    void setMaterial(Material* mat);

    auto& getBoundingBox() const { return mBoundingBox; }
    void setBoundingBox(const Aabb& aabb) { mBoundingBox = aabb; }

    static Mesh* createFullScreen();
    static Mesh* createPlane(float size = 1.0f);
    static Mesh* createCube(float size = 1.0f, VertexAttribute attributes = VertexAttribute::POSITION | VertexAttribute::NORMAL | VertexAttribute::UV_0);

    Vector<Ref<HwRenderPrimitive>> renderPrimitives;

private:
    Vector<SubMesh> mSubMeshes;
    Aabb mBoundingBox;
};

}
