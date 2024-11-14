#pragma once

#include "../GraphicsHandles.h"
#include "../ShaderResourceInfo.h"
#include "DescriptorSetLayout.h"
#include "VulkanDefs.h"
#include <fstream>
#include <stdlib.h>
#include <string>
#include <vector>

namespace mygfx {

class VulkanShaderModule : public HwShaderModule {
public:
    VulkanShaderModule(ShaderStage stage, const std::vector<uint8_t>& shaderCode, ShaderCodeType shaderCodeType = ShaderCodeType::SPIRV, const char* pShaderEntryPoint = nullptr);
    ~VulkanShaderModule();

    void collectShaderResource();

    Vector<PushConstant> pushConstants;
    Vector<SpecializationConst> specializationConsts;
    Vector<uint8_t> specializationData;
    ShaderStage shaderStage;
    Vector<uint8_t> shaderCode;
    String entryPoint = "main";
    VkShaderStageFlagBits vkShaderStage;
#if HAS_SHADER_OBJECT_EXT
    VkShaderStageFlags nextStage;
#else
    VkShaderModule shaderModule = VK_NULL_HANDLE;
#endif
};

class DescriptorSet;

class VulkanProgram : public HwProgram {
public:
    VulkanProgram();
    VulkanProgram(Ref<HwShaderModule>* shaderModules, uint32_t count);
    ~VulkanProgram();

    ShaderResourceInfo* getShaderResource(const String& name);
    DescriptorSet* getDescriptorSet(uint32_t index);
    HwDescriptorSet* createDescriptorSet(uint32_t index);
    bool createShaders();
    VkPipelineBindPoint getBindPoint() const { return (VkPipelineBindPoint)programType; }
    static constexpr int MAX_SHADER_STAGE = 8;
    VkPipelineLayout pipelineLayout = 0;
    uint32_t stageCount = 0;
#if HAS_SHADER_OBJECT_EXT
    VkShaderEXT shaders[MAX_SHADER_STAGE] {};
#else
    VkPipeline getGraphicsPipeline(const AttachmentFormats& attachmentFormats, const struct PipelineState* pipelineState);
    VkPipeline getComputePipeline();
    VkPipeline pipeline = VK_NULL_HANDLE;
    size_t attachmentFormatsHash = 0;
#endif
    VkShaderStageFlagBits stages[MAX_SHADER_STAGE] {};
    Vector<VkDescriptorSet> desciptorSets;
    Vector<PushConstant> pushConstants;

    std::map<uint32_t, std::vector<Ref<ShaderResourceInfo>>> combinedBindingMap;

private:
    Vector<Ref<VulkanShaderModule>> mShaderModules;
    Vector<Ref<DescriptorSetLayout>> mDescriptorSetLayouts;
    Vector<VkDescriptorSetLayout> descriptorSetLayouts;
    ProgramType programType;
    VkShaderCodeTypeEXT shaderCodeType;
    Vector<Ref<DescriptorSet>> mDesciptorSets;
};

VkShaderStageFlagBits ToVkShaderStage(ShaderStage stage);

} // namespace mygfx
