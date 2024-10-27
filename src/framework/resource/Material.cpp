#include "Material.h"
#include "GraphicsApi.h"
#include "Shader.h"
#include "Texture.h"

namespace mygfx {

static HashSet<Material*> sMaterials;
static HashSet<Material*> sAddMaterials;
static std::recursive_mutex sAddLock;
static HashSet<Material*> sRemoveMaterials;
static std::recursive_mutex sRemoveLock;

Material::Material()
{
    sAddLock.lock();
    sAddMaterials.insert(this);
    sAddLock.unlock();
}

Material::Material(Shader* shader, const String& materialUniformName)
{
    sAddLock.lock();
    sAddMaterials.insert(this);
    sAddLock.unlock();

    setShader(shader, materialUniformName);
}

Material::~Material()
{
    sRemoveLock.lock();
    sRemoveMaterials.insert(this);
    sRemoveLock.unlock();
}

void Material::setShader(Shader* shader, const String& materialUniformName)
{
    if (mShader == shader || shader == nullptr) {
        return;
    }

    mShader = shader;
    mMaterialUniformName = materialUniformName;
    mPipelineState = mShader->pipelineState;
    mShaderResourceInfo = shader->getProgram()->getShaderResource(materialUniformName);

    if (mShaderResourceInfo) {
        mMaterialData.resize(mShaderResourceInfo->getMemberSize());
        std::ranges::fill(mMaterialData, 0);
    }
}

void Material::setShaderParameter(const String& name, int v)
{
    if (mShaderResourceInfo) {
        auto member = mShaderResourceInfo->getMember(name);
        if (member) {
            assert(member->size == sizeof(int));
            std::memcpy(&mMaterialData[member->offset], &v, sizeof(int));
        }
        mShaderParameters[name] = v;
    }
}

void Material::setShaderParameter(const String& name, float v)
{
    if (mShaderResourceInfo) {
        auto member = mShaderResourceInfo->getMember(name);
        if (member) {
            assert(member->size == sizeof(float));
            std::memcpy(&mMaterialData[member->offset], &v, sizeof(float));
        }
        mShaderParameters[name] = v;
    }
}

void Material::setShaderParameter(const String& name, const vec3& v)
{
    if (mShaderResourceInfo) {
        auto member = mShaderResourceInfo->getMember(name);
        if (member) {
            assert(member->size == sizeof(vec3));
            std::memcpy(&mMaterialData[member->offset], &v, sizeof(vec3));
        }
        mShaderParameters[name] = v;
    }
}

void Material::setShaderParameter(const String& name, const vec4& v)
{
    if (mShaderResourceInfo) {
        auto member = mShaderResourceInfo->getMember(name);
        if (member) {
            assert(member->size == sizeof(vec4));
            std::memcpy(&mMaterialData[member->offset], &v, sizeof(vec4));
        }
        mShaderParameters[name] = v;
    }
}

void Material::setShaderParameter(const String& name, Texture* tex)
{
    if (mShaderResourceInfo) {
        auto member = mShaderResourceInfo->getMember(name);
        if (member) {
            int32_t texIndex = tex->getSRV()->index();
            assert(member->size == sizeof(int32_t));
            std::memcpy(&mMaterialData[member->offset], &texIndex, sizeof(int32_t));
        }
        mShaderParameters[name] = Ref<Texture>(tex);
    }
}

void Material::setDoubleSide(bool v)
{
    mPipelineState.rasterState.cullMode = v ? CullMode::NONE : CullMode::BACK;
}

void Material::setWireframe(bool v)
{
    mPipelineState.rasterState.polygonMode = v ? PolygonMode::LINE : PolygonMode::FILL;
}

void Material::setBlendMode(BlendMode blendMode)
{
    mPipelineState.colorBlendState = ColorBlendState::get(blendMode);
}

void Material::update()
{
    if (mMaterialData.empty()) {
        return;
    }

    void* pData;
    BufferInfo bufferInfo;
    if (!device().allocConstantBuffer((uint32_t)mMaterialData.size(), &pData, &bufferInfo)) {
        return;
    }

    std::memcpy(pData, mMaterialData.data(), mMaterialData.size());
    mMaterialUniforms = bufferInfo.offset;
}

void Material::updateAll()
{
    sAddLock.lock();
    for (auto m : sAddMaterials) {
        sMaterials.insert(m);
    }
    sAddLock.unlock();

    sRemoveLock.lock();
    for (auto m : sRemoveMaterials) {
        sMaterials.erase(m);
    }
    sRemoveLock.unlock();

    for (auto material : sMaterials) {
        material->update();
    }
}
}