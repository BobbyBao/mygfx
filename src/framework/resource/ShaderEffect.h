#pragma once
#include "core/Resource.h"
#include <array>

namespace mygfx {

class Shader;
struct PassID;
class DefineList;

class ShaderEffect : public Resource {
public:
    ShaderEffect();
    ~ShaderEffect();
    
    const auto& getShaders() const {  return mShaders; }
    Shader* getMainPass() { return mShaderPasses[0]; }

    Shader* getShader(const String& name);
    Shader* getShader(const PassID& pass);
    
    void add(const String& vsCode, const String& fsCode, const DefineList* marcos = nullptr);
    void add(const String& csCode, const DefineList* marcos = nullptr);
    void add(Shader* shader);

    static Ref<ShaderEffect> load(const String& fileName);
    static Ref<ShaderEffect> fromFile(const String& vs, const String& fs, const DefineList* marcos = nullptr);
private:
    Vector<Ref<Shader>> mShaders;
    std::array<Shader*, 64> mShaderPasses{};
};

}
