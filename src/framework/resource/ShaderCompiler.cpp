#include "ShaderCompiler.h"
#include "GraphicsDevice.h"
#include "core/FileSystem.h"
#include "utils/Log.h"
#include <mutex>
#include <shaderc/shaderc.hpp>

namespace mygfx {

namespace {
    std::mutex sIncludeFileMutex;
    std::unordered_map<String, String> sIncludeFiles;
    Vector<Path> sShaderPath;

    bool getIncludeFilePath(const String& fileName, Path& outPath)
    {
        for (auto& path : sShaderPath) {
            Path p = path / fileName;
            if (std::filesystem::exists(p)) {
                outPath = p;
                return true;
            }
        }

        return false;
    }
}

void ShaderCompiler::init()
{
    sShaderPath.push_back(FileSystem::convertPath("shaders"));
    sShaderPath.push_back(FileSystem::convertPath("shaders/glsl"));
}

class ShaderIncluder : public shaderc::CompileOptions::IncluderInterface {
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

        Path fullPath;
        if (getIncludeFilePath(fileName, fullPath)) {
            String content = FileSystem::readAllText(fullPath);
            ret->content = content.data();
            ret->content_length = content.length();
            sIncludeFiles.emplace(fileName, std::move(content));
            return ret;
        }
        return nullptr;
    }

    // Handles shaderc_include_result_release_fn callbacks.
    void ReleaseInclude(shaderc_include_result* data) override
    {
        delete data;
    }
};

static shaderc_shader_kind getShadercKind(ShaderStage stage)
{
    switch (stage) {
    case ShaderStage::VERTEX:
        return shaderc_vertex_shader;
    case ShaderStage::GEOMETRY:
        return shaderc_geometry_shader;
    case ShaderStage::TESSELLATION_CONTROL:
        return shaderc_tess_control_shader;
    case ShaderStage::TESSELLATION_EVALUATION:
        return shaderc_tess_evaluation_shader;
    case ShaderStage::FRAGMENT:
        return shaderc_fragment_shader;
    case ShaderStage::COMPUTE:
        return shaderc_compute_shader;
    default:
        assert(false && "Invalid shader stage");
        return shaderc_vertex_shader;
        break;
    }
}

bool compileShaderC(ShaderSourceType sourceType, const ShaderStage shader_type, const String& shaderName, const String& shaderCode, const char* pShaderEntryPoint, const char* shaderCompilerParams, const DefineList* pDefines, ByteArray& outSpvData)
{
    shaderc::CompileOptions options;
    // if (debug) {
    // options.SetGenerateDebugInfo();
    // } else {
    // options.SetOptimizationLevel(shaderc_optimization_level_performance);
    // }

    // options.SetTargetEnvironment(shaderc_target_env::shaderc_target_env_vulkan, 0);

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

    if (pShaderEntryPoint == nullptr || pShaderEntryPoint[0] == 0) {
        pShaderEntryPoint = "main";
    }
    // if (strcmp(pShaderEntryPoint, "main") != 0) {
    //	options.AddMacroDefinition(pShaderEntryPoint, "main");
    // }

    shaderc_shader_kind kind = getShadercKind(shader_type);
    shaderc::Compiler compiler;

    try {
        auto res = compiler.PreprocessGlsl(shaderCode, kind, shaderName.c_str(), options);
        if (res.GetCompilationStatus() != shaderc_compilation_status_success) {
            auto error = res.GetErrorMessage();
            LOG_ERROR(error);
            return false;
        }

        shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(res.begin(), res.cend() - res.begin(), kind, shaderName.c_str(), pShaderEntryPoint, options);
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

bool compileToSpirv(ShaderSourceType sourceType, const ShaderStage shader_type, const String& shaderName, const String& shaderCode, const char* pShaderEntryPoint, const char* shaderCompilerParams, const DefineList* pDefines, ByteArray& outSpvData)
{
    if (sourceType == ShaderSourceType::GLSL) {
        return compileShaderC(sourceType, shader_type, shaderName, shaderCode, pShaderEntryPoint, shaderCompilerParams, pDefines, outSpvData);
    }

    return false;
}

String generateSource(ShaderSourceType sourceType, const ShaderStage shader_type, const String& pshader, const char* shaderCompilerParams, const DefineList* pDefines)
{
    String shaderCode(pshader);
    String code;

    if (sourceType == ShaderSourceType::GLSL) {
        size_t offset = shaderCode.find("#version");
        if (offset != std::string::npos) {
            offset = shaderCode.find_first_of('\n', offset);
            code = shaderCode.substr(offset, shaderCode.size() - offset);
            shaderCode = shaderCode.substr(0, offset) + "\n";
        } else {
            //   shaderCode = "";
        }

    } else if (sourceType == ShaderSourceType::HLSL) {
        code = shaderCode;
        shaderCode = "";
    }

    shaderCode += "#define TARGET_VULKAN_ENVIRONMENT\n";

    if (shader_type == ShaderStage::VERTEX) {
        shaderCode += "#define SHADER_STAGE_VERTEX\n";
    } else if (shader_type == ShaderStage::FRAGMENT) {
        shaderCode += "#define SHADER_STAGE_FRAGMENT\n";
    } else if (shader_type == ShaderStage::COMPUTE) {
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

Ref<HwShaderModule> ShaderCompiler::compileFromString(ShaderSourceType sourceType, ShaderStage shader_type, const String& shaderName, const String& shaderCode, const char* pShaderEntryPoint, const char* shaderCompilerParams, const DefineList* pDefines)
{
    assert(shaderCode.size() > 0);

    Ref<HwShaderModule> sm;
    ByteArray SpvData;
#if false
    String shader = generateSource(sourceType, shader_type, shaderCode, shaderCompilerParams, pDefines);
    if (!compileToSpirv(sourceType, shader_type, shaderName, shader.c_str(), pShaderEntryPoint, shaderCompilerParams, pDefines, SpvData)) {
        return nullptr;
    }
#else
    DefineList defines;
    if (shader_type == ShaderStage::VERTEX) {
        defines.add("SHADER_STAGE_VERTEX");
    } else if (shader_type == ShaderStage::FRAGMENT) {
        defines.add("SHADER_STAGE_FRAGMENT");
    } else if (shader_type == ShaderStage::COMPUTE) {
        defines.add("SHADER_STAGE_COMPUTE");
    }

    if (LINEAR_COLOR_OUTPUT) {
        defines.add("LINEAR_OUTPUT");
    }

    if (pDefines) {
        defines += *pDefines;
    }

    if (!compileToSpirv(sourceType, shader_type, shaderName, shaderCode.c_str(), pShaderEntryPoint, shaderCompilerParams, &defines, SpvData)) {
        return nullptr;
    }
#endif

    assert(SpvData.size() != 0);
    sm = device().createShaderModule(shader_type, SpvData, ShaderCodeType::SPIRV, pShaderEntryPoint);
    return sm;
}

Ref<HwShaderModule> ShaderCompiler::compileFromFile(ShaderStage shader_type, const char* pFilename, const char* pShaderEntryPoint, const char* shaderCompilerParams, const DefineList* pDefines)
{
    String pShaderCode;
    ShaderSourceType sourceType = ShaderSourceType::GLSL;

    const char* pExtension = pFilename + std::max<size_t>(strlen(pFilename) - 4, 0);
    if (strcmp(pExtension, "glsl") == 0)
        sourceType = ShaderSourceType::GLSL;
    else if (strcmp(pExtension, "hlsl") == 0)
        sourceType = ShaderSourceType::HLSL;
    else
        assert(!"Can't tell shader type from its extension");

    Path fullPath = pFilename;
    pShaderCode = FileSystem::readAllText(fullPath);
    auto res = compileFromString(sourceType, shader_type, pFilename, pShaderCode, pShaderEntryPoint, shaderCompilerParams, pDefines);
    if (res) {
        return res;
    }

    return nullptr;
}

}