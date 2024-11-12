#include "Material.h"
#include "GraphicsApi.h"
#include "Shader.h"
#include "Texture.h"

namespace mygfx {

static HashSet<Material*> sMaterials;
static std::recursive_mutex sMaterialLock;

Material::Material()
{
    sMaterialLock.lock();
    sMaterials.insert(this);
    sMaterialLock.unlock();
}

Material::Material(ShaderEffect* shader)
{
    sMaterialLock.lock();
    sMaterials.insert(this);
    sMaterialLock.unlock();

    setShader(shader);
}

Material::~Material()
{
    sMaterialLock.lock();
    sMaterials.erase(this);
    sMaterialLock.unlock();
}

void Material::setShader(ShaderEffect* shader)
{
    if (mShaderFX == shader || shader == nullptr) {
        return;
    }

    mShaderFX = shader;
    auto& materialUniformName = shader->getMaterialUniformName();
    mPipelineState = mShaderFX->getMainPass()->pipelineState;
    mShaderResourceInfo = mShaderFX->getMainPass()->getShaderResource(materialUniformName);

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
    sMaterialLock.lock();
    for (auto material : sMaterials) {
        material->update();
    }
    sMaterialLock.unlock();
}
}