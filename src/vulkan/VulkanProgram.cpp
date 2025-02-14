#include "VulkanProgram.h"
#include "VulkanDevice.h"
#include "VulkanInitializers.hpp"
#include "utils/Log.h"
#include "utils/ThreadUtils.h"

namespace mygfx {

#if !HAS_SHADER_OBJECT_EXT

inline static std::unordered_set<PipelineCache*> sPipelineCaches;
inline static std::mutex sLock;
thread_local PipelineCache sPipelineCache;

void PipelineInfo::destroy()
{
    if (pipeline) {
        vkDestroyPipeline(gfx().device, pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
    }
}

PipelineCache::PipelineCache()
{
    std::unique_lock locker(sLock);
    sPipelineCaches.insert(this);
}

PipelineCache::~PipelineCache()
{
    std::unique_lock locker(sLock);
    sPipelineCaches.erase(this);
}

void PipelineCache::gc()
{
    using namespace std::literals::chrono_literals;
    static Vector<size_t> toRemove;
    auto now = Clock::now();
    std::unique_lock locker(sLock);
    for (auto& pipelineCache : sPipelineCaches) {
        
        for (auto& info : *pipelineCache) {
            if (now - info.second.lastTime < 100s) {
                // todo:
                info.destroy();
                toRemove.push_back(info.first);
            }
        }

        if (toRemove.size() > 0) {
            for (auto hash : toRemove) {
                pipelineCache->erase(hash);
            }
            toRemove.clear();
        }
    }
}

#endif

ShaderResourceInfo* findShaderResource(std::vector<Ref<ShaderResourceInfo>> shaderResources, uint32_t set, uint32_t binding)
{
    for (auto& res : shaderResources) {
        if (set == res->set && res->binding() == binding) {
            return res;
        }
    }

    return nullptr;
}

ShaderResourceInfo* HwProgram::getShaderResource(const String& name)
{
    return static_cast<VulkanProgram*>(this)->getShaderResource(name);
}

HwDescriptorSet* HwProgram::getDescriptorSet(uint32_t index)
{
    return static_cast<VulkanProgram*>(this)->getDescriptorSet(index);
}

HwDescriptorSet* HwProgram::createDescriptorSet(uint32_t index)
{
    return static_cast<VulkanProgram*>(this)->createDescriptorSet(index);
}

VulkanProgram::VulkanProgram()
{
}

VulkanProgram::VulkanProgram(Ref<HwShaderModule>* shaderModules, uint32_t count)
{
    assert(count <= MAX_SHADER_STAGE);
    shaderCodeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
    stageCount = count;

    VkShaderStageFlags fullShaderStageFlags = 0;

    for (uint32_t i = 0; i < stageCount; i++) {
        VulkanShaderModule* sm = static_cast<VulkanShaderModule*>(shaderModules[i].get());
        mShaderModules.emplace_back(sm);

        stages[i] = sm->vkShaderStage;
        fullShaderStageFlags |= sm->vkShaderStage;

        for (auto& pc : sm->pushConstants) {
            bool found = false;
            for (auto& pushConst : pushConstants) {
                if (pushConst.name == pc.name) {
                    assert(pushConst.offset == pc.offset && pushConst.size == pc.size);
                    pushConst.stageFlags |= sm->shaderStage;
                    found = true;
                    break;
                }
            }

            if (!found) {
                pushConstants.push_back(pc);
            }
        }

        for (auto& res : sm->shaderResourceInfo) {
            auto combineSet = combinedBindingMap.find(res->set);
            if (combineSet == combinedBindingMap.end()) {
                combinedBindingMap[res->set] = std::vector<Ref<ShaderResourceInfo>>();
                combinedBindingMap[res->set].push_back(res);
            } else {
                auto unionBinding = findShaderResource(combineSet->second, res->set, res->binding());
                if (unionBinding) {
                    unionBinding->dsLayoutBinding.stageFlags |= ShaderStage(sm->vkShaderStage);

                    if (unionBinding->name != res->name && unionBinding->dsLayoutBinding.descriptorType != res->dsLayoutBinding.descriptorType) {
                        LOG_ERROR("Error binding, res name: {}, {}", unionBinding->name.c_str(), res->name.c_str());
                    }
                } else {
                    combineSet->second.push_back(res);
                }
            }
        }
    }

    if (fullShaderStageFlags == (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)) {
        programType = ProgramType::GRAPHICS;
    } else if (fullShaderStageFlags == VK_SHADER_STAGE_COMPUTE_BIT) {
        programType = ProgramType::COMPUTE;
    } else {
        assert(false);
    }

    enum ResourceSetType : int {
        Default = -1,
        CombinedImageSampler,
        Sampler,
        SampledImage,
        StorageImage,
        StorageBuffer,
    };

    VkDescriptorSetLayout descriptorSetLayout { VK_NULL_HANDLE };
    for (auto& it : combinedBindingMap) {
        std::ranges::sort(it.second, [](auto& v1, auto& v2) { return v1->dsLayoutBinding.binding < v2->dsLayoutBinding.binding; });

        ResourceSetType resourceSetType = ResourceSetType::Default;
        for (auto& res : it.second) {
            if (res->bindless) {

                assert(it.second.size() == 1);
                if (res->dsLayoutBinding.descriptorType == DescriptorType::COMBINED_IMAGE_SAMPLER) {
                    resourceSetType = ResourceSetType::CombinedImageSampler;
                } else if (res->dsLayoutBinding.descriptorType == DescriptorType::STORAGE_IMAGE) {
                    resourceSetType = ResourceSetType::StorageImage;
                } else if (res->dsLayoutBinding.descriptorType == DescriptorType::SAMPLED_IMAGE) {
                    resourceSetType = ResourceSetType::SampledImage;
                } else if (res->dsLayoutBinding.descriptorType == DescriptorType::SAMPLER) {
                    resourceSetType = ResourceSetType::Sampler;
                }
            }
        }

        auto dsLayout = new DescriptorSetLayout(it.second);
        mDescriptorSetLayouts.emplace_back(dsLayout);

        switch (resourceSetType) {
        case ResourceSetType::CombinedImageSampler: {
            auto ds = gfx().getTextureSet()->getDescriptorSet(dsLayout);
            desciptorSets.push_back(*ds);
            mDesciptorSets.emplace_back(ds);
        } break;
        case ResourceSetType::SampledImage: {
            auto ds = gfx().getImageSet()->getDescriptorSet(dsLayout);
            desciptorSets.push_back(*ds);
            mDesciptorSets.emplace_back(ds);
        } break;
        case ResourceSetType::StorageImage: {
            auto ds = gfx().getStorageImageSet()->getDescriptorSet(dsLayout);
            desciptorSets.push_back(*ds);
            mDesciptorSets.emplace_back(ds);
        } break;
        case ResourceSetType::Sampler: {
            auto ds = gfx().getSamplerSet()->getDescriptorSet(dsLayout);
            desciptorSets.push_back(*ds);
            mDesciptorSets.emplace_back(ds);
        } break;
        default: {
            auto ds = new DescriptorSet(dsLayout);
            mDesciptorSets.emplace_back(ds);
            desciptorSets.push_back(*ds);

            for (uint32_t j = 0; j < dsLayout->numBindings(); j++) {
                if (it.second[j]->dsLayoutBinding.descriptorType == DescriptorType::UNIFORM_BUFFER_DYNAMIC) {
                    auto sz = it.second[j]->size;
                    if (ds->dynamicBufferSize[j] < sz) {
                        ds->dynamicBufferSize[j] = sz;
                        gfx().updateDynamicDescriptorSet(it.second[j]->dsLayoutBinding.binding, sz, *ds);
                    }
                }
            }

        } break;
        }

        mVkDescriptorSetLayouts.push_back(dsLayout->handle());
    }

    Vector<VkPushConstantRange> pushConstRanges;
    for (auto& pc : pushConstants) {
        pushConstRanges.push_back(VkPushConstantRange { (VkShaderStageFlags)pc.stageFlags, pc.offset, pc.size });
    }

    VkPipelineLayoutCreateInfo pipelineLayoutCI = initializers::pipelineLayoutCreateInfo(mVkDescriptorSetLayouts.data(), (uint32_t)mVkDescriptorSetLayouts.size());
    pipelineLayoutCI.pushConstantRangeCount = (uint32_t)pushConstRanges.size();
    pipelineLayoutCI.pPushConstantRanges = pushConstRanges.data();
    VK_CHECK_RESULT(vkCreatePipelineLayout(gfx().device, &pipelineLayoutCI, nullptr, &pipelineLayout));

    createShaders();
}

VulkanProgram::~VulkanProgram()
{
#if HAS_SHADER_OBJECT_EXT
    for (uint32_t i = 0; i < stageCount; i++) {
        vkDestroyShaderEXT(gfx().device, shaders[i], nullptr);
    }
#else

    pipelineInfo.destroy();

#if !HAS_DYNAMIC_STATE3
    for (auto& info : pipelineCache) {
        info.second.destroy();
    }

    pipelineCache.clear();

#endif

#endif
    vkDestroyPipelineLayout(gfx().device, pipelineLayout, nullptr);
}

ShaderResourceInfo* VulkanProgram::getShaderResource(const String& name)
{
    for (auto& sm : mShaderModules) {
        for (auto& res : sm->shaderResourceInfo) {
            if (name == res->name) {
                return res;
            }
        }
    }
    return nullptr;
}

DescriptorSet* VulkanProgram::getDescriptorSet(uint32_t index)
{
    if (index < mDesciptorSets.size()) {
        return mDesciptorSets[index];
    }
    return nullptr;
}

HwDescriptorSet* VulkanProgram::createDescriptorSet(uint32_t index)
{
    if (index < mDesciptorSets.size()) {
        auto& dsLayout = mDescriptorSetLayouts[index];
        if (dsLayout->isBindless) {
            return mDesciptorSets[index];
        } else {
            auto ds = new DescriptorSet(dsLayout);
            auto& res = combinedBindingMap[index];
            for (uint32_t j = 0; j < dsLayout->numBindings(); j++) {
                auto& layoutBinding = *dsLayout->getBinding(j);
                if (layoutBinding.descriptorType == DescriptorType::UNIFORM_BUFFER_DYNAMIC) {
                    auto sz = res[j]->size;
                    if (ds->dynamicBufferSize[j] < sz) {
                        ds->dynamicBufferSize[j] = sz;
                        gfx().updateDynamicDescriptorSet(layoutBinding.binding, sz, *ds);
                    }
                }
            }
            return ds;
        }
    }
    return nullptr;
}

bool VulkanProgram::createShaders()
{
#if HAS_SHADER_OBJECT_EXT
    VkShaderCreateInfoEXT shaderCreateInfos[MAX_SHADER_STAGE] {};
    Vector<VkPushConstantRange> pushConstRanges;
    Vector<VkSpecializationMapEntry> specializationMapEntries;
    VkSpecializationInfo specializationInfo;
    for (uint32_t i = 0; i < stageCount; i++) {
        VulkanShaderModule* sm = mShaderModules[i].get();
        pushConstRanges.clear();
        for (auto& pc : pushConstants) {
            if (any(pc.stageFlags & sm->shaderStage)) {
                pushConstRanges.push_back(VkPushConstantRange { (VkShaderStageFlags)pc.stageFlags, pc.offset, pc.size });
            }
        }

        /*
        specializationMapEntries.clear();
        for (auto& sc : sm->specializationConsts) {
                specializationMapEntries.push_back(VkSpecializationMapEntry{sc.id, 0, sc.size});
        }

        specializationInfo.mapEntryCount = specializationMapEntries.size();
        specializationInfo.pMapEntries = specializationMapEntries.data();*/

        shaderCreateInfos[i].sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT;
        shaderCreateInfos[i].flags = stageCount > 1 ? VK_SHADER_CREATE_LINK_STAGE_BIT_EXT : 0;
        shaderCreateInfos[i].stage = sm->vkShaderStage;
        shaderCreateInfos[i].nextStage = sm->nextStage;
        shaderCreateInfos[i].codeType = shaderCodeType;
        shaderCreateInfos[i].pCode = sm->shaderCode.data();
        shaderCreateInfos[i].codeSize = sm->shaderCode.size();
        shaderCreateInfos[i].pName = sm->entryPoint.data();
        shaderCreateInfos[i].setLayoutCount = (uint32_t)mVkDescriptorSetLayouts.size();
        shaderCreateInfos[i].pSetLayouts = mVkDescriptorSetLayouts.data();
        shaderCreateInfos[i].pPushConstantRanges = pushConstRanges.data();
        shaderCreateInfos[i].pushConstantRangeCount = (uint32_t)pushConstRanges.size();

        if (specializationMapEntries.size() > 0) {
            shaderCreateInfos[i].pSpecializationInfo = &specializationInfo;
        } else {
            shaderCreateInfos[i].pSpecializationInfo = nullptr;
        }
    }

    VkResult result = vkCreateShadersEXT(gfx().device, stageCount, shaderCreateInfos, nullptr, shaders);
    // If the function returns e.g. VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT, the binary file is no longer (or not at all) compatible with the current implementation
    if (result == VK_SUCCESS) {

        /*
        if (shaderCodeType == ShaderCodeType::Spirv) {
                size_t dataSize{ 0 };
                char* data{ nullptr };
                std::fstream is;

                g_vkGetShaderBinaryDataEXT(gfx().device, shader, &dataSize, nullptr);
                data = new char[dataSize];
                g_vkGetShaderBinaryDataEXT(gfx().device, shader, &dataSize, data);
                is.open(vsBin, std::ios::binary | std::ios::out);
                is.write(data, dataSize);
                is.close();
                delete[] data;
        }*/

        return true;
    } else {
        std::cout << "Could not load binary shader files (" << tools::errorString(result) << ", loading SPIR - V instead\n";
    }
#else

#endif
    return false;
}

#if !HAS_SHADER_OBJECT_EXT

VkPipeline VulkanProgram::getGraphicsPipeline(const AttachmentFormats& attachmentFormats, const PipelineState* pipelineState)
{
    // assert(ThreadUtils::isThisThread(gfx().renderThreadID));

    size_t pipelineHash = attachmentFormats.getHash();

#if !HAS_DYNAMIC_STATE3
    pipelineHash = fnv1a(pipelineHash, (const unsigned char*)pipelineState, sizeof(PipelineState));
    auto it = pipelineCache.find(pipelineHash);
    if (it != pipelineCache.end()) {
        it->second.lastTime = Clock::now();
        return it->second.pipeline;
    }
#else

    if (pipelineHash == pipelineInfo.hash) {
        pipelineInfo.lastTime = Clock::now();
        return pipelineInfo.pipeline;
    } else {
        pipelineInfo.destroy();
    }
#endif

    VkPipelineShaderStageCreateInfo stages[16];
    for (int i = 0; i < mShaderModules.size(); i++) {
        stages[i] = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .flags = 0,
            .stage = mShaderModules[i]->vkShaderStage,
            .module = mShaderModules[i]->shaderModule,
            .pName = mShaderModules[i]->entryPoint.c_str(),
        };
    }

    VkPipelineVertexInputStateCreateInfo vertexInputState = initializers::pipelineVertexInputStateCreateInfo();
#if !HAS_DYNAMIC_STATE3
    VkVertexInputAttributeDescription vertexInputAttributeDescription[16];
    VkVertexInputBindingDescription vertexInputBindingDescription[16];
    VulkanVertexInput* vertexInput = (VulkanVertexInput*)pipelineState->program->vertexInput.get();
    if (vertexInput) {
        for (auto i = 0; i < vertexInput->attributeDescriptions.size(); i++) {
            vertexInputAttributeDescription[i] = {
                .location = vertexInput->attributeDescriptions[i].location,
                .binding = vertexInput->attributeDescriptions[i].binding,
                .format = vertexInput->attributeDescriptions[i].format,
                .offset = vertexInput->attributeDescriptions[i].offset
            };
        }
        for (auto i = 0; i < vertexInput->bindingDescriptions.size(); i++) {
            vertexInputBindingDescription[i] = {
                .binding = vertexInput->bindingDescriptions[i].binding,
                .stride = vertexInput->bindingDescriptions[i].stride,
                .inputRate = vertexInput->bindingDescriptions[i].inputRate
            };
        }
        vertexInputState.pVertexAttributeDescriptions = vertexInputAttributeDescription;
        vertexInputState.pVertexBindingDescriptions = vertexInputBindingDescription;
        vertexInputState.vertexAttributeDescriptionCount = (uint32_t)vertexInput->attributeDescriptions.size();
        vertexInputState.vertexBindingDescriptionCount = (uint32_t)vertexInput->bindingDescriptions.size();
    }

#endif

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = initializers::pipelineInputAssemblyStateCreateInfo(
            (VkPrimitiveTopology)pipelineState->primitiveState.primitiveTopology, 0, false);
    VkPipelineTessellationStateCreateInfo tessellationState = initializers::pipelineTessellationStateCreateInfo(3);
    VkPipelineViewportStateCreateInfo viewportState = initializers::pipelineViewportStateCreateInfo(0, 0);
    VkPipelineRasterizationStateCreateInfo rasterizationState = initializers::pipelineRasterizationStateCreateInfo(VkPolygonMode::VK_POLYGON_MODE_FILL, VkCullModeFlagBits::VK_CULL_MODE_NONE, VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
    VkPipelineMultisampleStateCreateInfo multisampleState = initializers::pipelineMultisampleStateCreateInfo(VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT);
    VkPipelineDepthStencilStateCreateInfo depthStencilState = initializers::pipelineDepthStencilStateCreateInfo(true, true, INVERTED_DEPTH ? VK_COMPARE_OP_GREATER_OR_EQUAL : VK_COMPARE_OP_LESS_OR_EQUAL);
    auto colorBlendAttachmentState = initializers::pipelineColorBlendAttachmentState(VkColorComponentFlagBits::VK_COLOR_COMPONENT_R_BIT | VkColorComponentFlagBits::VK_COLOR_COMPONENT_G_BIT | VkColorComponentFlagBits::VK_COLOR_COMPONENT_B_BIT | VkColorComponentFlagBits::VK_COLOR_COMPONENT_A_BIT, false);

#if !HAS_DYNAMIC_STATE3

    rasterizationState.polygonMode = (VkPolygonMode)pipelineState->rasterState.polygonMode;
    rasterizationState.rasterizerDiscardEnable = pipelineState->rasterState.rasterizerDiscardEnable;

    multisampleState.alphaToCoverageEnable = pipelineState->rasterState.alphaToCoverageEnable;
    multisampleState.alphaToOneEnable = pipelineState->rasterState.alphaToOneEnable;
    multisampleState.rasterizationSamples = (VkSampleCountFlagBits)pipelineState->rasterState.rasterizationSamples;

    colorBlendAttachmentState.blendEnable = pipelineState->colorBlendState.colorBlendEnable;
    colorBlendAttachmentState.colorBlendOp = (VkBlendOp)pipelineState->colorBlendState.colorBlendOp;
    colorBlendAttachmentState.alphaBlendOp = (VkBlendOp)pipelineState->colorBlendState.alphaBlendOp;
    colorBlendAttachmentState.srcColorBlendFactor = (VkBlendFactor)pipelineState->colorBlendState.srcColorBlendFactor;
    colorBlendAttachmentState.dstColorBlendFactor = (VkBlendFactor)pipelineState->colorBlendState.dstColorBlendFactor;
    colorBlendAttachmentState.srcAlphaBlendFactor = (VkBlendFactor)pipelineState->colorBlendState.srcAlphaBlendFactor;
    colorBlendAttachmentState.dstAlphaBlendFactor = (VkBlendFactor)pipelineState->colorBlendState.dstAlphaBlendFactor;
    colorBlendAttachmentState.colorWriteMask = (VkColorComponentFlags)pipelineState->colorBlendState.colorWrite;
#endif

    VkPipelineColorBlendStateCreateInfo colorBlendState = initializers::pipelineColorBlendStateCreateInfo(1, &colorBlendAttachmentState);

    std::vector<VkDynamicState> dynamic_state_enables = {
        // VK_DYNAMIC_STATE_VIEWPORT,
        // VK_DYNAMIC_STATE_SCISSOR,

        VK_DYNAMIC_STATE_CULL_MODE,
        VK_DYNAMIC_STATE_FRONT_FACE,
        VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY,

        VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT,
        VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT,

        VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
        VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
        VK_DYNAMIC_STATE_DEPTH_COMPARE_OP,

        VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE_EXT,

        //============================

        VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE,

        VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE,
        VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE_EXT,

    //============================
#if HAS_DYNAMIC_STATE3

        VK_DYNAMIC_STATE_POLYGON_MODE_EXT,
        VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT,
        VK_DYNAMIC_STATE_SAMPLE_MASK_EXT,

        VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT,

        VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT,
        VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT,
        VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT,

        VK_DYNAMIC_STATE_VERTEX_INPUT_EXT
#endif
    };

    VkPipelineDynamicStateCreateInfo dynamicState = initializers::pipelineDynamicStateCreateInfo(dynamic_state_enables);

    VkPipelineRenderingCreateInfoKHR pipeline_create { VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR };
    pipeline_create.pNext = VK_NULL_HANDLE;
    pipeline_create.colorAttachmentCount = attachmentFormats.colorAttachmentCount;
    pipeline_create.pColorAttachmentFormats = attachmentFormats.attachmentFormats;
    pipeline_create.depthAttachmentFormat = attachmentFormats.depthAttachmentFormat();
    pipeline_create.stencilAttachmentFormat = attachmentFormats.stencilAttachmentFormat();

    VkGraphicsPipelineCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &pipeline_create,
        .stageCount = (uint32_t)mShaderModules.size(),
        .pStages = stages,
        .pVertexInputState = &vertexInputState,
        .pInputAssemblyState = &inputAssemblyState,
        .pTessellationState = &tessellationState,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizationState,
        .pMultisampleState = &multisampleState,
        .pDepthStencilState = &depthStencilState,
        .pColorBlendState = &colorBlendState,
        .pDynamicState = &dynamicState,

        .layout = pipelineLayout,
        .renderPass = VK_NULL_HANDLE,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1,
    };

    VkPipeline pipe = VK_NULL_HANDLE;
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(gfx().device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipe));

#if !HAS_DYNAMIC_STATE3
    pipelineCache.emplace(pipelineHash, PipelineInfo { .pipeline = pipe, .hash = pipelineHash, .lastTime = Clock::now() });
#else
    pipelineInfo.hash = pipelineHash;
    pipelineInfo.pipeline = pipe;
#endif
    return pipe;
}

VkPipeline VulkanProgram::getComputePipeline()
{
    if (pipelineInfo.pipeline) {
        return pipelineInfo.pipeline;
    }

    if (mShaderModules.size() == 0) {
        return VK_NULL_HANDLE;
    }

    VkComputePipelineCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .stage = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .flags = 0,
            .stage = mShaderModules[0]->vkShaderStage,
            .module = mShaderModules[0]->shaderModule,
            .pName = mShaderModules[0]->entryPoint.c_str(),
        },
        .layout = pipelineLayout,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1,
    };

    VK_CHECK_RESULT(vkCreateComputePipelines(gfx().device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipelineInfo.pipeline));
    return pipelineInfo.pipeline;
}
#endif

} // namespace mygfx
