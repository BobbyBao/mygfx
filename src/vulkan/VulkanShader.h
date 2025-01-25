#pragma once

#include "../GraphicsHandles.h"
#include "../ShaderResourceInfo.h"
#include "DescriptorSetLayout.h"
#include "VulkanDefs.h"
#include <fstream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <unordered_map>

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

VkShaderStageFlagBits ToVkShaderStage(ShaderStage stage);

} // namespace mygfx
