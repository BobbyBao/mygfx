Shader "test" 
{
    MaterialBlock "MaterialUniforms"
    {

    }

    Pass
    {
        VertexShader = "shaders/primitive.vert"
        PixelShader = "shaders/pbr.frag"

    }

    
}
