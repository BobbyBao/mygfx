#pragma once

#include <fstream>
#include <stdlib.h>
#include <string>
#include <vector>
#include "VulkanDefs.h"
#include "../GraphicsHandles.h"
#include "../ShaderResourceInfo.h"
#include "DescriptorSetLayout.h"

namespace mygfx
{
	class VulkanShaderModule : public HwShaderModule {
	public:
		VulkanShaderModule(ShaderStage stage, const std::vector<uint8_t>& shaderCode, ShaderCodeType shaderCodeType = ShaderCodeType::Spirv);

		void collectShaderResource();

		PushConstant pushConstants;
		std::vector<SpecializationConst> specializationConsts;
		ShaderStage shaderStage;
		VkShaderStageFlagBits vkShaderStage;
		VkShaderStageFlags  nextStage;
		std::vector<uint8_t> shaderCode;
	};

	class DescriptorSet;

	class VulkanProgram : public HwProgram {
	public:
		VulkanProgram();
		VulkanProgram(Ref<HwShaderModule>* shaderModules, uint32_t count);
		~VulkanProgram();

		ShaderResourceInfo* getShaderResource(const String& name);
		DescriptorSet* getDescriptorSet(uint32_t index);
		bool createShaders();
		VkPipelineBindPoint getBindPoint() const { return (VkPipelineBindPoint)programType; }
		static constexpr int MAX_SHADER_STAGE = 8;
		VkPipelineLayout pipelineLayout = 0;
		uint32_t stageCount = 0;
		VkShaderEXT shaders[MAX_SHADER_STAGE]{};
		VkShaderStageFlagBits stages[MAX_SHADER_STAGE]{};
		std::vector<VkDescriptorSet> desciptorSets;
	private:
		std::vector<Ref<VulkanShaderModule>> mShaderModules;
		std::vector<Ref<DescriptorSetLayout>> mDescriptorSetLayouts;
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		ProgramType programType;
		VkShaderCodeTypeEXT shaderCodeType;		
		std::vector<Ref<DescriptorSet>> mDesciptorSets;
	};

	VkShaderStageFlagBits ToVkShaderStage(ShaderStage stage);

}        // namespace mygfx
