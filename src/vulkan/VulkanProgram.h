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
#include "VulkanShader.h"

namespace mygfx {

class DescriptorSet;

#if !HAS_SHADER_OBJECT_EXT

class VulkanProgram;

struct PipelineInfo {
    VkPipeline pipeline = VK_NULL_HANDLE;
    size_t hash = 0;
    TimePoint lastTime {};

    void destroy();
};

class PipelineCache : public std::unordered_map<size_t, PipelineInfo> {
public:
    PipelineCache();
    ~PipelineCache();

    static void gc();
};

#endif

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
    PipelineInfo pipelineInfo;
#if !HAS_DYNAMIC_STATE3
    PipelineCache pipelineCache;
#endif

#endif
    VkShaderStageFlagBits stages[MAX_SHADER_STAGE] {};
    Vector<VkDescriptorSet> desciptorSets;
    Vector<PushConstant> pushConstants;

    std::map<uint32_t, std::vector<Ref<ShaderResourceInfo>>> combinedBindingMap;

private:
    Vector<Ref<VulkanShaderModule>> mShaderModules;
    Vector<Ref<DescriptorSetLayout>> mDescriptorSetLayouts;
    Vector<VkDescriptorSetLayout> mVkDescriptorSetLayouts;
    ProgramType programType;
    VkShaderCodeTypeEXT shaderCodeType;
    Vector<Ref<DescriptorSet>> mDesciptorSets;
};


} // namespace mygfx
