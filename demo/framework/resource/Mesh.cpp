#include "Mesh.h"
#include "GraphicsApi.h"

namespace mygfx {

	SubMesh::SubMesh(VertexData* vertexData) {
		this->vertexData = vertexData;
		if (vertexData->indexBuffer) {
			drawArgs.indexCount = (uint32_t)vertexData->indexBuffer->count();
		} else {
			drawArgs.vertexCount = (uint32_t)vertexData->vertexBuffers[0]->count();
		}
	}

	Mesh::Mesh() {
	}

	Mesh::~Mesh() {
	}

	SubMesh& Mesh::addSubMesh(VertexData* vertexData, Material* mat) {
		auto& subMesh = mSubMeshes.emplace_back(vertexData);
		subMesh.material = mat;
		renderPrimitives.emplace_back(gfxApi().createRenderPrimitive(subMesh.vertexData, subMesh.drawArgs));
		return subMesh;
	}
	
	SubMesh& Mesh::addSubMesh(VertexData* vertexData, const DrawPrimitiveCommand& drawArgs, Material* mat) {
		auto& subMesh = mSubMeshes.emplace_back(vertexData);
		subMesh.drawArgs = drawArgs;
		subMesh.material = mat;
		renderPrimitives.emplace_back(gfxApi().createRenderPrimitive(vertexData, drawArgs));
		return subMesh;
	}

	void Mesh::setMaterial(uint32_t index, Material* mat) {
		if (index < mSubMeshes.size()) {
			mSubMeshes[index].material = mat;
		}
	}

	void Mesh::setMaterial(Material* mat) {
		for (auto& subMesh : mSubMeshes) {
			subMesh.material = mat;
		}
	}

	Mesh* Mesh::createPlane(float size)
	{
		size = size / 2;

		float3 pos[] =
		{
			{ -size, 0.0f, -size },
			{ size, 0.0f, -size },
			{ size, 0.0f, size },
			{ -size, 0.0f, size },
		};

		float2 tex[] =
		{
			{ 0.0f, 0.0f },
			{ 1.0f, 0.0f },
			{ 1.0f, 1.0f },
			{ 0.0f, 1.0f },
		};

		float3 norm[] =
		{
			{ 0.0f, 1.0, 0.0f },
			{ 0.0f, 1.0, 0.0f },
			{ 0.0f, 1.0, 0.0f },
			{ 0.0f, 1.0, 0.0f },
		};

		uint32_t  indices[] = { 0, 2, 1, 0, 3, 2 };

		uint32_t vertexCount = (uint32_t)std::size(pos);
		
		auto vertexData = new VertexData();
		auto vbPos = gfxApi().createBuffer1(BufferUsage::VERTEX, MemoryUsage::GPU_ONLY, vertexCount, pos);
		vbPos->extra = (uint16_t)VertexAttribute::POSITION;
		vertexData->vertexBuffers.push_back(vbPos);

		auto vbTex = gfxApi().createBuffer1(BufferUsage::VERTEX, MemoryUsage::GPU_ONLY, vertexCount, tex);
		vbTex->extra = (uint16_t)VertexAttribute::UV_0;
		vertexData->vertexBuffers.push_back(vbTex);

		auto vbNorm = gfxApi().createBuffer1(BufferUsage::VERTEX, MemoryUsage::GPU_ONLY, vertexCount, norm);
		vbTex->extra = (uint16_t)VertexAttribute::NORMAL;
		vertexData->vertexBuffers.push_back(vbNorm);
		
		vertexData->indexBuffer = gfxApi().createBuffer1(BufferUsage::INDEX, MemoryUsage::GPU_ONLY, (uint32_t)std::size(indices), indices);

		Mesh* mesh = new Mesh();
		mesh->addSubMesh(vertexData);
		mesh->setBoundingBox({ {-size, -0.01f, -size}, {size, 0.01f, size} });
		return mesh;
	}

	Mesh* Mesh::createCube(float size, VertexAttribute attributes)
	{
		const int NUM_VERTICES = 4 * 6; // 4 vertices per side * 6 sides
		const int NUM_ENTRIES_PER_VERTEX = 8;
		const int NUM_VERTEX_ENTRIES = NUM_VERTICES * NUM_ENTRIES_PER_VERTEX;
		const int NUM_INDICES = 3 * 2 * 6; // 3 indices per face * 2 faces per side * 6 sides

		const float CUBE_SIZE = size;
		const float CUBE_HALF_SIZE = CUBE_SIZE / 2.0f;

		// Create 4 vertices per side instead of 6 that are shared for the whole cube.
		// The reason for this is with only 6 vertices the normals will look bad
		// since each vertex can "point" in a different direction depending on the face it is included in.

		float3 pos[] =
		{
			// front side
			{ -CUBE_HALF_SIZE, -CUBE_HALF_SIZE, CUBE_HALF_SIZE},
			{CUBE_HALF_SIZE, -CUBE_HALF_SIZE, CUBE_HALF_SIZE},
			{CUBE_HALF_SIZE,  CUBE_HALF_SIZE, CUBE_HALF_SIZE},
			{-CUBE_HALF_SIZE,  CUBE_HALF_SIZE, CUBE_HALF_SIZE},

			// back side
			{CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE },
			{-CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE},
			{-CUBE_HALF_SIZE, CUBE_HALF_SIZE, -CUBE_HALF_SIZE},
			{CUBE_HALF_SIZE, CUBE_HALF_SIZE, -CUBE_HALF_SIZE},

			// left side
			{-CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE},
			{-CUBE_HALF_SIZE, -CUBE_HALF_SIZE, CUBE_HALF_SIZE},
			{-CUBE_HALF_SIZE, CUBE_HALF_SIZE, CUBE_HALF_SIZE},
			{-CUBE_HALF_SIZE, CUBE_HALF_SIZE, -CUBE_HALF_SIZE},

			// right side
			{CUBE_HALF_SIZE, -CUBE_HALF_SIZE, CUBE_HALF_SIZE},
			{CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE},
			{CUBE_HALF_SIZE, CUBE_HALF_SIZE, -CUBE_HALF_SIZE},
			{CUBE_HALF_SIZE, CUBE_HALF_SIZE, CUBE_HALF_SIZE},

			// up side
			{-CUBE_HALF_SIZE, CUBE_HALF_SIZE, CUBE_HALF_SIZE},
			{CUBE_HALF_SIZE, CUBE_HALF_SIZE, CUBE_HALF_SIZE},
			{CUBE_HALF_SIZE, CUBE_HALF_SIZE, -CUBE_HALF_SIZE},
			{-CUBE_HALF_SIZE, CUBE_HALF_SIZE, -CUBE_HALF_SIZE},

			// down side
			{-CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE},
			{CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE},
			{CUBE_HALF_SIZE, -CUBE_HALF_SIZE, CUBE_HALF_SIZE},
			{-CUBE_HALF_SIZE, -CUBE_HALF_SIZE, CUBE_HALF_SIZE},
		};

		float2 tex[] =
		{
			{0,1}, {1,1}, {1,0}, {0,0},
			{0,1}, {1,1}, {1,0}, {0,0},
			{0,1}, {1,1}, {1,0}, {0,0},
			{0,1}, {1,1}, {1,0}, {0,0},
			{0,1}, {1,1}, {1,0}, {0,0},
			{0,1}, {1,1}, {1,0}, {0,0},
		};

		float3 norm[] =
		{ 
			{0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1},
			{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

			{-1, 0, 0}, {-1, 0, 0}, {-1, 0,0}, {-1, 0, 0},
			{1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0},

			{0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0},
			{0, -1, 0}, {0, -1, 0}, {0, -1, 0}, {0, -1, 0},
		};

		uint32_t indices[] = {
			// front
			0,1,2,
			0,2,3,

			// back
			4,5,6,
			4,6,7,

			// left
			8,9,10,
			8,10,11,

			// right
			12,13,14,
			12,14,15,

			// up
			16,17,18,
			16,18,19,

			// down
			20,21,22,
			20,22,23
		};

		uint32_t vertexCount = (uint32_t)std::size(pos);

		auto vertexData = new VertexData();
		auto vbPos = gfxApi().createBuffer1(BufferUsage::VERTEX, MemoryUsage::GPU_ONLY, vertexCount, pos);
		vbPos->extra = (uint16_t)VertexAttribute::POSITION;
		vertexData->vertexBuffers.push_back(vbPos);

		if (any(attributes & VertexAttribute::UV_0)) {
			auto vbTex = gfxApi().createBuffer1(BufferUsage::VERTEX, MemoryUsage::GPU_ONLY, vertexCount, tex);
			vbTex->extra = (uint16_t)VertexAttribute::UV_0;
			vertexData->vertexBuffers.push_back(vbTex);
		}

		if (any(attributes & VertexAttribute::NORMAL)) {
			auto vbNorm = gfxApi().createBuffer1(BufferUsage::VERTEX, MemoryUsage::GPU_ONLY, vertexCount, norm);
			vbNorm->extra = (uint16_t)VertexAttribute::NORMAL;
			vertexData->vertexBuffers.push_back(vbNorm);
		}

		vertexData->indexBuffer = gfxApi().createBuffer1(BufferUsage::INDEX, MemoryUsage::GPU_ONLY, (uint32_t)std::size(indices), indices);
		Mesh* mesh = new Mesh();
		mesh->addSubMesh(vertexData);
		mesh->setBoundingBox({ {-CUBE_HALF_SIZE, -CUBE_HALF_SIZE, -CUBE_HALF_SIZE}, {CUBE_HALF_SIZE, CUBE_HALF_SIZE, CUBE_HALF_SIZE} });
		return mesh;
	}

}