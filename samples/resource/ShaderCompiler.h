#pragma once
#include "ShaderResourceInfo.h"
#include "../FileSystem.h"

namespace mygfx {

class ShaderCompiler {
public:
    void init();
    
    static bool getShaderFilePath(const String& fileName, Path& outPath);

    static Ref<HwShaderModule> compileFromString(ShaderSourceType sourceType, ShaderStage shader_type, const String& shaderName, const String& pShaderCode, const char* pShaderEntryPoint, const char* pExtraParams, const DefineList* pDefines);
    static Ref<HwShaderModule> compileFromFile(ShaderStage shader_type, const char* pFilename, const char* pShaderEntryPoint, const char* pExtraParams, const DefineList* pDefines);
};

}
