#pragma once
#include "core/Resource.h"
#include "render/PassName.h"
#include <array>

namespace mygfx {

class Shader;
struct PassName;
class DefineList;

class ShaderEffect : public Resource {
public:
    ShaderEffect();
    ~ShaderEffect();
    
    const auto& getShaders() const {  return mShaderPasses; }
    Shader* getMainPass() { return getShader(PassName::Main); }

    Shader* getShader(const String& name);
    Shader* getShader(const PassName& pass);
    
    void add(const String& vsCode, const String& fsCode, const DefineList* marcos = nullptr);
    void add(const String& csCode, const DefineList* marcos = nullptr);
    void add(Shader* shader);

    PROPERTY_GET_SET_1(String, MaterialUniformName)

    static Ref<ShaderEffect> load(const String& fileName);
    static Ref<ShaderEffect> fromFile(const String& vs, const String& fs, const DefineList* marcos = nullptr);
private:
    Vector<std::pair<PassName, Ref<Shader>>> mShaderPasses;
    String mMaterialUniformName = "MaterialUniforms";
};

}
