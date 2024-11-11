Shader "test" 
{
    MaterialBlock "MaterialUniforms"
    {

    }

    Pass
    {
        BlendMode ALPHA

        VertexInput
        {
            PerVertex = [R32G32B32_SFLOAT, END, R32G32_SFLOAT, END, R32G32B32_SFLOAT]
        }

        Macros
        {
            MATERIAL_METALLICROUGHNESS = ""
        }

        VertexShader = "glsl/primitive.vert"
        PixelShader = "glsl/pbr.frag"

    }

    
}
