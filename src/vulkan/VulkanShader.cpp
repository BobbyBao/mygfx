#include "VulkanShader.h"
#include "VulkanDevice.h"
#include <spirv_cross/spirv_glsl.hpp>
#include "utils/Log.h"

using namespace spirv_cross;

namespace mygfx
{
	ShaderResourceInfo* findShaderResource(std::vector<Ref<ShaderResourceInfo>> shaderResources, uint32_t set, uint32_t binding) {
		for (auto& res : shaderResources) {
			if (set == res->set && res->binding() == binding) {
				return res;
			}
		}

		return nullptr;
	}

	ShaderResourceInfo* HwProgram::getShaderResource(const String& name) {
		return static_cast<VulkanProgram*>(this)->getShaderResource(name);
	}

	HwDescriptorSet* HwProgram::getDescriptorSet(uint32_t index) {
		return static_cast<VulkanProgram*>(this)->getDescriptorSet(index);
	}

	VulkanProgram::VulkanProgram() {
	}

	VulkanProgram::VulkanProgram(Ref<HwShaderModule>* shaderModules, uint32_t count) {
		assert(count <= MAX_SHADER_STAGE);
		shaderCodeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
		stageCount = count;

		VkShaderStageFlags fullShaderStageFlags = 0;

		std::map<uint32_t, std::vector<Ref<ShaderResourceInfo>>> combinedBindingMap;
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
		};

		auto isDynamicUniformSet = [](const std::vector<Ref<ShaderResourceInfo>>& bindings)-> bool {
			if (bindings.empty()) {
				return false;
			}

			for (auto& res : bindings) {
				if (res->dsLayoutBinding.descriptorType != DescriptorType::UNIFORM_BUFFER_DYNAMIC) {
					return false;
				}
			}
			return true;
		};
		
		VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
		for (auto& it : combinedBindingMap) {
			std::ranges::sort(it.second, [](auto& v1, auto& v2) { return v1->dsLayoutBinding.binding < v2->dsLayoutBinding.binding; });

			ResourceSetType resourceSetType = ResourceSetType::Default;
			for (auto& res : it.second) {
				if (res->bindless) {

					assert(it.second.size() == 1);
					if (res->dsLayoutBinding.descriptorType == DescriptorType::COMBINED_IMAGE_SAMPLER) {
						resourceSetType = ResourceSetType::CombinedImageSampler;
					}
					else if (res->dsLayoutBinding.descriptorType == DescriptorType::STORAGE_IMAGE) {
						resourceSetType = ResourceSetType::StorageImage;
					}
					else if (res->dsLayoutBinding.descriptorType == DescriptorType::SAMPLED_IMAGE) {
						resourceSetType = ResourceSetType::SampledImage;
					}
					else if (res->dsLayoutBinding.descriptorType == DescriptorType::SAMPLER) {
						resourceSetType = ResourceSetType::Sampler;
					}
				}

			}
			

			auto dsLayout = new DescriptorSetLayout(it.second);
			mDescriptorSetLayouts.emplace_back(dsLayout);

			switch (resourceSetType) {
			case ResourceSetType::CombinedImageSampler:
			{
				auto ds = gfx().getTextureSet()->getDescriptorSet(dsLayout);
				desciptorSets.push_back(*ds);
				mDesciptorSets.emplace_back(ds);
			}
			break;
			case ResourceSetType::SampledImage:
				assert(false);
				break;
			case ResourceSetType::Sampler:
				assert(false);
				break;
			case ResourceSetType::StorageImage:
				assert(false);
				break;
			default:
			{
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

			}
			break;
			}

			descriptorSetLayouts.push_back(dsLayout->handle());
		}

		VkPipelineLayoutCreateInfo pipelineLayoutCI = initializers::pipelineLayoutCreateInfo(descriptorSetLayouts.data(), (uint32_t)descriptorSetLayouts.size());
		VK_CHECK_RESULT(vkCreatePipelineLayout(gfx().device, &pipelineLayoutCI, nullptr, &pipelineLayout));

		createShaders();
	}

	VulkanProgram::~VulkanProgram()
	{
		for (uint32_t i = 0; i < stageCount; i++) {
			g_vkDestroyShaderEXT(gfx().device, shaders[i], nullptr);
		}

		vkDestroyPipelineLayout(gfx().device, pipelineLayout, nullptr);
	}

	ShaderResourceInfo* VulkanProgram::getShaderResource(const String& name) {
		for (auto& sm : mShaderModules) {
			for (auto& res : sm->shaderResourceInfo) {
				if (name == res->name) {
					return res;
				}
			}
		}
		return nullptr;
	}
	
	DescriptorSet* VulkanProgram::getDescriptorSet(uint32_t index) {
		if (index < mDesciptorSets.size()) {
			return mDesciptorSets[index];
		}
		return nullptr;
	}

	bool VulkanProgram::createShaders() {
		VkShaderCreateInfoEXT shaderCreateInfos[MAX_SHADER_STAGE]{};
		SmallVector<VkPushConstantRange, 16> pushConstRanges;
		SmallVector<VkSpecializationMapEntry, 32> specializationMapEntries;
		VkSpecializationInfo specializationInfo;
		for (uint32_t i = 0; i < stageCount; i++) {			
			VulkanShaderModule* sm = mShaderModules[i].get();
			pushConstRanges.clear();
			for (auto& pc : pushConstants) {
				if (any(pc.stageFlags & sm->shaderStage)) {
					pushConstRanges.push_back(VkPushConstantRange{(VkShaderStageFlags)pc.stageFlags, pc.offset, pc.size});
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
			shaderCreateInfos[i].setLayoutCount = (uint32_t)descriptorSetLayouts.size();
			shaderCreateInfos[i].pSetLayouts = descriptorSetLayouts.data();
			shaderCreateInfos[i].pPushConstantRanges = pushConstRanges.data();
			shaderCreateInfos[i].pushConstantRangeCount = (uint32_t)pushConstRanges.size();

			if (specializationMapEntries.size() > 0) {
				shaderCreateInfos[i].pSpecializationInfo = &specializationInfo;
			} else {
				shaderCreateInfos[i].pSpecializationInfo = nullptr;
			}
		}

		VkResult result = g_vkCreateShadersEXT(gfx().device, stageCount, shaderCreateInfos, nullptr, shaders);
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

		return false;
	}

	VkShaderStageFlagBits ToVkShaderStage(ShaderStage stage) {

		switch (stage)
		{
		case ShaderStage::VERTEX:
			return VK_SHADER_STAGE_VERTEX_BIT;
		case ShaderStage::FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;
		case ShaderStage::COMPUTE: return VK_SHADER_STAGE_COMPUTE_BIT;
		case ShaderStage::TESSELLATION_CONTROL: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		case ShaderStage::TESSELLATION_EVALUATION: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		case ShaderStage::GEOMETRY: return VK_SHADER_STAGE_GEOMETRY_BIT;
		default:return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		}
	}

	VkShaderStageFlags GetNextShaderStage(ShaderStage stage) {

		switch (stage)
		{
		case ShaderStage::VERTEX:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		case ShaderStage::FRAGMENT: return 0;
		case ShaderStage::COMPUTE: return 0;
		case ShaderStage::TESSELLATION_CONTROL: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		case ShaderStage::TESSELLATION_EVALUATION: return VK_SHADER_STAGE_FRAGMENT_BIT;
		case ShaderStage::GEOMETRY: return VK_SHADER_STAGE_FRAGMENT_BIT;
		default:return 0;
		}
	}

	static VkDescriptorType getDescriptorType(const Compiler* compiler, const spirv_cross::Resource& resource, bool image, bool storage, bool input = false);
	static VkFormat getResourceFormat(SPIRType::BaseType basetype, uint32_t vecsize, uint32_t& fmtSize, bool isColor);
	static UniformType toUniformType(const SPIRType& spirvType, uint32_t* size = 0);
	static void collectionResource(VkShaderStageFlagBits shaderStage, CompilerGLSL* compiler, const spirv_cross::SmallVector<spirv_cross::Resource>& resources,
		std::vector<Ref<ShaderResourceInfo>>& layout, bool image = false, bool storage = false, bool input = false);
	static void getMembers(CompilerGLSL* compiler, ShaderStruct& u, const SPIRType& spirvType);

	VulkanShaderModule::VulkanShaderModule(ShaderStage stage, const std::vector<uint8_t>& shaderCode, ShaderCodeType shaderCodeType, const char* pShaderEntryPoint) {

		this->shaderCode = shaderCode;

		if (pShaderEntryPoint && pShaderEntryPoint[0]) {
			entryPoint = pShaderEntryPoint;
		}

		vkShaderStage = ToVkShaderStage(stage);
		nextStage = GetNextShaderStage(stage);

		collectShaderResource();

	}

	void VulkanShaderModule::collectShaderResource()
	{
		auto compiler = std::make_unique<CompilerGLSL>((const uint32_t*)shaderCode.data(), shaderCode.size() / 4);
		auto res = compiler->get_shader_resources();

		pushConstants.clear();
		if (!res.push_constant_buffers.empty()) {
			//LOG_INFO("PushConstant :");
			for (auto& pushConst : res.push_constant_buffers) {

				auto& pc = pushConstants.emplace_back();
				pc.stageFlags = shaderStage;
				SPIRType type = compiler->get_type(pushConst.type_id);
				auto strutSize = (uint32_t)compiler->get_declared_struct_size(type);

				//LOG_INFO(pushConst.name, ": size ", strutSize, ", : member count ", type.member_types.size());

				auto baseOffset = std::numeric_limits<uint32_t>::max();
				pc.name = pushConst.name;

				getMembers(compiler.get(), pc, type);
				for (auto& member : pc.members) {
					baseOffset = std::min(baseOffset, member.offset);
				}

				pc.offset = baseOffset;
				pc.size = strutSize - baseOffset;
			}
		}

		//LOG_INFO("uniforms: ");

		if (!res.uniform_buffers.empty()) {
			collectionResource(vkShaderStage, compiler.get(), res.uniform_buffers, shaderResourceInfo);
		}

		if (!res.sampled_images.empty()) {
			collectionResource(vkShaderStage, compiler.get(), res.sampled_images, shaderResourceInfo, true, false);
		}

		if (!res.storage_buffers.empty()) {
			collectionResource(vkShaderStage, compiler.get(), res.storage_buffers, shaderResourceInfo, false, true);
		}

		if (!res.storage_images.empty()) {
			collectionResource(vkShaderStage, compiler.get(), res.storage_images, shaderResourceInfo, true, true);
		}

		if (!res.separate_images.empty()) {
			collectionResource(vkShaderStage, compiler.get(), res.separate_images, shaderResourceInfo, true, false);
		}

		if (!res.separate_samplers.empty()) {
			collectionResource(vkShaderStage, compiler.get(), res.separate_samplers, shaderResourceInfo);
		}

		if (!res.subpass_inputs.empty()) {
			collectionResource(vkShaderStage, compiler.get(), res.subpass_inputs, shaderResourceInfo, false, false, true);
		}

		if (!res.atomic_counters.empty()) {
			collectionResource(vkShaderStage, compiler.get(), res.atomic_counters, shaderResourceInfo);
		}

		if (!res.acceleration_structures.empty()) {
			collectionResource(vkShaderStage, compiler.get(), res.acceleration_structures, shaderResourceInfo);
		}

		auto specialConsts = compiler->get_specialization_constants();
		for (auto& constant : specialConsts) {
			auto& ct = compiler->get_constant(constant.id);
			SPIRType type = compiler->get_type(ct.constant_type);
			auto& specConst = specializationConsts.emplace_back();
			specConst.id = constant.constant_id;
			specConst.type = toUniformType(type, &specConst.size);
		}
	}


	static VkDescriptorType getDescriptorType(const Compiler* compiler, const spirv_cross::Resource& resource, bool image, bool storage, bool input)
	{
		SPIRType type = compiler->get_type(resource.type_id);
		uint32_t nonWritable = compiler->get_decoration(resource.id, spv::Decoration::DecorationNonWritable);
		switch (type.basetype) {
		case SPIRType::BaseType::Struct:
			if (storage) {
				auto bufferBlockFlags = compiler->get_buffer_block_flags(resource.id);
				if (bufferBlockFlags.get(spv::Decoration::DecorationNonWritable)) {
					return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				} else {
					return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				}
			} else {
				return VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			}
		case SPIRType::BaseType::SampledImage:
			return VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		case SPIRType::BaseType::Image:
			if (input) {
				return VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			}
			return storage ? VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		case SPIRType::BaseType::Sampler:
			return VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER;
		default:
			throw std::runtime_error("Unhandled SPIR-V data type.");
		}
	}

	static void collectionResource(VkShaderStageFlagBits shaderStage, CompilerGLSL* compiler, const spirv_cross::SmallVector<spirv_cross::Resource>& resources,
		std::vector<Ref<ShaderResourceInfo>>& layout, bool image, bool storage, bool input)
	{
		for (auto& res : resources) {
			SPIRType type = compiler->get_type(res.type_id);
			auto set = compiler->get_decoration(res.id, spv::Decoration::DecorationDescriptorSet);
			auto binding = compiler->get_decoration(res.id, spv::Decoration::DecorationBinding);
			auto descriptorType = getDescriptorType(compiler, res, image, storage, input);

			for (auto& resInfo : layout) {
				if (resInfo->dsLayoutBinding.binding == binding && resInfo->set == set && resInfo->dsLayoutBinding.descriptorType != (DescriptorType)descriptorType)
					LOG_INFO("The same binding slot was used by multiple resources: {}, set: {}, binding: {}, ", res.name, set, binding, (int)(resInfo->dsLayoutBinding.descriptorType));
			}

			if (descriptorType == VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
				//if (res.name.ends_with("_dynamic")) {
				descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				//}
			} else if (descriptorType == VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
				if (res.name.ends_with("_dynamic")) {
					descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
				}
			}

			auto* shaderResourceInfo = new ShaderResourceInfo();
			shaderResourceInfo->set = set;
			shaderResourceInfo->name = res.name;
			shaderResourceInfo->dsLayoutBinding = DescriptorSetLayoutBinding{ .binding = binding, .descriptorType = (DescriptorType)descriptorType, .descriptorCount = 1, .stageFlags = (ShaderStage)shaderStage };
			shaderResourceInfo->dsLayoutBinding.name = shaderResourceInfo->name;
			
			layout.emplace_back(shaderResourceInfo);
				
			if (descriptorType == VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
				auto strutSize = (uint32_t)compiler->get_declared_struct_size(type);
				shaderResourceInfo->size = strutSize;
				getMembers(compiler, *shaderResourceInfo, type);
			}

			auto arrDims = type.array.size();
			if (arrDims == 1) {
				//for (uint arr = 0; arr < arrDims; arr++)
				uint32_t arr = 0;
				{
					bool isLiteral = type.array_size_literal[arr];
					uint32_t arrCount = type.array[arr];
					if (isLiteral) {
						shaderResourceInfo->dsLayoutBinding.descriptorCount = arrCount == 0 ? VARIABLE_DESC_COUNT : arrCount;
						shaderResourceInfo->bindless = arrCount == 0;
					}

					//LOG_DEBUG("{}, isLiteral: {}, count: {}", res.name, isLiteral, arrCount);

				}
			}

			//LOG_INFO("{}  : set {}, binding {}, type {}", res.name, set, binding, (int)(descriptorType));
		}
	}

	static VkFormat getResourceFormat(SPIRType::BaseType basetype, uint32_t vecsize, uint32_t& fmtSize, bool isColor)
	{
		switch (basetype) {
		case spirv_cross::SPIRType::Unknown:
			break;
		case spirv_cross::SPIRType::Void:
			break;
		case spirv_cross::SPIRType::Boolean:
			break;
		case spirv_cross::SPIRType::SByte:
			fmtSize = 1 * vecsize;
			switch (vecsize) {
			case 1:
				return VK_FORMAT_R8_SINT;
			case 2:
				return VK_FORMAT_R8G8_SINT;
			case 3:
				return VK_FORMAT_R8G8B8_SINT;
			case 4:
				return VK_FORMAT_R8G8B8A8_SINT;
			default:
				break;
			}
			break;
		case spirv_cross::SPIRType::UByte:
			fmtSize = 1 * vecsize;
			switch (vecsize) {
			case 1:
				return VK_FORMAT_R8_UINT;
			case 2:
				return VK_FORMAT_R8G8_UINT;
			case 3:
				return VK_FORMAT_R8G8B8_UINT;
			case 4:
				return VK_FORMAT_R8G8B8A8_UINT;
			default:
				break;
			}
			break;
		case spirv_cross::SPIRType::Short:
			fmtSize = 2 * vecsize;
			switch (vecsize) {
			case 1:
				return VK_FORMAT_R16_SINT;
			case 2:
				return VK_FORMAT_R16G16_SINT;
			case 3:
				return VK_FORMAT_R16G16B16_SINT;
			case 4:
				return VK_FORMAT_R16G16B16A16_SINT;
			default:
				break;
			}
			break;
		case spirv_cross::SPIRType::UShort:
			fmtSize = 2 * vecsize;
			switch (vecsize) {
			case 1:
				return VK_FORMAT_R16_UINT;
			case 2:
				return VK_FORMAT_R16G16_UINT;
			case 3:
				return VK_FORMAT_R16G16B16_UINT;
			case 4:
				return VK_FORMAT_R16G16B16A16_UINT;
			default:
				break;
			}
			break;
		case spirv_cross::SPIRType::Int:
			fmtSize = 4 * vecsize;
			switch (vecsize) {
			case 1:
				return VK_FORMAT_R32_SINT;
			case 2:
				return VK_FORMAT_R32G32_SINT;
			case 3:
				return VK_FORMAT_R32G32B32_SINT;
			case 4:
				return VK_FORMAT_R32G32B32A32_SINT;
			default:
				break;
			}
			break;
		case spirv_cross::SPIRType::UInt:
			fmtSize = 4 * vecsize;
			switch (vecsize) {
			case 1:
				return VK_FORMAT_R32_UINT;
			case 2:
				return VK_FORMAT_R32G32_UINT;
			case 3:
				return VK_FORMAT_R32G32B32_UINT;
			case 4:
				return VK_FORMAT_R32G32B32A32_UINT;
			default:
				break;
			}
			break;
		case spirv_cross::SPIRType::Int64:
			break;
		case spirv_cross::SPIRType::UInt64:
			break;
		case spirv_cross::SPIRType::AtomicCounter:
			break;
		case spirv_cross::SPIRType::Half:
			fmtSize = 2 * vecsize;
			switch (vecsize) {
			case 1:
				return VK_FORMAT_R16_SFLOAT;
			case 2:
				return VK_FORMAT_R16G16_SFLOAT;
			case 3:
				return VK_FORMAT_R16G16B16_SFLOAT;
			case 4:
				return VK_FORMAT_R16G16B16A16_SFLOAT;
			default:
				break;
			}
			break;
		case spirv_cross::SPIRType::Float:
			fmtSize = 4 * vecsize;
			switch (vecsize) {
			case 1:
				return VK_FORMAT_R32_SFLOAT;
			case 2:
				return VK_FORMAT_R32G32_SFLOAT;
			case 3:
				return VK_FORMAT_R32G32B32_SFLOAT;
			case 4:
				if (isColor) {
					fmtSize = 4;
					return VK_FORMAT_R8G8B8A8_UNORM;
				}
				return VK_FORMAT_R32G32B32A32_SFLOAT;
			default:
				break;
			}
			break;
		case spirv_cross::SPIRType::Double:
			break;
		case spirv_cross::SPIRType::Struct:
			break;
		case spirv_cross::SPIRType::Image:
			break;
		case spirv_cross::SPIRType::SampledImage:
			break;
		case spirv_cross::SPIRType::Sampler:
			break;
			//case spirv_cross::SPIRType::AccelerationStructure:
			//    break;
			//case spirv_cross::SPIRType::RayQuery:
			//    break;
		case spirv_cross::SPIRType::ControlPointArray:
			break;
		case spirv_cross::SPIRType::Char:
			break;
		default:
			break;
		}
		return VK_FORMAT_UNDEFINED;
	}

	static UniformType toUniformType(const SPIRType& spirvType, uint32_t* size)
	{
		UniformType uniformType = UniformType::UNKNOWN;
		switch (spirvType.basetype) {
		case SPIRType::Boolean: uniformType = UniformType::BOOL; if (size) *size = 4; break;
		case SPIRType::SByte: uniformType = UniformType::INT8; if (size) *size = 1; break;
		case SPIRType::UByte: uniformType = UniformType::UINT8; if (size) *size = 1; break;
		case SPIRType::Short: uniformType = UniformType::INT16;  if (size) *size = 2; break;
		case SPIRType::UShort: uniformType = UniformType::UINT16;  if (size) *size = 2; break;
		case SPIRType::Int: uniformType = UniformType::INT32; if (size) *size = 4; break;
		case SPIRType::UInt: uniformType = UniformType::UINT32;  if (size) *size = 4; break;
		case SPIRType::Int64: uniformType = UniformType::INT64;  if (size) *size = 8; break;
		case SPIRType::UInt64: uniformType = UniformType::UINT64;  if (size) *size = 8; break;
		case SPIRType::AtomicCounter: uniformType = UniformType::UNKNOWN;  if (size) *size = 8; break;
		case SPIRType::Half: uniformType = UniformType::HALF; if (size) *size = 2; break;
		case SPIRType::Float: uniformType = UniformType::FLOAT;  if (size) *size = 4; break;
		case SPIRType::Double: uniformType = UniformType::DOUBLE;  if (size) *size = 8; break;
		case SPIRType::Struct: uniformType = UniformType::STRUCT; break;
		case SPIRType::Image: uniformType = UniformType::OBJECT; break;
		case SPIRType::SampledImage: uniformType = UniformType::OBJECT; break;
		case SPIRType::Sampler: uniformType = UniformType::UNKNOWN; break;
		case SPIRType::AccelerationStructure: uniformType = UniformType::UNKNOWN; break;
		case SPIRType::RayQuery: uniformType = UniformType::UNKNOWN; break;
		default: break;
		};

		if (uniformType == UniformType::FLOAT) {
			switch (spirvType.vecsize) {
			case 1:
				uniformType = UniformType::FLOAT;
				break;
			case 2:
				uniformType = UniformType::VEC2;
				if (size) *size = 8;
				break;
			case 3:
				uniformType = UniformType::VEC3;
				if (size) *size = 12;
				break;
			case 4:
				switch (spirvType.columns)
				{
				case 1:
					uniformType = UniformType::VEC4;
					break;
				case 4:
					uniformType = UniformType::MAT4;
					break;
				default:
					break;
				}
				if (size) *size = 16*spirvType.columns;
				break;
			default:
				assert(false);
				break;
			}
		}

		return uniformType;
	}

	static void getMembers(CompilerGLSL* compiler, ShaderStruct& u, const SPIRType& spirvType)
	{
		u.type = toUniformType(spirvType);

		uint32_t memberCount = (uint32_t)spirvType.member_types.size();
		if (memberCount == 0)
			return;

		for (uint32_t memberIndex = 0; memberIndex < memberCount; memberIndex++) {
			auto name = compiler->get_member_name(spirvType.self, memberIndex);
			auto size = (uint32_t)compiler->get_declared_struct_member_size(spirvType, memberIndex);// (uint32_t)range.range;
			auto offset = compiler->get_member_decoration(spirvType.self, memberIndex, spv::DecorationOffset); //(uint32_t)range.offset;

			ShaderStruct resType{ .name = name, .offset = offset, .size = size };
			auto& memberType = compiler->get_type(spirvType.member_types[memberIndex]);
			getMembers(compiler, resType, memberType);
			u.members.push_back(resType);

		}
	}




}        // namespace mygfx
