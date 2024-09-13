#include "ModelLoader.h"
#include "GraphicsApi.h"
#include "Material.h"
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"
#include "scene/Camera.h"
#include "scene/Light.h"
#include "scene/Node.h"
#include "scene/Renderable.h"

#define CGLTF_IMPLEMENTATION
#include <cgltf/cgltf.h>

#include "utils/FileUtils.h"
#include "utils/Log.h"
#include <filesystem>

namespace mygfx {

using Path = std::filesystem::path;

template <typename T>
const T* getBufferData(const cgltf_accessor* accessor)
{
    assert(sizeof(T) == accessor->stride);
    return reinterpret_cast<const T*>(((uint8_t*)accessor->buffer_view->buffer->data) + accessor->offset + accessor->buffer_view->offset);
}

Ref<Node> ModelLoader::load(const String& fileName)
{
    cgltf_options options = {};

    auto content = FileUtils::readAll(fileName);

    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse(&options, content.data(), (uint32_t)content.size(), &data);

    if (result != cgltf_result_success) {
        LOG_ERROR("Could not load glTF file {}, error : {}", fileName, (int)result);
        cgltf_free(data);
        return nullptr;
    }

    std::filesystem::path filePath = fileName;

    auto gltfPath = FileUtils::convertPath(filePath);
    mFilePath = gltfPath.parent_path();

    result = cgltf_load_buffers(&options, data, gltfPath.string().c_str());
    if (result != cgltf_result_success) {
        LOG_ERROR("Unable to load resources.");
        cgltf_free(data);
        return nullptr;
    }

    mGltfModel = data;

    loadMaterials();

    loadImages();

    loadScene();

    cgltf_free(data);

    return mRootNode;
}

void ModelLoader::loadScene()
{
    mRootNode = new Node();

    auto& gltfModel = *mGltfModel;
    mNodes.resize(gltfModel.nodes_count);

    for (int s = 0; s < gltfModel.scenes_count; s++) {
        const auto& scene = gltfModel.scenes[s];
        for (size_t i = 0; i < scene.nodes_count; i++) {
            auto& node = *scene.nodes[i];
            loadNode(mRootNode, node, mScale);
        }
    }
}

bool ModelLoader::hasSkin() const
{
    return mEnableSkin && mGltfModel->skins_count > 0;
}

void ModelLoader::loadNode(Node* parent, cgltf_node& node, float globalscale)
{
    Node* newNode = parent->createChild<Node>(node.name ? node.name : "UnNamed");
    Renderable* renderable = nullptr;
    Camera* camera = nullptr;
    Light* light = nullptr;
    if (node.mesh) {
        renderable = newNode->addComponent<Renderable>();
    } else if (node.camera) {
        camera = newNode->addComponent<Camera>();
        createCamera(camera, node.camera);
    } else if (node.light) {
        light = newNode->addComponent<Light>();
        createLight(light, node.light);
    }

    if (node.has_mesh_gpu_instancing) {
        // todo:mesh gpu instancing
    }

    // Generate local node matrix
    if (node.has_matrix) {
        mat4 local {
            { (float)node.matrix[0], (float)node.matrix[1], (float)node.matrix[2], (float)node.matrix[3] },
            { (float)node.matrix[4], (float)node.matrix[5], (float)node.matrix[6], (float)node.matrix[7] },
            { (float)node.matrix[8], (float)node.matrix[9], (float)node.matrix[10], (float)node.matrix[11] },
            { (float)node.matrix[12], (float)node.matrix[13], (float)node.matrix[14], (float)node.matrix[15] }
        };
        newNode->setTransform(local);
    } else {
        vec3 translation(node.translation[0], node.translation[1], node.translation[2]);
        quat q((float)node.rotation[3], (float)node.rotation[0], (float)node.rotation[1], (float)node.rotation[2]);
        vec3 scale((float)node.scale[0], (float)node.scale[1], (float)node.scale[2]);
        newNode->setTRS(translation, q, scale);
    }

    size_t nodeIndex = &node - mGltfModel->nodes;
    mNodes[nodeIndex] = newNode;

    // Node with children
    if (node.children_count > 0) {
        for (auto i = 0; i < node.children_count; i++) {
            loadNode(newNode, *node.children[i], globalscale);
        }
    }

    // Node contains mesh data
    if (node.mesh) {

        const auto& mesh = *node.mesh;
        Mesh* newMesh = new Mesh();
        newMesh->setName(mesh.name);
        Aabb boundingBox;
        std::vector<uint32_t> indexBuffer;

        const uint32_t numMorphTargets = (uint32_t)mesh.primitives[0].targets_count;
        bool hasSkinOrMorphing = (hasSkin() && node.skin) || numMorphTargets;
        DefineList defineList;

        auto find_attribute = [](const cgltf_primitive& primitive, cgltf_attribute_type attribute_type) -> auto {
            for (int i = 0; i < primitive.attributes_count; i++) {
                if (primitive.attributes[i].type == attribute_type) {
                    return &primitive.attributes[i];
                }
            }
            return (cgltf_attribute*)nullptr;
        };

        for (size_t j = 0; j < mesh.primitives_count; j++) {
            const cgltf_primitive& primitive = mesh.primitives[j];
            if (primitive.indices == nullptr) {
                continue;
            }

            if (numMorphTargets != primitive.targets_count) {
                LOG_ERROR("Sister primitives must all have the same number of morph targets.");
                continue;
            }

            indexBuffer.clear();

            uint32_t indexStart = 0;
            uint32_t vertexStart = 0;
            uint32_t indexCount = 0;
            uint32_t vertexCount = 0;
            float3 posMin {};
            float3 posMax {};
            bool hasSkin = false;
            auto material = primitive.material ? mMaterials[primitive.material - mGltfModel->materials] : mMaterials.back();

            Vector<Ref<HwBuffer>> vbs;
            FormatList formats;
            // Vertices
            {
                const float3* bufferPos = nullptr;
                const float3* bufferNormals = nullptr;
                const float2* bufferTexCoords = nullptr;
                const float* bufferColors = nullptr;
                const float4* bufferTangents = nullptr;
                uint32_t numColorComponents;
                const u16vec4* bufferJoints = nullptr;
                const float4* bufferWeights = nullptr;

                // Position attribute is required
                auto pos = find_attribute(primitive, cgltf_attribute_type_position);
                const cgltf_accessor& posAccessor = *pos->data;
                const cgltf_buffer_view& posView = *posAccessor.buffer_view;
                bufferPos = reinterpret_cast<const float3*>(((uint8_t*)posView.buffer->data) + posAccessor.offset + posView.offset);

                posMin = float3((float)posAccessor.min[0], (float)posAccessor.min[1], (float)posAccessor.min[2]);
                posMax = float3((float)posAccessor.max[0], (float)posAccessor.max[1], (float)posAccessor.max[2]);

                vertexCount = static_cast<uint32_t>(posAccessor.count);

                {
                    auto vb = gfxApi().createBuffer(BufferUsage::VERTEX, MemoryUsage::GPU_ONLY, vertexCount * sizeof(float3), sizeof(float3), bufferPos);
                    vb->extra = (uint16_t)VertexAttribute::POSITION;
                    vbs.push_back(vb);
                    formats += Format::R32G32B32_SFLOAT;
                }

                if (auto attr = find_attribute(primitive, cgltf_attribute_type_normal)) {
                    bufferNormals = getBufferData<float3>(attr->data);

                    auto vb = gfxApi().createBuffer(BufferUsage::VERTEX, MemoryUsage::GPU_ONLY, vertexCount * sizeof(float3), sizeof(float3), bufferNormals);
                    vb->extra = (uint16_t)VertexAttribute::NORMAL;
                    defineList.add("HAS_NORMAL_VEC3", vbs.size());
                    vbs.push_back(vb);
                    formats.append({ Format::END, Format::R32G32B32_SFLOAT });
                }

                if (auto attr = find_attribute(primitive, cgltf_attribute_type_texcoord)) {
                    const auto& accessor = *attr->data;
                    bufferTexCoords = getBufferData<float2>(attr->data);

                    if (mVertexAttribute == VertexAttribute::NONE || any(mVertexAttribute & VertexAttribute::UV_0)) {
                        auto vb = device().createBuffer(BufferUsage::VERTEX, MemoryUsage::GPU_ONLY, accessor.count * sizeof(float2), sizeof(float2), bufferTexCoords);
                        vb->extra = (uint16_t)VertexAttribute::UV_0;
                        defineList.add("HAS_TEXCOORD_0_VEC2", vbs.size());
                        vbs.push_back(vb);
                        formats.append({ Format::END, Format::R32G32_SFLOAT });
                    }

                } else {

                    if (any(mVertexAttribute & VertexAttribute::UV_0)) {
                        Vector<float2> float2Buffer(vertexCount, float2 { 0.0f });
                        auto vb = device().createBuffer(BufferUsage::VERTEX, MemoryUsage::GPU_ONLY, float2Buffer.size() * sizeof(float2), sizeof(float2), float2Buffer.data());
                        vb->extra = (uint16_t)VertexAttribute::UV_0;
                        defineList.add("HAS_TEXCOORD_0_VEC2", vbs.size());
                        vbs.push_back(vb);
                        formats.append({ Format::END, Format::R32G32_SFLOAT });
                    }
                }

                if (auto attr = find_attribute(primitive, cgltf_attribute_type_color)) {
                    const auto& accessor = *attr->data;
                    const auto& view = *accessor.buffer_view;
                    // Color buffer are either of type vec3 or vec4
                    numColorComponents = accessor.type == cgltf_type_vec3 ? 3 : 4;
                    bufferColors = reinterpret_cast<const float*>(((uint8_t*)view.buffer->data) + accessor.offset + view.offset);

                    if (mVertexAttribute == VertexAttribute::NONE || any(mVertexAttribute & VertexAttribute::COLOR)) {
                        auto vb = device().createBuffer(BufferUsage::VERTEX, MemoryUsage::GPU_ONLY, accessor.count * sizeof(float4), sizeof(float4), bufferColors);
                        vb->extra = (uint16_t)VertexAttribute::COLOR;
                        defineList.add(numColorComponents == 3 ? "HAS_COLOR_0_VEC3" : "HAS_COLOR_0_VEC4", vbs.size());
                        vbs.push_back(vb);
                        formats.append({ Format::END, numColorComponents == 3 ? Format::R32G32B32_SFLOAT : Format::R32G32B32A32_SFLOAT });
                    }

                } else {

                    if (any(mVertexAttribute & VertexAttribute::COLOR)) {
                        Vector<float4> float4Buffer(vertexCount, float4 { 1.0f });
                        auto vb = device().createBuffer(BufferUsage::VERTEX, MemoryUsage::GPU_ONLY, float4Buffer.size() * sizeof(float4), sizeof(float4), float4Buffer.data());
                        vb->extra = (uint16_t)VertexAttribute::COLOR;
                        defineList.add(numColorComponents == 3 ? "HAS_COLOR_0_VEC3" : "HAS_COLOR_0_VEC4", vbs.size());
                        vbs.push_back(vb);
                        formats.append({ Format::END, numColorComponents == 3 ? Format::R32G32B32_SFLOAT : Format::R32G32B32A32_SFLOAT });
                    }
                }

                if (auto attr = find_attribute(primitive, cgltf_attribute_type_tangent)) {
                    const auto& accessor = *attr->data;
                    const auto& view = *accessor.buffer_view;
                    if (accessor.type == cgltf_type_vec4) {
                        bufferTangents = getBufferData<float4>(&accessor);

                        auto vb = gfxApi().createBuffer(BufferUsage::VERTEX, MemoryUsage::GPU_ONLY, vertexCount * sizeof(float4), sizeof(float4), bufferTangents);
                        vb->extra = (uint16_t)VertexAttribute::TANGENTS;
                        defineList.add("HAS_TANGENT_VEC4", vbs.size());
                        vbs.push_back(vb);
                        formats.append({ Format::END, Format::R32G32B32A32_SFLOAT });

                    } else {
                        LOG_WARNING("tangent format {}", int(accessor.type));
                    }
                }

                // Indices
                {
                    const auto& accessor = *primitive.indices;
                    const auto& bufferView = *accessor.buffer_view;
                    const auto& buffer = *bufferView.buffer;

                    indexCount = static_cast<uint32_t>(accessor.count);

                    switch (accessor.component_type) {
                    case cgltf_component_type_r_32u: {
                        uint32_t* buf = new uint32_t[accessor.count];
                        memcpy(buf, &((uint8_t*)buffer.data)[accessor.offset + bufferView.offset], accessor.count * sizeof(uint32_t));
                        for (size_t index = 0; index < accessor.count; index++) {
                            indexBuffer.push_back(buf[index] + vertexStart);
                        }
                        delete[] buf;
                        break;
                    }
                    case cgltf_component_type_r_16u: {
                        uint16_t* buf = new uint16_t[accessor.count];
                        memcpy(buf, &((uint8_t*)buffer.data)[accessor.offset + bufferView.offset], accessor.count * sizeof(uint16_t));
                        for (size_t index = 0; index < accessor.count; index++) {
                            indexBuffer.push_back(buf[index] + vertexStart);
                        }
                        delete[] buf;
                        break;
                    }
                    case cgltf_component_type_r_8u: {
                        uint8_t* buf = new uint8_t[accessor.count];
                        memcpy(buf, &((uint8_t*)buffer.data)[accessor.offset + bufferView.offset], accessor.count * sizeof(uint8_t));
                        for (size_t index = 0; index < accessor.count; index++) {
                            indexBuffer.push_back(buf[index] + vertexStart);
                        }
                        delete[] buf;
                        break;
                    }
                    default:
                        LOG_ERROR("Index component type {} not supported!", (int)accessor.component_type);
                        return;
                    }
                }

                // Skinning
                // Joints
                if (auto attr = find_attribute(primitive, cgltf_attribute_type_joints)) {
                    const auto& accessor = *attr->data;
                    bufferJoints = getBufferData<u16vec4>(attr->data);

                    if (mVertexAttribute == VertexAttribute::NONE || any(mVertexAttribute & VertexAttribute::BONE_INDICES)) {
                        auto vb = device().createBuffer(BufferUsage::VERTEX, MemoryUsage::GPU_ONLY, accessor.count * sizeof(u16vec4), sizeof(u16vec4), bufferJoints);
                        vb->extra = (uint16_t)VertexAttribute::BONE_INDICES;
                        defineList.add("HAS_JOINTS_0_VEC4", vbs.size());
                        vbs.push_back(vb);
                        formats.append({ Format::END, Format::R16G16B16A16_UINT });
                    }
                }

                if (auto attr = find_attribute(primitive, cgltf_attribute_type_weights)) {
                    const auto& accessor = *attr->data;
                    bufferWeights = getBufferData<float4>(attr->data);

                    if (mVertexAttribute == VertexAttribute::NONE || any(mVertexAttribute & VertexAttribute::BONE_WEIGHTS)) {
                        auto vb = device().createBuffer(BufferUsage::VERTEX, MemoryUsage::GPU_ONLY, accessor.count * sizeof(float4), sizeof(float4), bufferWeights);
                        vb->extra = (uint16_t)VertexAttribute::BONE_WEIGHTS;
                        defineList.add("HAS_WEIGHTS_0_VEC4", vbs.size());
                        vbs.push_back(vb);
                        formats.append({ Format::END, Format::R32G32B32A32_SFLOAT });
                    }
                }

                hasSkin = (bufferJoints && bufferWeights);

                size_t indexBufferSize = indexBuffer.size() * sizeof(uint32_t);

                auto vertexData = new VertexData();
                vertexData->vertexBuffers = vbs;
                vertexData->indexBuffer = device().createBuffer(BufferUsage::INDEX, MemoryUsage::GPU_ONLY, indexBufferSize, sizeof(uint32_t), indexBuffer.data());

                DrawPrimitiveCommand drawArgs;
                drawArgs.firstIndex = indexStart;
                drawArgs.indexCount = indexCount;
                auto& subMesh = newMesh->addSubMesh(vertexData, drawArgs);

                subMesh.material = primitive.material ? getMaterial(primitive.material - mGltfModel->materials, hasSkinOrMorphing, &defineList) : getDefaultMaterial(&defineList);
                subMesh.material->shader()->setVertexInput(formats);
                subMesh.boundingBox = Aabb{posMin, posMax};
                boundingBox.merge(subMesh.boundingBox);

                if (primitive.targets_count > 0) {
                    // auto targets = loadMorphTargets(primitive, vertexCount, (uint3*)indexBuffer.data(), (uint32_t)indexBuffer.size() / 3);
                    // subMesh.morphTargetBuffer = targets;
                }
            }

            newMesh->setBoundingBox(boundingBox);
            mBoundingBox.merge(boundingBox);
        }

        renderable->setMesh(newMesh);
    }
}

void ModelLoader::createCamera(Camera* camera, cgltf_camera* c)
{
    if (c->type == cgltf_camera_type_orthographic) {
        camera->setOrtho(true);
        camera->setOrthoSize({ c->data.orthographic.xmag, c->data.orthographic.ymag });
        camera->setNearPlane(c->data.orthographic.znear);
        camera->setFarPlane(c->data.orthographic.zfar);
    } else {
        camera->setOrtho(false);
        camera->setFov(c->data.perspective.yfov);

        if (c->data.perspective.has_aspect_ratio) {
            camera->setAspectRatio(c->data.perspective.aspect_ratio);
        }

        camera->setNearPlane(c->data.perspective.znear);

        if (c->data.perspective.has_zfar) {
            camera->setFarPlane(c->data.perspective.zfar);
        }
    }
}

void ModelLoader::createLight(Light* light, cgltf_light* l)
{
    light->setColor(make_vec3(l->color));
    light->setIntensity(l->intensity);
    light->setType((LightType)(l->type - 1));
    light->setRange(l->range);
    light->setSpotInnerConeAngle(l->spot_inner_cone_angle);
    light->setSpotOuterConeAngle(l->spot_outer_cone_angle);
}

// Parses a data URI and returns a blob that gets malloc'd in cgltf, which the caller must free.
// (implementation snarfed from meshoptimizer)
static uint8_t* parseDataUri(const char* uri, std::string* mimeType, size_t* psize)
{
    if (strncmp(uri, "data:", 5) != 0) {
        return nullptr;
    }
    const char* comma = strchr(uri, ',');
    if (comma && comma - uri >= 7 && strncmp(comma - 7, ";base64", 7) == 0) {
        const char* base64 = comma + 1;
        const size_t base64Size = strlen(base64);
        size_t size = base64Size - base64Size / 4;
        if (base64Size >= 2) {
            size -= base64[base64Size - 2] == '=';
            size -= base64[base64Size - 1] == '=';
        }
        void* data = 0;
        cgltf_options options = {};
        cgltf_result result = cgltf_load_buffer_base64(&options, size, base64, &data);
        if (result != cgltf_result_success) {
            return nullptr;
        }
        *mimeType = std::string(uri + 5, comma - 7);
        *psize = size;
        return (uint8_t*)data;
    }
    return nullptr;
}

void ModelLoader::loadImages()
{
    FileUtils::pushPath(mFilePath);

    for (int i = 0; i < mGltfModel->textures_count; i++) {
        auto& srcTexture = mGltfModel->textures[i];
        const cgltf_image* image = srcTexture.basisu_image ? srcTexture.basisu_image : srcTexture.image;
        const cgltf_buffer_view* bv = image->buffer_view;
        const char* uri = image->uri;
        std::string mime = image->mime_type ? image->mime_type : "";
        size_t dataUriSize;
        uint8_t* dataUriContent = uri ? parseDataUri(uri, &mime, &dataUriSize) : nullptr;
        bool srgb = mSrgb[i];

        if (mime.empty()) {
            assert(uri && "Non-URI images must supply a mime type.");
            const std::string extension = Path(uri).extension().string();
            mime = extension == "jpg" ? "image/jpeg" : "image/" + extension;
        }

        if (void** bufferViewData = bv ? &bv->buffer->data : nullptr; bufferViewData) {
            assert(!dataUriContent);
            const size_t offset = bv ? bv->offset : 0;
            uint8_t* sourceData = offset + (uint8_t*)*bufferViewData;
            const uint32_t totalSize = uint32_t(bv ? bv->size : 0);

            auto tex = Texture::createFromData(Span<uint8_t> { sourceData, (size_t)totalSize }, mime, {});
            if (tex == nullptr) {
                LOG_ERROR("Load texture failed : {}", totalSize);
            }

            mTextures.push_back(tex);

        } else if (dataUriContent) {

            auto tex = Texture::createFromData(Span<uint8_t> { dataUriContent, dataUriSize }, mime, {});
            if (tex == nullptr) {
                LOG_ERROR("Load texture failed : {}", dataUriSize);
            }

            mTextures.push_back(tex);
        } else {
            auto tex = Texture::createFromFile(uri, { .srgb = srgb });
            if (tex == nullptr) {
                LOG_ERROR("Load texture failed : {}", uri);
            }

            mTextures.push_back(tex);
        }
    }

    FileUtils::popPath();
}

void ModelLoader::loadMaterials()
{
    mMaterials.resize(mGltfModel->materials_count);
    mSrgb.resize(mGltfModel->textures_count);
    for (int i = 0; i < mGltfModel->materials_count; i++) {
        auto& mat = mGltfModel->materials[i];

        if (mat.emissive_texture.texture) {
            mSrgb[cgltf_texture_index(mGltfModel, mat.emissive_texture.texture)] = true;
        }

        if (mat.pbr_metallic_roughness.base_color_texture.texture) {
            mSrgb[cgltf_texture_index(mGltfModel, mat.pbr_metallic_roughness.base_color_texture.texture)] = true;
        }

        if (mat.pbr_metallic_roughness.metallic_roughness_texture.texture) {
            mSrgb[cgltf_texture_index(mGltfModel, mat.pbr_metallic_roughness.metallic_roughness_texture.texture)] = true;
        }

        if (mat.pbr_specular_glossiness.diffuse_texture.texture) {
            mSrgb[cgltf_texture_index(mGltfModel, mat.pbr_specular_glossiness.diffuse_texture.texture)] = true;
        }

        if (mat.pbr_specular_glossiness.specular_glossiness_texture.texture) {
            mSrgb[cgltf_texture_index(mGltfModel, mat.pbr_specular_glossiness.specular_glossiness_texture.texture)] = true;
        }
    }

    if (mMaterials.empty()) {
        // mMaterials.emplace_back(getDefaultMaterial(nullptr));
    }
}

void getDefines(cgltf_material& mat, DefineList& defineList)
{
    defineList.add("USE_IBL")
        .add("DEBUG_NONE", 0)
        .add("DEBUG", 0);

    if (mat.normal_texture.texture) {
        defineList.add("HAS_NORMAL_MAP");
    }

    if (mat.emissive_texture.texture) {
        defineList.add("HAS_EMISSIVE_MAP");
    }

    if (mat.occlusion_texture.texture) {
        defineList.add("HAS_OCCLUSION_MAP");
    }

    if (mat.has_pbr_metallic_roughness) {
        defineList.add("MATERIAL_METALLICROUGHNESS");

        if (mat.pbr_metallic_roughness.base_color_texture.texture) {
            defineList.add("HAS_BASE_COLOR_MAP");
        }

        // Metallic roughness workflow
        if (mat.pbr_metallic_roughness.metallic_roughness_texture.texture) {
            defineList.add("HAS_METALLIC_ROUGHNESS_MAP");
        }
    }

    if (mat.has_pbr_specular_glossiness) {
        defineList.add("MATERIAL_SPECULARGLOSSINESS");

        if (mat.pbr_specular_glossiness.diffuse_texture.texture) {
            defineList.add("HAS_DIFFUSE_MAP");
        }

        if (mat.pbr_specular_glossiness.specular_glossiness_texture.texture) {
            defineList.add("HAS_SPECULAR_GLOSSINESS_MAP");
        }
    }
}

Material* ModelLoader::getMaterial(size_t index, bool skined, const DefineList* marcos)
{
    if (mMaterials[index]) {
        return mMaterials[index];
    }

    DefineList defineList;
    if (marcos) {
        defineList += *marcos;
    }

    auto& mat = mGltfModel->materials[index];
    getDefines(mat, defineList);

    auto defaultShader = mShader ? mShader : Shader::fromFile("shaders/primitive.vert", "shaders/pbr.frag", &defineList);

    Material* material = new Material(defaultShader, "MaterialUniforms");

    if (mat.has_pbr_metallic_roughness) {
        auto& baseColorFactor = mat.pbr_metallic_roughness.base_color_factor;
        material->setShaderParameter("u_BaseColorFactor", vec4((float)baseColorFactor[0], (float)baseColorFactor[1], (float)baseColorFactor[2], (float)baseColorFactor[3]));
        material->setShaderParameter("u_RoughnessFactor", (float)mat.pbr_metallic_roughness.roughness_factor);
        material->setShaderParameter("u_MetallicFactor", (float)mat.pbr_metallic_roughness.metallic_factor);
    }

    if (mat.pbr_metallic_roughness.base_color_texture.texture) {
        auto baseColorTexture = getTexture(mat.pbr_metallic_roughness.base_color_texture.texture - mGltfModel->textures);
        material->setShaderParameter("u_BaseColorTexture", baseColorTexture);
    }

    // Metallic roughness workflow
    if (mat.pbr_metallic_roughness.metallic_roughness_texture.texture) {
        auto metallicRoughnessTexture = getTexture(mat.pbr_metallic_roughness.metallic_roughness_texture.texture - mGltfModel->textures);
        material->setShaderParameter("u_MetallicRoughnessTexture", metallicRoughnessTexture);
    }

    material->setShaderParameter("u_NormalScale", 1.0f);
    if (mat.normal_texture.texture) {
        auto normalTexture = getTexture(mat.normal_texture.texture - mGltfModel->textures);
        material->setShaderParameter("u_NormalTexture", normalTexture);
    }

    vec3 emissiveFactor = vec3((float)mat.emissive_factor[0], (float)mat.emissive_factor[1],
        (float)mat.emissive_factor[2]); // , mat.emissive_strength.emissive_strength);
    material->setShaderParameter("u_EmissiveFactor", emissiveFactor);

    if (mat.emissive_texture.texture) {
        auto emissiveTexture = getTexture(mat.emissive_texture.texture - mGltfModel->textures);
        material->setShaderParameter("u_EmissiveTexture", emissiveTexture);
    }

    material->setShaderParameter("u_OcclusionStrength", 1.0f);
    if (mat.occlusion_texture.texture) {
        auto occlusionTexture = getTexture(mat.occlusion_texture.texture - mGltfModel->textures);
        material->setShaderParameter("u_OcclusionTexture", occlusionTexture);
        material->setShaderParameter("u_OcclusionStrength", mat.occlusion_texture.scale);
    }

    if (mat.alpha_mode == cgltf_alpha_mode_blend) {
        material->setBlendMode(BlendMode::ALPHA);
    } else if (mat.alpha_mode == cgltf_alpha_mode_mask) {
        material->setBlendMode(BlendMode::MASK);
    } else {
        material->setBlendMode(BlendMode::NONE);
    }

    material->setShaderParameter("u_AlphaCutoff", mat.alpha_cutoff);
    material->setDoubleSide(mat.double_sided);
    mMaterials[index] = material;
    return material;
}

Texture* ModelLoader::getTexture(size_t index)
{
    if (index < mTextures.size()) {
        return mTextures[index];
    }
    return nullptr;
}

Ref<Material> ModelLoader::getDefaultMaterial(const DefineList* marcos)
{
    DefineList macros;
    macros.add("HAS_TEXCOORD_0_VEC2", 1)
        .add("HAS_NORMAL_VEC3", 2)
        .add("MATERIAL_METALLICROUGHNESS")
        .add("LINEAR_OUTPUT")
        .add("USE_IBL")
        .add("DEBUG_NONE", 0)
        .add("DEBUG", 0);

    auto defaultShader = Shader::fromFile("shaders/primitive.vert", "shaders/pbr.frag", &macros);
    defaultShader->setVertexInput({ Format::R32G32B32_SFLOAT, Format::END, Format::R32G32_SFLOAT, Format::END, Format::R32G32B32_SFLOAT });

    Ref<Material> material(new Material(defaultShader, "MaterialUniforms"));
    material->setShaderParameter("u_BaseColorFactor", vec4 { 1.0f, 1.0f, 1.0f, 1.0f });
    material->setShaderParameter("u_MetallicFactor", 0.0f);
    material->setShaderParameter("u_RoughnessFactor", 0.5f);
    return material;
}
}
