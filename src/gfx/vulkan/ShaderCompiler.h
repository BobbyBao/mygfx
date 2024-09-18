#pragma once
#include "../ShaderResourceInfo.h"

class Sync;

namespace mygfx {

class ShaderCompiler {
public:
    ShaderCompiler();
    ~ShaderCompiler();

    // Does as the function name says and uses a cache
    static Ref<HwShaderModule> compileFromString(ShaderSourceType sourceType, const ShaderStage shader_type, const String& shaderName, const String& pShaderCode, const char* pShaderEntryPoint, const char* pExtraParams, const DefineList* pDefines);
    static Ref<HwShaderModule> compileFromFile(const ShaderStage shader_type, const char* pFilename, const char* pShaderEntryPoint, const char* pExtraParams, const DefineList* pDefines);
};

}
