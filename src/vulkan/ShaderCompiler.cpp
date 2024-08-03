#include "ShaderCompiler.h"
#include <codecvt>
#include <locale>
//#include <shlobj.h>
#include <iostream>
#include <fstream>
#include "VulkanDevice.h"

#define USE_SHADER_C

#include <shaderc/shaderc.hpp>
#include "utils/Log.h"
#include "utils/FileUtils.h"

namespace mygfx
{
	namespace
	{
		std::mutex sIncludeFileMutex;
		std::unordered_map<String, String> sIncludeFiles;
	}

	ShaderCompiler::ShaderCompiler()
	{
	}

	ShaderCompiler::~ShaderCompiler()
	{
	}

	class ShaderIncluder : public shaderc::CompileOptions::IncluderInterface
	{
	public:
		shaderc_include_result* GetInclude(const char* requested_source,
			shaderc_include_type type, const char* requesting_source, size_t include_depth) override

		{
			std::lock_guard locker(sIncludeFileMutex);

			String fileName(requested_source);	
			shaderc_include_result* ret = new shaderc_include_result();
			ret->source_name = fileName.c_str();
			ret->source_name_length = fileName.length();
			
			auto it = sIncludeFiles.find(fileName);
			if (it != sIncludeFiles.end()) {
				ret->content = it->second.data();
				ret->content_length = it->second.length();
				return ret;
			}

			String content = FileUtils::readAllText(fileName);
			ret->content = content.data();
			ret->content_length = content.length();
			sIncludeFiles.emplace(fileName, std::move(content));

			return ret;
		}

		// Handles shaderc_include_result_release_fn callbacks.
		void ReleaseInclude(shaderc_include_result* data) override
		{
			//delete data;
		}
	};

	static shaderc_shader_kind getShadercKind(VkShaderStageFlagBits stage)
	{
		switch (stage) {
		case VK_SHADER_STAGE_VERTEX_BIT:
			return shaderc_vertex_shader;
		case VK_SHADER_STAGE_GEOMETRY_BIT:
			return shaderc_geometry_shader;
		case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
			return shaderc_tess_control_shader;
		case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
			return shaderc_tess_evaluation_shader;
		case VK_SHADER_STAGE_FRAGMENT_BIT:
			return shaderc_fragment_shader;
		case VK_SHADER_STAGE_COMPUTE_BIT:
			return shaderc_compute_shader;
		default:
			assert(false && "Invalid shader stage");
			return shaderc_vertex_shader;
			break;
		}
	}


	bool compileShaderC(ShaderSourceType sourceType, const VkShaderStageFlagBits shader_type, const String& shaderCode, const char* pShaderEntryPoint, const char* shaderCompilerParams, const DefineList* pDefines, ByteArray& outSpvData)
	{
		shaderc::CompileOptions options;
		//if (debug) {
		//options.SetGenerateDebugInfo();
		//} else {
		//options.SetOptimizationLevel(shaderc_optimization_level_performance);
		//}

		//options.SetTargetEnvironment(shaderc_target_env::shaderc_target_env_vulkan, 0);

		if (sourceType == ShaderSourceType::HLSL) {
			options.SetSourceLanguage(shaderc_source_language_hlsl);
			options.SetTargetSpirv(shaderc_spirv_version::shaderc_spirv_version_1_0);
		} else {
			options.SetSourceLanguage(shaderc_source_language_glsl);
			options.SetTargetSpirv(shaderc_spirv_version::shaderc_spirv_version_1_4);
		}

		auto includer = std::make_unique<ShaderIncluder>();
		options.SetIncluder(std::move(includer));
		if (pDefines) {
			for (const auto& macro : *pDefines) {
				if (macro.second.empty()) {
					options.AddMacroDefinition(macro.first);
				} else {
					options.AddMacroDefinition(macro.first, macro.second);
				}
			}
		}

		//if (strcmp(pShaderEntryPoint, "main") != 0) {
		//	options.AddMacroDefinition(pShaderEntryPoint, "main");
		//}

		shaderc_shader_kind kind = getShadercKind(shader_type);
		shaderc::Compiler compiler;

		try {
			shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(shaderCode, kind, "", "main", options);
			if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
				auto error = result.GetErrorMessage();
				LOG_ERROR(error);
				return false;
			}

			outSpvData.resize((char*)result.end() - (char*)result.begin());
			memcpy(outSpvData.data(), result.begin(), outSpvData.size());
		} catch (std::exception& e) {
			LOG_ERROR(e.what());
			return false;
		}

		return true;
	}

	bool compileToSpirv(ShaderSourceType sourceType, const VkShaderStageFlagBits shader_type, const String& shaderCode, const char* pShaderEntryPoint, const char* shaderCompilerParams, const DefineList* pDefines, ByteArray& outSpvData)
	{
		if (sourceType == ShaderSourceType::GLSL) {
			return compileShaderC(sourceType, shader_type, shaderCode, pShaderEntryPoint, shaderCompilerParams, pDefines, outSpvData);
		}

		return false;
	}

	String generateSource(ShaderSourceType sourceType, const VkShaderStageFlagBits shader_type, const String& pshader, const char* shaderCompilerParams, const DefineList* pDefines)
	{
		String shaderCode(pshader);
		String code;

		if (sourceType == ShaderSourceType::GLSL) {
			size_t offset = shaderCode.find("#version");
			if (offset != std::string::npos) {
				offset = shaderCode.find_first_of('\n', offset);
				code = shaderCode.substr(offset, shaderCode.size() - offset);
				shaderCode = shaderCode.substr(0, offset) + "\n";
			}
			else {
				shaderCode = "";
			}

		} else if (sourceType == ShaderSourceType::HLSL) {
			code = shaderCode;
			shaderCode = "";
		}

		shaderCode += "#define TARGET_VULKAN_ENVIRONMENT\n";
		
		if (shader_type == VK_SHADER_STAGE_VERTEX_BIT) {			
			shaderCode += "#define SHADER_STAGE_VERTEX\n";
		} else if (shader_type == VK_SHADER_STAGE_FRAGMENT_BIT) {			
			shaderCode += "#define SHADER_STAGE_FRAGMENT\n";
		} else if (shader_type == VK_SHADER_STAGE_COMPUTE_BIT) {			
			shaderCode += "#define SHADER_STAGE_COMPUTE\n";
		}

		// add the #defines to the code to help debugging
		if (pDefines) {
			for (auto it = pDefines->begin(); it != pDefines->end(); it++)
				shaderCode += "#define " + it->first + " " + it->second + "\n";
		}

		// concat the actual shader code
		shaderCode += code;

		return shaderCode;
	}
	
	 ShaderStage ToShaderStage(VkShaderStageFlagBits stage) {
		
		switch (stage)
		{
		case VK_SHADER_STAGE_VERTEX_BIT:
			return ShaderStage::VERTEX;
		case VK_SHADER_STAGE_FRAGMENT_BIT: return ShaderStage::FRAGMENT;
		case VK_SHADER_STAGE_COMPUTE_BIT: return ShaderStage::COMPUTE;
		case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: return ShaderStage::TESSELLATION_CONTROL;
		case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return ShaderStage::TESSELLATION_EVALUATION;
		case VK_SHADER_STAGE_GEOMETRY_BIT: return ShaderStage::GEOMETRY;
		default:return ShaderStage::None;
		}
	}
	
	Ref<HwShaderModule> ShaderCompiler::compileFromString(ShaderSourceType sourceType, const VkShaderStageFlagBits shader_type, const String& shaderCode, const char* pShaderEntryPoint, const char* shaderCompilerParams, const DefineList* pDefines)
	{
		assert(shaderCode.size() > 0);

		Ref<HwShaderModule> sm;
		ByteArray SpvData;
		String shader = generateSource(sourceType, shader_type, shaderCode, shaderCompilerParams, pDefines);
		if (!compileToSpirv(sourceType, shader_type, shader.c_str(), pShaderEntryPoint, shaderCompilerParams, pDefines, SpvData)) {
			return nullptr;
		}

		assert(SpvData.size() != 0);
		sm = device().createShaderModule(ToShaderStage(shader_type), SpvData, ShaderCodeType::SPIRV);
		return sm;

	}

	Ref<HwShaderModule> ShaderCompiler::compileFromFile(const VkShaderStageFlagBits shader_type, const char* pFilename, const char* pShaderEntryPoint, const char* shaderCompilerParams, const DefineList* pDefines)
	{
		String pShaderCode;
		ShaderSourceType sourceType;

		const char* pExtension = pFilename + std::max<size_t>(strlen(pFilename) - 4, 0);
		if (strcmp(pExtension, "glsl") == 0)
			sourceType = ShaderSourceType::GLSL;
		else if (strcmp(pExtension, "hlsl") == 0)
			sourceType = ShaderSourceType::HLSL;
		else
			assert(!"Can't tell shader type from its extension");

		std::string fullpath = pFilename;
		pShaderCode = FileUtils::readAllText(fullpath);
		auto res = compileFromString(sourceType, shader_type, pShaderCode, pShaderEntryPoint, shaderCompilerParams, pDefines);
		if (res) {
			//gfx().setResourceName(VK_OBJECT_TYPE_SHADER_MODULE, (uint64_t)pShader->module, pFilename);
			return res;
		}
		
		return nullptr;
	}
	
}