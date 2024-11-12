#pragma once
#include "core/System.h"
#include "ShaderResourceInfo.h"

class Sync;

namespace mygfx {

class ShaderCompiler :  public System {
public:
    void init() override;
    
    static bool getShaderFilePath(const String& fileName, Path& outPath);

    static Ref<HwShaderModule> compileFromString(ShaderSourceType sourceType, ShaderStage shader_type, const String& shaderName, const String& pShaderCode, const char* pShaderEntryPoint, const char* pExtraParams, const DefineList* pDefines);
    static Ref<HwShaderModule> compileFromFile(ShaderStage shader_type, const char* pFilename, const char* pShaderEntryPoint, const char* pExtraParams, const DefineList* pDefines);
};

}
