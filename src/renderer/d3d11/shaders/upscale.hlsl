// input struct
struct vs_in {
    float2 pos: POS0;
    float2 uv: TEXCOORD0;
};
// output struct
struct vs_out {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

vs_out vs_main(vs_in input) {
    vs_out output = (vs_out)0; // zero the memory
    output.pos = float4(input.pos, 0.0f, 1.0f);
    output.uv = input.uv;

    return output;
}



Texture2D    InternalTex  : register(t0);
SamplerState PointSampler : register(s0);

float4 ps_main(vs_out input) : SV_TARGET {
    float4 tex_albedo = InternalTex.Sample(PointSampler, input.uv);

    if(tex_albedo.w < 0.01)
    {
        discard;
    }
    
    return tex_albedo;
}
